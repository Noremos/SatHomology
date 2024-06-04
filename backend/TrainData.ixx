module;

#include <sqlite3.h>

#include <vector>
#include <cassert>
#include <functional>
//#include <iostream>
#include "Usings.h"
#include "../Bind/Json.h"

#include "MetadataIO.h"
#include "MatrImg.h"
#include "../Bind/Framework.h"
export module TrainIO;

import BackTypes;
// import MetadataCoreIO;
// import Platform;
//import BackBind;
// import MatrModule;

//enum ClassCategType
//{
//	CCT_base
//};

export struct CachedObjectId
{
	int tileId = -1;
	int vecId = -1;
	bool hasData() const
	{
		return vecId != -1;
	}
};

class IBarClassCategor : public IJsonIO
{
public:
	int id;
	BackString name;
	IBarClassCategor(int id = -1, const BackString& name = "") :
		id(id), name(name)
	{ }
};

export class BarClassCategor : public IBarClassCategor
{
public:
	bool show = true;
	BackColor color;

	BarClassCategor(int id = 0, const BackString& name = "") : IBarClassCategor(id, name),
		show(true), color(BackColor::random())
	{ }

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider&)
	{
		state->scInt("id", id);
		state->scStr("name", name);
		int size = 3;

		JsonArrayIOState* arr = state->arrayBegin("color", size);
		arr->scUchar(0, color.r);
		arr->scUchar(1, color.g);
		arr->scUchar(2, color.b);
		state->arrayEnd();
	}
};

//class IBarCategories
//{
//	virtual int addValue(const BackString& nname) = 0;
//
//};

export class BarCategories // : IBarCategories
{
	int counter = 0;

public:
	std::vector<BarClassCategor> categs;

	int addValue(const BackString& nname)
	{
		int id = counter++;

		categs.push_back({ id, nname });
		return id;
	}

	BarClassCategor* get(int id)
	{
		for (auto& c : categs)
		{
			if (c.id == id)
			{
				return &c;
			}
		}

		return nullptr;
	}

	void changeName(int id, const BackString& nname)
	{
		for (auto& c : categs)
		{
			if (c.id == id)
			{
				c.name = nname;
				break;
			}
		}
	}

	void remove(int id)
	{
		for (size_t i = 0; i < categs.size(); i++)
		{
			if (categs[i].id == id)
			{
				categs.erase(categs.begin() + i);
				break;
			}
		}
	}

	size_t size()
	{
		return categs.size();
	}

	static BarCategories loadCategories(const BackPathStr& path)
	{
		BarCategories categ;
		if (!pathExists(path))
		{
			categ.categs.push_back({ 0, "Bad" , });
			categ.categs.push_back({ 1, "Kurgan"});
			categ.counter = 2;

			return categ;
		}

		//barclassificator barclas;
		BackJson loadDoc = jsonFromFile(path.string());// Project::getPathSt(BackPath::classifier));

		int temp;
		MetadataProvider d("", temp);

		categ.counter = loadDoc["counter"].asInt();
		JsonArray list = loadDoc["categories"];
		for (auto item : list)
		{
			const BackJson& catId = item;
			BarClassCategor bcc;
			bcc.read(catId, d);
			categ.categs.push_back(bcc);
		}

		return categ;
	}

	void saveCategories(const BackPathStr& path)
	{
		int temp;
		MetadataProvider d("", temp);

		JsonArray arr(Json::ValueType::arrayValue);
		for (size_t i = 0; i < categs.size(); i++)
		{
			JsonObject catId;
			categs[i].write(catId, d);
			arr.append(catId);
		}

		BackJson loadDoc;
		loadDoc["counter"] = counter;
		loadDoc["categories"] = arr;
		jsonToFile(loadDoc, path);
	}
};

// class ClassData
// {
// 	int categId;
// 	BarcodeHolder curBar;
// };

export class ClassDataIO
{
	sqlite3* db = nullptr;

public:
	void openRead(const BackPathStr& dbpath)
	{
		open(dbpath);
	}

	void openWrite(const BackPathStr& dbpath)
	{
		open(dbpath);
	}

	void open(const BackPathStr& dbpath)
	{
		char* err = 0;

		// "traindata.dblite"
		// открываем соединение
		if (sqlite3_open(dbpath.string().c_str(), &db))
			fprintf(stderr, "Ошибка открытия/создания БД: %s\n", sqlite3_errmsg(db));
		else
		{
			const char* SQL = "CREATE TABLE IF NOT EXISTS CLASS_DATA(class_id, bbf, icon)";
			// INSERT INTO FOO VALUES(1,2,3); INSERT INTO FOO SELECT * FROM FOO;";

			if (sqlite3_exec(db, SQL, 0, 0, &err))
			{
				fprintf(stderr, "Ошибка SQL: %sn", err);
				sqlite3_free(err);
			}
		}
	}

