#pragma once

#include <bitset>
#include "barcodeCreator.h"
#include <fstream>
#include <functional>
#include <sstream>

struct uint_6bit
{
	// simple cases
	uchar x1 : 1;
	uchar x2 : 1;
	uint8_t v : 6;
	uint_6bit(uint v = 0) : x1(0), x2(0), v(v) {}

	const char *ptr() { return reinterpret_cast<const char *>(this); }
};

struct uint_14bit
{
	// simple cases
	uint16_t x1 : 1;
	uint16_t x2 : 1;
	uint16_t v : 14;
	uint_14bit(uint val = 0) : x1(0), x2(1), v(val) {}

	const char *ptr() { return reinterpret_cast<const char *>(this); }
};

struct uint_22bit
{
	// simple cases
	uchar x1 : 1;
	uchar x2 : 1;
	uchar v0 : 6, v1, v2;
	uint_22bit(uint val) : x1(1), x2(0) { set(val); }

	void set(uint val)
	{
		v0 = (uchar)((val >> 16) & 0xff);
		v1 = (uchar)((val >> 8));
		v2 = (uchar)(val & 0xff);
	}

	uint get()
	{
		uint out = v0 << 16;
		out = out | (v1 << 8);
		out = out | v1;
		return out;
	}

	const char *ptr()
	{
		return reinterpret_cast<const char *>(this);
	}
};

struct uint_30bit
{
	// simple cases
	uint x1 : 1;
	uint x2 : 1;
	uint v : 30;
	uint_30bit(uchar val) : x1(1), x2(1), v(val)
	{}

	const char *ptr()
	{
		return reinterpret_cast<const char *>(this);
	}
};

static_assert(sizeof (uint_6bit) == 1, "not 1 byte");
static_assert(sizeof (uint_14bit) == 2, "not 2 byte");
static_assert(sizeof (uint_22bit) == 3, "not 3 byte");
static_assert(sizeof (uint_30bit) == 4, "not 4 byte");


class BarcodeHolder
{
public:
	bc::barlinevector lines;
	bc::barvector matrix;
	int depth;
	int getDeath()
	{
		return depth;
	}
	void cloneLines(bc::barlinevector& out) const
	{
		for (size_t var = 0; var < lines.size(); ++var)
		{
			out.push_back(lines[var]->clone(false));
		}
	}
	~BarcodeHolder()
	{
		for (size_t var = 0; var < lines.size(); ++var)
		{
			delete lines[var];
		}
		lines.clear();
	}
};



class BarcodesHolder
{
public:
	std::vector<BarcodeHolder*> lines;

	~BarcodesHolder()
	{
		for (size_t var = 0; var < lines.size(); ++var)
		{
			delete lines[var];
		}
		lines.clear();
	}
};


//class BinState
//{
//	std::fstream* stream;

//public:
//	using act = uint;

//	int pType() = 0;
//	act pInt(act value) = 0;
//	size_t pInt64(size_t value) = 0;
//	Barscalar pBarscalar(const Barscalar& value) = 0;

//	template<class D, class T<D>>
//	D pElement(const T& array, const size_t& index) = 0;
//	template<class D, class T<D*>>
//	D pElementDynamic(const T& array, const size_t& index) = 0;

//	act pArray(act arrSize) = 0;
//	act pFixedArray(act arraySize, act elementSize) = 0;
//	void endFixedArray() = 0;

//	void close()
//	{
//		delete stream;
//	}
//	bc::Baritem* getBaritem() = 0;
//};


//class BinStateRead : public BinState
//{
//	std::function<Barscalar(std::fstream* stream)> parseBarscal;
//	QByteArray raw;

//	std::fstream* backup = nullptr;
//	std::uniqe_ptr<bc::Baritem> itemPtr;
//public:

//	BinStateRead()
//	{
//		itemPtr = new bc::Baritem();
//	}

