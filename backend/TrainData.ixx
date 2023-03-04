module;

#include <vector>
#include <iostream>

#include "../Bind/Common.h"
#include "../side/sqlite/sqlite3.h"

export module TrainIO;
import BarcodeModule;
import JsonCore;
import IOCore;
import BarholdersModule;
import BackTypes;
import Platform;


//enum ClassCategType
//{
//	CCT_base
//};
class IBarClassCategor
{
public:
	int id;
	BackString name;

	//virtual void load(const BackJson& json) = 0;
	//virtual BackJson save() = 0;
};


class BarClassCategor : public IBarClassCategor
{
public:
};

class IBarCategories
{
	virtual int addValue(const BackString& nname) = 0;

};

export class BarCategories
{
	int counter;

public:
	std::vector<BarClassCategor> categs;

	int addValue(const BackString& nname)
	{
		int id = counter++;

		categs.push_back({ id, nname });
		return id;
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
			categ.categs.push_back({ 0, "Bad" });
			categ.categs.push_back({ 1, "Kurgan"});
			categ.counter = 2;

			return categ;
		}

		//barclassificator barclas;
		BackJson loadDoc = jsonFromFile(path.string());// Project::getPathSt(BackPath::classifier));

		categ.counter = loadDoc["counter"].asInt();
		JsonArray list = loadDoc["categories"];
		for (auto item : list)
		{
			const BackJson& catId = item;
			categ.categs.push_back({ catId["id"].asInt(), catId["name"].asString() });
		}

		//auto loadColback = [&categ](int classId, const BackString& name)
		//{
		//	categ.value.push_back(classId);
		//	categ.name.push_back(name);
		//};

		return categ;
	}

	void saveCategories()
	{
		BackPathStr path = "";// Project::getProject()->getPath(BackPath::classifier);

		JsonArray arr;
		for (size_t i = 0; i < categs.size(); i++)
		{
			JsonObject catId;
			catId["id"] = categs[i].id;
			catId["name"] = categs[i].name;
			arr.append(catId);
		}

		BackJson loadDoc;
		loadDoc["counter"] = counter;
		loadDoc["categories"] = arr;
		jsonToFile(loadDoc, path);
	}
};

class ClassData
{
	int categId;
	BarcodeHolder curBar;
};

class ClassDataCacher
{
	sqlite3* db = nullptr;

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

	using TrainCallback = std::function<void(int classId, vbuffer bbfFile, BackImage preview, size_t localId)>;

	enum LoadField : unsigned char
	{
		LF_BINFILE = 1,
		LF_ICON = 2,
		LF_ALL = 255
	};

	void loadAll(TrainCallback callback, int classId = -1, LoadField filter = LF_ALL)
	{
		sqlite3_stmt* stmt = prepareSelect(filter, classId);

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			vbuffer bff;
			BackImage img(1,1,1);
			size_t locId;
			int classId;

			loadEnd(stmt, filter, classId, bff, img, locId);
			callback(classId, bff, img, locId);
		}

		sqlite3_finalize(stmt);
	}

private:
	BackString prepareSelectBase(LoadField filter = LF_ALL)
	{
		BackString statement = "SELECT ";

		if (filter == LF_ALL)
		{
			statement += " * ";
		}
		else
		{
			bool added = true;
			statement += " class_id, ";
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
			}
		}
		statement += " FROM CLASS_DATA";
		return statement;
	}

	sqlite3_stmt* prepareSelect(LoadField filter = LF_ALL, int classId = -1)
	{
		BackString statement = prepareSelectBase(filter);

		if (classId != -1)
		{
			statement += " WHERE rowid = ?";
		}
		else
			statement += ";";

		sqlite3_stmt* stmt;
		auto rc = sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, nullptr);

		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
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
		auto rc = sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, nullptr);

		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
			return nullptr;
		}

		if (localId != -1)
			sqlite3_bind_int64(stmt, 1, localId); // bind the rowid value to the first placeholder

		return stmt;
	}

	void loadEnd(sqlite3_stmt* stmt, LoadField filter, int& classId, vbuffer& bbfFile, BackImage& preview, size_t& locId)
	{
		classId = sqlite3_column_int(stmt, 0);
		int counter;
		locId = sqlite3_column_int64(stmt, 0); // rowid
		int counterId = 1;

		bbfFile.release();
		preview = BackImage(1,1,1);
		if (filter & LF_BINFILE)
		{
			const void* file = sqlite3_column_blob(stmt, counterId );
			int binfileSize = sqlite3_column_bytes(stmt, counterId );
			bbfFile.copyDataFrom(reinterpret_cast<const uchar*>(file), binfileSize);

			++counterId;
		}
		if (filter & LF_ICON)
		{
			StaticArray<const uchar> imgBuff;

			const void* imgData = sqlite3_column_blob(stmt, counterId);
			int imgSize = sqlite3_column_bytes(stmt, counterId);

			preview = imreadFromMemory(reinterpret_cast<const uchar*>(imgData), imgSize);
			++counterId;
		}
	}