	using TrainCallback = std::function<void(int classId, vbuffer& bbfFile, BackImage preview, size_t localId)>;

	enum LoadField : unsigned char
	{
		LF_BINFILE = 1,
		LF_ICON = 2,
		// LF_ROWID = 4,
		LF_ALL = 255
	};

	void loadAll(TrainCallback& callback, int classId = -1, LoadField filter = LF_ALL)
	{
		sqlite3_stmt* stmt = prepareSelect(filter, classId);

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			vbuffer bff;
			BackImage img(1,1,1);
			size_t locId;
			// int classIdo;

			loadEnd(stmt, filter, classId, bff, img, locId);
			callback(classId, bff, std::move(img), locId);
		}

		sqlite3_finalize(stmt);
	}



	size_t getClassCount(int classId)
	{
		BackString sql = "SELECT COUNT(*) FROM CLASS_DATA WHERE class_id = ?";

		sqlite3_stmt* stmt;
		auto rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);

		if (rc != SQLITE_OK)
		{
			//std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
			return 0;
		}

		assert(classId != -1);
		sqlite3_bind_int64(stmt, 1, classId); // bind the rowid value to the first placeholder

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_ROW)
		{
			//std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
			return 0;
		}

		size_t size = sqlite3_column_int64(stmt, 0);

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			//std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
			return 0;
		}

		// Finalize statement
		sqlite3_finalize(stmt);

		return size;
	}

private:
	BackString prepareSelectBase(LoadField filter = LF_ALL)
	{
		BackString statement = "SELECT ";

		// if (filter == LF_ALL)
		// {
		// 	statement += " rowid, * ";
		// }
		// else
		// {
			// if (filter & LF_ROWID)
			// {
			// 	if (added)
			// 		statement += ", ";

			// 	statement += "rowid ";
			// 	added = true;
			// }

		bool added = true;
		statement += "rowid, class_id ";

		if (filter & LF_BINFILE)
		{
			if (added)
				statement += ", ";

			statement += "bbf";
			added = true;
		}
		if (filter & LF_ICON)
		{
			if (added)
				statement += ", ";

			statement += "icon";
			added = true;
		}

		statement += " FROM CLASS_DATA";
		return statement;
	}

	sqlite3_stmt* prepareSelect(LoadField filter = LF_ALL, int classId = -1)
	{
		BackString statement = prepareSelectBase(filter);

		if (classId != -1)
		{
			statement += " WHERE class_id = ?";
		}
		else
			statement += ";";

		sqlite3_stmt* stmt;
		auto rc = sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, nullptr);

		if (rc != SQLITE_OK)
		{
			////std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
			return nullptr;
		}

		if (classId != -1)
			sqlite3_bind_int64(stmt, 1, classId); // bind the rowid value to the first placeholder

		return stmt;
	}

	sqlite3_stmt* prepareSelectSingle(LoadField filter = LF_ALL, size_t localId = -1)
	{
		BackString statement = prepareSelectBase(filter);

		if (localId != -1)
		{
			statement += " WHERE rowid = ?";
		}
		else
			statement += ";";


		sqlite3_stmt* stmt;
		auto rc = sqlite3_prepare_v2(db, statement.c_str(), statement.length(), &stmt, nullptr);

		if (rc != SQLITE_OK)
		{
			//std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
			return nullptr;
		}

		if (localId != -1)
			sqlite3_bind_int64(stmt, 1, localId); // bind the rowid value to the first placeholder

		return stmt;
	}

	void loadEnd(sqlite3_stmt* stmt, LoadField filter, int& classId, vbuffer& bbfFile, BackImage& preview, size_t& locId)
	{
		int counterId = 0;

		locId = sqlite3_column_int64(stmt, counterId++); // rowid
		classId = sqlite3_column_int(stmt, counterId++);

		bbfFile.release();
		preview = BackImage(1,1,1);
		if (filter & LF_BINFILE)
		{
			const void* file = sqlite3_column_blob(stmt, counterId);
			int binfileSize = sqlite3_column_bytes(stmt, counterId);
			bbfFile.copyDataFrom(reinterpret_cast<const buchar*>(file), binfileSize);

			++counterId;
		}
		if (filter & LF_ICON)
		{
			StaticBuffer<const buchar> imgBuff;

			const void* imgData = sqlite3_column_blob(stmt, counterId);
			int imgSize = sqlite3_column_bytes(stmt, counterId);

			preview = imreadFromMemory(reinterpret_cast<const buchar*>(imgData), imgSize);
			++counterId;
		}
	}