//	int pType() override
//	{
//		int ysize = 0;
//		bc::BarType bt = (bc::BarType)pInt(0);
//		switch (bt)
//		{
//		case BarType::BYTE8_1:
//			parseBarscal = [](std::fstream *stream) {
//				Barscalar scal;
//				scal.type = BarType::BYTE8_1;
//				stream->read((char *) &scal.data.b1, 1);
//				return scal;
//			};
//			ysize = 1;
//			break;
//		case BarType::BYTE8_3:
//			parseBarscal = [](std::fstream *stream) {
//				Barscalar scal;
//				scal.type = BarType::BYTE8_3;
//				stream->read((char *) scal.data.b3, 3);
//				return scal;
//			};
//			ysize = 3;
//			break;
//		case BarType::FLOAT32_1:
//			break;
//			parseBarscal = [](std::fstream *stream) {
//				Barscalar scal;
//				scal.type = BarType::FLOAT32_1;
//				(*stream) >> scal.data.f;
//				return scal;
//			};
//			ysize = 4;
//		default:
//			throw std::exception();
//			break;
//		}

//		return ysize;
//	}

//	act pInt(act)
//	{
//		act vale;
//		*stream >> vale;
//		return vale;
//	}

//	size_t pInt64(size_t)
//	{
//		size_t vale;
//		*stream >> vale;
//		return vale;
//	}

//	Barscalar pBarscalar(const Barscalar&) override
//	{
//		return parseBarscal(stream);
//	}

//	template<class D, class T<D>>
//	D pElement(const T&, const size_t&)
//	{
//		return D();
//	}

//	template<class D, class T<D*>>
//	D pElementDynamic(const T&, const size_t&)
//	{
//		return new D();
//	}

//	act pArray(act arrSize)
//	{
//		return pInt(arrSize);
//	}

//	act pFixedArray(act arraySize, act elementSize)
//	{
//		raw.resize(arraySize * elementSize);
//		stream->read(raw.data(), raw.length());
//		assert(backup == nullptr);
//		backup = stream;
//		stream = new linestream(&raw, QIODevice::ReadOnly);
//	}

//	void endFixedArray()
//	{
//		assert(backup);
//		delete stream;
//		stream = backup;
//	}

//	bc::Baritem* getBaritem()
//	{
//		return baritemPtr.ptr();
//	}

//	void release()
//	{
//		baritemPtr.release();
//	}
//};


//class BinStateWrite
//{
//	std::function<void(std::fstream & stream, const Barscalar &scal)> valueFunction;

//public:
//	bc::Baritem* item;

//	int pType()
//	{
//		BarType bt = item->barlines[0]->matr[0].value.type;
//		switch (bt)
//		{
//		case BarType::BYTE8_1:
//			valueFunction = [](std::fstream *stream, const Barscalar &scal) { stream->writeRawData((const char *) &scal.data.b1, 1); };
//			break;
//		case BarType::BYTE8_3:
//			valueFunction = [](std::fstream *stream, const Barscalar &scal) { stream->writeRawData((const char *) scal.data.b3, 3); };
//			break;
//		case BarType::FLOAT32_1:
//			break;
//			valueFunction = [](std::fstream *stream, const Barscalar &scal) { stream << scal.data.f; };
//		default:
//			break;
//		}

//		return 0;
//	}

//	act pInt(act value)
//	{
//		*stream << value;
//		return value;
//	}
//	size_t pInt64(size_t value)
//	{
//		*stream << value;
//		return value;
//	}
//	Barscalar pBarscalar(const Barscalar& value)
//	{
//		valueFunction(stream, value);
//		return value;
//	}

//	template<class D, class T<D>>
//	D pElement(const T& array, const size_t& index)
//	{
//		return array[index];
//	}
//	template<class D, class T<D*>>
//	D pElementDynamic(const T& array, const size_t& index)
//	{
//		return array[index];
//	}

//	act pArray(act arrSize)
//	{
//		return pInt(arrSize);
//	}
//	act pFixedArray(act arraySize, act elementSize)
//	{
//		return pArray(arraySize);
//	}
//	void endFixedArray()
//	{ }

//	bc::Baritem* getBaritem()
//	{

//	}
//};

//struct BarBinFile
//{
//private:
//	std::fstream stream;
//	QFile binFile;

//	bool writeMode;

//	bool openRead(BackString path)
//	{
//		writeMode = false;
//		binFile.setFileName(path);
//		stream.setDevice(&binFile);