public:
	int save(int classId, const vbuffer& bbfFile, BackImage* preview)
	{
		const char* SQL = "CREATE TABLE IF NOT EXISTS ClassData(id, bbf, png)";
// INSERT INTO FOO VALUES(1,2,3); INSERT INTO FOO SELECT * FROM FOO;";
		char* err = 0;

		if (sqlite3_exec(db, SQL, 0, 0, &err))
		{
			fprintf(stderr, "Ошибка SQL: %sn", err);
			sqlite3_free(err);
		}

		sqlite3_stmt* stmt = NULL;

		const char* statement = "INSERT INTO CLASS_DATA VALUES (?, ?, ?);";

		auto rc = sqlite3_prepare_v2(db, statement, 0, &stmt, NULL);

		rc = sqlite3_bind_blob(stmt, 1, bbfFile.data(), bbfFile.size(), SQLITE_TRANSIENT);

		if (preview)
		{
			vbuffer imgPrev = imwriteToMemory(*preview);
			rc = sqlite3_bind_blob(stmt, 2, imgPrev.data(), imgPrev.size(), SQLITE_TRANSIENT);
		}
		else
			rc = sqlite3_bind_null(stmt, 2);


		const char* result = sqlite3_errmsg(db);
		rc = sqlite3_step(stmt);
		size_t locId = sqlite3_last_insert_rowid(db);

		return locId;
	}

	void close()
	{
		sqlite3_close(db);
	}

	~ClassDataCacher()
	{
		close();
	}
};

export class barclassificator
{
public:
	BarCategories categs;
	std::vector <bc::Barcontainer> classes;


	//void loadClasses(const BackDirStr& path, barclassificator& barclas)
	//{
	//	barclas.udpdateClasses();
	//	auto& crgs = barclas.categs.value;

	//	for (auto categ : crgs)
	//	{
	//		BackDirStr dirl = path / intToStr(categ);
	//		if (!pathExists(dirl))
	//			continue;

	//		for (auto const& entry : std::filesystem::directory_iterator(dirl))
	//		{
	//			if (!entry.is_regular_file())
	//			{
	//				continue;
	//			}

	//			auto ext = entry.path().extension();
	//			if (ext == ".bbf")
	//			{
	//				BackPathStr filename = entry.path();
	//				GeoBarCloudCache reader;
	//				reader.openRead(filename.string());
	//				int ind = 0;
	//				std::unique_ptr<BarcodeHolder> prt(reader.load(ind));
	//				//barclas.addData(categ, prt->lines, true);
	//			}
	//			else if (ext == ".jpg")
	//			{

	//			}
	//		}
	//	}
	//}


	void addData(int classInd, bc::barlinevector& cont, const bool move = true)
	{
		bc::Baritem* item = new bc::Baritem();
		if (move)
		{
			item->barlines = std::move(cont);
		}
		else
		{
			// Copy
			for (size_t j = 0; j < cont.size(); j++)
			{
				item->barlines.push_back(cont[j]->clone());
			}
		}

		item->relen();
		classes[classInd].addItem(item);
	}

	void addData(int classInd, bc::Baritem* item)
	{
		item->relen();
		classes[classInd].addItem(item);
	}


	void udpdateClasses()
	{
		for (size_t i = classes.size(); i < categs.size(); i++)
		{
			classes.push_back(bc::Barcontainer());
		}
	}

	void removeLast(int classInd)
	{
		classes[classInd].remoeLast();
	}