public:
	size_t save(int classId, const vbuffer& bbfFile, BackImage* preview)
	{
		sqlite3_stmt* stmt = NULL;

		BackString statement = "INSERT INTO CLASS_DATA(class_id, bbf, icon) VALUES (?, ?, ?);";

		auto rc = sqlite3_prepare_v2(db, statement.c_str(), statement.length(), &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			//std::cerr << "Failed to prepare statement: " << sqlite3_errstr(rc) << sqlite3_errmsg(db) << std::endl;
		}

		rc = sqlite3_bind_int(stmt, 1, classId); // bind the rowid value to the first placeholder
		if (rc != SQLITE_OK)
		{
			//std::cerr << "Failed to prepare statement: " << sqlite3_errstr(rc) << sqlite3_errmsg(db) << std::endl;
		}

		rc = sqlite3_bind_blob(stmt, 2, bbfFile.data(), bbfFile.size(), SQLITE_STATIC);
		if (rc != SQLITE_OK)
		{
			//std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		}

		MemImgData imgPrev;
		if (preview)
		{
			imgPrev = imwriteToMemory(*preview);
			rc = sqlite3_bind_blob(stmt, 3, imgPrev.data(), imgPrev.size(), SQLITE_STATIC);
		}
		else
			rc = sqlite3_bind_null(stmt, 3);

		if (rc != SQLITE_OK)
		{
			//std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		}

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			//std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
		}

		size_t locId = sqlite3_last_insert_rowid(db);

		// Finalize statement
		sqlite3_finalize(stmt);

		return locId;
	}

	void close()
	{
		sqlite3_close(db);
	}

	~ClassDataIO()
	{
		close();
	}

	void remove(int localId)
	{
		sqlite3_stmt* stmt = NULL;
		const char* statement = "DELETE FROM CLASS_DATA WHERE ROWID = ?;";

		auto rc = sqlite3_prepare_v2(db, statement, -1, &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			printf("Failed to prepare statement : %s\n", sqlite3_errmsg(db));
		}

		rc = sqlite3_bind_int64(stmt, 1, localId); // bind the rowid value to the first placeholder
		if (rc != SQLITE_OK)
		{
			printf("Failed to bind data: %s\n", sqlite3_errmsg(db));
		}

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_OK)
		{
			printf("Failed to execute statement : %s\n", sqlite3_errmsg(db));
		}

		// Finalize statement
		sqlite3_finalize(stmt);
	}
};

// class ByteStream : public std::ostream {

// public:
// 	ByteStream() : std::ostream()
// 	{}

// 	// Overload operator<< for std::byte
// 	ByteStream& operator<<(std::byte b) {
// 		*static_cast<std::ostream*>(this) << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
// 		return *this;
// 	}
// };



export class BarClassifierDataIO
{
public:
	void loadImgs(std::function<void(int classId, const BackPathStr& path)> callback, int* categorues, int size)
	{
		BackPathStr pathp = "";// Project::getPathSt(BackPath::classfiles);
		for (size_t i = 0; i < size; i++)
		{
			int categ = categorues[i];
			BackDirStr dirl = pathp / (BackPathStr)intToStr(categ);
			if (!pathExists(dirl))
				continue;

			// for (auto const& entry : std::filesystem::directory_iterator(dirl))
			// {
			// 	if (!entry.is_regular_file())
			// 	{
			// 		continue;
			// 	}

			// 	BackPathStr filename = entry.path().string();
			// 	auto ext = entry.path().extension();
			// 	if (ext == ".jpg")
			// 	{
			// 		callback(categ, filename);
			// 	}
			// }
		}
	}

	//void save(IClassItem* curBar, int classIndex, BackImage* img)
	//{
	//	BackPathStr path = "";// Project::getProject()->getPath(BackPath::classfiles);
	//	BackDirStr doorPat = path / intToStr(classIndex);
	//	mkDirIfNotExists(doorPat);

	//	std::mt19937 generator(std::random_device{}());
	//	std::uniform_int_distribution<int> distribution(1, 1000);
	//	BackString name;
	//	do
	//	{
	//		name = std::format("{}.bbf", distribution(generator));
	//	} while (pathExists(doorPat / name));
	//	doorPat /= name;

	//	GeoBarCloudCache writer;
	//	writer.openWrite(doorPat.string());
	//	writer.save(curBar, 0);

	//	if (img)
	//	{
	//		doorPat = doorPat.replace_extension("jpg");
	//		imwrite(doorPat, *img);
	//	}
	//}
};