//		bool b = binFile.open(QIODevice::ReadOnly);
//		if (b)
//		{
//			uint size;
//			stream >> size;
//			for (uint i = 0; i < size; ++i)
//			{
//				size_t off;
//				stream >> off;
//				memoffs.push_back(off);
//			}
//			return true;
//		}
//		else
//			return false;
//	}

//	bool openWrite(BackString path, int maxBufferSize = 10000)
//	{
//		writeMode = true;
//		binFile.setFileName(path);
//		stream.setDevice(&binFile);

//		return binFile.open(QIODevice::WriteOnly);
//	}
//	bool ended() { return binFile.atEnd(); }

//	using act = uint;


//	bc::Baritem *process(int &index)
//	{
//		bc::Baritem rsitem = state->getBaritem();

//		state->pInt(index); // Index
//		int typeSize = state->pType(); // BarType

//		bc::barlinevector &vec = rsitem->barlines;

//		size_t linesCount =  state->pArray(vec);
//		for (size_t i = 0; i < linesCount; ++i)
//		{
//			bc::barline *&line = state->pElement(vec, i);

//			if (!state->validate(line))
//				continue;

//			uint id = state->pInt(i);
//			line->start = state->pScalar(line->start);
//			line->setEnd(state->pScalar(line->end()));

//			act matrSize = sate->pFixedArray(line->matr, 4 + typeSize);
//			for (act j = 0; j < matrSize; ++j)
//			{
//				bc::barvalue& v = state->pElement(line->matr, i);
//				v.index = state->pInt(v.index);
//				v.value = state->pBarscalar(v.value);
//			}
//			state->endFixedArray();

//			auto childrenFull = state->getFullChildrenArray();
//			uint childrenSize = sate->pFixedArray(childrenFull, typeSize * 2);
//			for (size_t jk = 0; jk < childrenSize; ++jk)
//			{
//				bc::barline* &cline = state->pElement(childrenFull, jk)
//				cline->start = state->pBarscalar(cline->start);
//				cline->setEnd(state->pBarscalar(cline->end()));
//			}
//			state->endFixedArray();
//		}

//		return rsitem;
//	}
//}

struct BarBinFile
{
public:
	using act = uint;

private:
	std::fstream stream;
	//QFile binFile;

	bool writeMode;

	uint readInt(std::iostream& stream)
	{
		uint vale;
		stream >> vale;
		return vale;

		uchar data[4]{0, 0, 0, 0};
		stream >> data[0];

		std::bitset<2> flag(data[0]);
		int asf = flag.to_ulong();
		switch (asf)
		{
		case 0:
			return static_cast<uint_6bit>(data[0]).v;
		case 1:
			stream >> data[1];
			return static_cast<uint_6bit>(*data).v;
		case 2:
			stream >> data[1];
			stream >> data[2];
			return static_cast<uint_22bit>(*data).get();
		case 3:
			stream.read((char *) data + 1, 3);
			return static_cast<uint_30bit>(*data).v;
		default:
			throw;
		}
	}

	void writeInt(std::fstream& stream, uint val)
	{
		stream << val;
		return;

		if (val < 64)
		{
			uint_6bit v(val);
			stream.write(v.ptr(), 1);
		}
		else if (val < 16384)
		{
			uint_14bit v(val);
			stream.write(v.ptr(), 2);
		}
		else if (val < 16777216)
		{
			uint_22bit v(val);
			stream.write(v.ptr(), 3);
		}
		else
		{
			uint_30bit v(val);
			stream.write(v.ptr(), 4);
		}
	}

public:
	bool openRead(const std::string& path)
	{
		writeMode = false;
		//binFile.setFileName(path);
		stream.open(path, std::ios::in);

		if (stream.is_open())
		{
			uint size;
			stream >> size;
			for (uint i = 0; i < size; ++i)
			{
				size_t off;
				stream >> off;
				memoffs.push_back(off);
			}
			return true;
		}
		else
			return false;
	}

	bool openWrite(std::string path, int maxBufferSize = 10000)
	{
		writeMode = true;
		stream.open(path, std::ios::out);

		return stream.is_open();
	}