	////	int getType(bc::barlinevector &bar0lines)
	//int getType(const BarcodeHolder* bar0)
	//{
	//	auto cp = bc::CompireStrategy::CommonToLen;
	//	float res = 0;

	//	//		bc::Baritem *bar0 = new bc::Baritem();
	//	//		bar0->barlines = bar0lines;
	//	bc::Baritem newOne;
	//	bar0->cloneLines(newOne.barlines);
	//	//		newOne.shdowCopy = true;
	//	newOne.relen();

	//	int maxInd = -1;
	//	float maxP = res;
	//	for (size_t i = 0; i < classes.size(); i++)
	//	{
	//		float ps = classes[i].compireBest(&newOne, cp);
	//		if (ps > maxP)
	//		{
	//			maxP = ps;
	//			maxInd = i;
	//		}
	//	}

	//	//		bar0.barlines.clear();
	//	//		delete bar0;

	//	assert(maxP <= 1.0);
	//	return maxP > 0.5 ? maxInd : -1;
	//}


	~barclassificator()
	{
		//		for (size_t i = 0; i < N * 2; i++)
		//		{
		//			delete classes[i];
		//		}
	}
private:
	Barscalar asScalar(const BackJson& arr)
	{
		return Barscalar(arr[0].asDouble(), arr[1].asDouble(), arr[2].asDouble());
	}
};


//void parseItem(const JsonObject &obj, bc::barlinevector &lines)
//{
//	bc::barline *line = new bc::barline();
//	line->start = asScalar(obj["start"].array());
//	line->m_end = asScalar(obj["end"].array());

//	lines.push_back(line);

//	auto arrcoors = obj["children"].array();
//	for (size_t i = 0; i < arrcoors.size(); i++)
//	{
//		parseItem(arrcoors.at(i).object(), lines);
//	}
//}
//BarcodesHolder toHoldes(const bc::CloudPointsBarcode::CloudPoints& cloud)
//{
//	bc::CloudPointsBarcode clodCrt;
//	std::unique_ptr<bc::Barcontainer> hold(clodCrt.createBarcode(&cloud));

//	BarcodesHolder holder;
//	if (cloud.points.size() == 0)
//		return holder;

//	bc::Baritem* main = hold->getItem(0);
//	for (size_t var = 0; var < main->barlines.size(); ++var)
//	{
//		auto* line = main->barlines[var];
//		BarcodeHolder* barhold = new BarcodeHolder();
//		holder.lines.push_back(barhold);

//		barhold->matrix = std::move(line->matr);
//		line->getChilredAsList(barhold->lines, true, true, false);
//	}

//	return holder;
//}


//public:
	//void addClass(const BackPathStr& binFile, int classInd)
	//{
	//	BackString val;

	//	std::ifstream file(path.string());
	//	BackJson jsonDocument = BackJson::parse(file);

	//	// �� �������� �������� ������ � ������� ������� QJsonObject
	//	if (jsonDocument.is_array())
	//	{
	//		BackJson jsonItems = jsonDocument.array();

	//		for (size_t i = 0; i < jsonItems.size(); i++)
	//		{
	//			JsonObject jsItem = jsonItems.at(i).object();
	//			bc::Baritem *item = new bc::Baritem();
	//			parseItem(jsItem, item->barlines);
	//			item->relen();
	//			classes[classInd].addItem(item);
	//		}
	//	}
	//	else
	//	{
	//		bc::Baritem *item = new bc::Baritem();
	//		parseItem(jsonDocument.object(), item->barlines);
	//		item->relen();
	//		classes[classInd].addItem(item);
//	}
//}


//void addClass(bc::barline *line, int classInd)
//{
//	bc::Baritem *item = new bc::Baritem();
//	line->getChilredAsList(item->barlines, true, true);
//	item->relen();

//	classes[classInd].addItem(item);
//}


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

			for (auto const& entry : std::filesystem::directory_iterator(dirl))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				BackPathStr filename = entry.path().string();
				auto ext = entry.path().extension();
				if (ext == ".jpg")
				{
					callback(categ, filename);
				}
			}
		}
	}

	void save(BarcodeHolder* curBar, int classIndex, BackImage* img)
	{
		BackPathStr path = "";// Project::getProject()->getPath(BackPath::classfiles);
		BackDirStr doorPat = path / intToStr(classIndex);
		mkDirIfNotExists(doorPat);

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
	}
};