	std::vector<size_t> memoffs;
	void writeHeaderProto(int iteemsSize)
	{
		stream << (uint) iteemsSize;
		for (int i = 0; i < iteemsSize; ++i)
		{
			size_t memOff = 0;
			stream << memOff;
		}
	}

	struct sets
	{
		int totalSize;
	};

	bool ended()
	{ 
		return stream.eof();
	}

	BarcodeHolder* readItem(const int index)
	{
		for (size_t var = 0; var < memoffs.size(); ++var)
		{
			// seekg for input
			stream.seekg(memoffs[var]);

			int sindex;
			stream >> sindex;
			if (sindex == index)
			{
				stream.seekg(memoffs[var]);
				BarcodesHolder tmp = read(sindex);
				assert(tmp.lines.size() > 0);
				auto *line = tmp.lines[0];
				tmp.lines.clear();
				return line;
			}
		}
		return nullptr;
	}

	BarcodesHolder read(int &index)
	{
		std::unordered_map<uint, bc::barline *> ids;

		stream >> index;
		//qDebug() << "ID:" << index;

		BarType bt;
		int btRaw;
		stream >> btRaw;
		bt = (BarType) btRaw;

		BarcodesHolder rsitem;

		std::function<Barscalar(std::iostream& stream)> parseBarscal;

		int ysize;
		switch (bt)
		{
		case BarType::BYTE8_1:
			parseBarscal = [](std::iostream& stream) {
				Barscalar scal;
				scal.type = BarType::BYTE8_1;
				stream.read((char *) &scal.data.b1, 1);
				return scal;
			};
			ysize = 1;
			break;
		case BarType::BYTE8_3:
			parseBarscal = [](std::iostream& stream) {
				Barscalar scal;
				scal.type = BarType::BYTE8_3;
				stream.read((char *) scal.data.b3, 3);
				return scal;
			};
			ysize = 3;
			break;
		case BarType::FLOAT32_1:
			break;
			parseBarscal = [](std::iostream& stream) {
				Barscalar scal;
				scal.type = BarType::FLOAT32_1;
				stream >> scal.data.f;
				return scal;
			};
			ysize = 4;
		default:
			break;
		}

		auto &vec = rsitem.lines;
		size_t vecSize;
		stream >> vecSize;

		for (size_t i = 0; i < vecSize; ++i)
		{
			BarcodeHolder *barlines = new BarcodeHolder();
			vec.push_back(barlines);

			uint id = readInt(stream);					// Read 4 (id)
			Barscalar start = parseBarscal(stream);		// Read barscalar
			Barscalar end = parseBarscal(stream);		// Read barscalar
			barlines->depth = readInt(stream);			// Read 4 (id)

			bc::barline *line = new bc::barline(start, end, 0);
			barlines->lines.push_back(line);

//			ids.insert(id, line);
//			qDebug() << "ID:" << id;

			act arrSize = readInt(stream);				// Read 4 (N)
			if (arrSize > 0)
			{
				std::vector<char> raw;
				raw.resize(arrSize * (4 + ysize));
				stream.read(raw.data(), raw.size());
				std::stringstream linestream;
				linestream.write(raw.data(), raw.size());

				for (act j = 0; j < arrSize; ++j)
				{
					uint index = readInt(linestream);			// Read 4 * N
					Barscalar val = parseBarscal(linestream);	// Read barscalar * N

					bc::barvalue v(bc::barvalue::getStatPoint(index), val);
					barlines->matrix.push_back(v);
				}
			}

			act arrSize2 = readInt(stream);			// Read 4 (N)
			//qDebug() << "arr:" << arrSize2;
			for (size_t jk = 0; jk < arrSize2; ++jk)
			{
				Barscalar start2 = parseBarscal(stream);	// Read barscalar * N
				Barscalar end2 = parseBarscal(stream);		// Read barscalar * N

				bc::barline *line2 = new bc::barline(start2, end2, 0);
				barlines->lines.push_back(line2);
			}
		}

//		for (size_t i = 0; i < vecSize; ++i)
//		{
//			uint chlSize = readInt(stream);

//			//			QByteArray raw2;
//			//			raw2.resize(chlSize * (sizeof(size_t)));
//			//			stream.read(raw2.data(), raw2.length());
//			//			std::fstream ctream(&raw2, QIODevice::ReadOnly);

//			auto *prnt = vec[i];
//			for (act j = 0; j < chlSize; ++j)
//			{
//				uint idc = readInt(stream);
//				assert(ids.contains(idc));
////				if (ids.contains(idc))
//				ids[idc]->setparent(prnt);
//			}
//		}

		return rsitem;
	}

	void write(const bc::Baritem *item, int index, sets& set)
	{
		std::unordered_map<bc::barline *, uint> ids;
//		uint counter = 0;

		memoffs.push_back((size_t) stream.tellp());

		stream << index;
		BarType bt = item->barlines[0]->matr[0].value.type;
		stream << (int) bt;

		std::function<void(std::fstream & stream, const Barscalar &scal)> valueFunction;

		switch (bt)
		{
		case BarType::BYTE8_1:
			valueFunction = [](std::fstream &stream, const Barscalar &scal) { stream.write((const char *) &scal.data.b1, 1); };
			break;
		case BarType::BYTE8_3:
			valueFunction = [](std::fstream &stream, const Barscalar &scal) { stream.write((const char *) scal.data.b3, 3); };
			break;
		case BarType::FLOAT32_1:
			break;
			valueFunction = [](std::fstream &stream, const Barscalar &scal) { stream << scal.data.f; };
		default:
			break;
		}

		size_t afterPos = stream.tellp();
		const bc::barlinevector &vec = item->barlines;
		stream << vec.size();

		size_t realIndex = 0;
		for (size_t i = 0; i < vec.size(); ++i)
		{
			bc::barline *line = vec[i];

			float proc = 100.f * (float) line->matr.size() / set.totalSize;
//			if (proc < 0.01 || proc > 10)
//			{
//				continue;
//			}

			uint rid = realIndex++;
			ids.insert(std::make_pair(line, rid));

			writeInt(stream, rid);						// Write int (id)
			valueFunction(stream, line->start);			// Write barscalar
			valueFunction(stream, line->end());			// Write barscalar
			writeInt(stream, line->getDeath());			// Write int (depth)

			auto &arr = line->matr;
			assert(arr.size() < 4294967295);
			writeInt(stream, (act)arr.size());		// Write 4 bytes (N)
			for (act j = 0; j < arr.size(); ++j)
			{
				writeInt(stream, arr[j].index);			// Write 4 * N
				valueFunction(stream, arr[j].value);	// Write barscalar * N
			}

			{
				BarcodeHolder barhold;

				line->getChilredAsList(barhold.lines, false, false);
				assert(barhold.lines.size() < 4294967295);
				act arrSize2 = barhold.lines.size();
				writeInt(stream, arrSize2); 	// Write 4 bytes (N)
				//qDebug() << arrSize2;

				for (size_t jk = 0; jk < arrSize2; ++jk)
				{
					bc::barline *line2 = barhold.lines[jk];

					valueFunction(stream, line2->start);	// Write barscalar * N
					valueFunction(stream, line2->end());	// Write barscalar * N
				}
				barhold.lines.clear();
			}
		}

		size_t curPos = stream.tellp();
		stream.seekp(afterPos);
		stream << realIndex;
		stream.seekp(curPos);

//		for (size_t i = 0; i < vec.size(); ++i)
//		{
//			bc::barline *line = vec[i];
//			auto &chl = line->children;
//			act chlSize = chl.size();
//			writeInt(stream, chlSize);
//			for (act i = 0; i < chlSize; ++i)
//			{
//				assert(ids.contains(chl[i]));
//				writeInt(stream, ids[chl[i]]);
//			}
//		}
	}

	void close()
	{
		if (writeMode)
		{
			stream.seekp(0);

			stream << (uint) memoffs.size();
			for (size_t i = 0; i < memoffs.size(); ++i)
			{
				stream << (size_t) memoffs[i];
			}
			writeMode = false;
		}
		stream.close();
	}
};
