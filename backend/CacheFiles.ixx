module;

#include <string>
#include "../Bind/Common.h"

// #include "project.h"

export module CacheFilesModule;

import BarholdersModule;
import BarcodeModule;

using uint = unsigned int;


// export template<class TItem, class THolder>
// class GeoBar
// {
// protected:
// 	std::unique_ptr<StateBinFile::BinState> state;
// public:

// 	void openRead(const BackPathStr& str)
// 	{
// 		state.reset(new StateBinFile::BinStateReader());
// 		if (!state->open(str.string()))
// 			throw;
// 	}

// 	void openWrite(const BackPathStr& str)
// 	{
// 		state.reset(new StateBinFile::BinStateWriter());
// 		if (!state->open(str.string()))
// 			throw;
// 	}


// 	void openRead(std::istringstream& str)
// 	{
// 		state.reset(new StateBinFile::BinStateReader(str));
// 	}

// 	void openWrite(std::ostringstream& str)
// 	{
// 		state.reset(new StateBinFile::BinStateWriter(str));
// 	}

// 	TItem* load(int& index)
// 	{
// 		assert(state->isReading());
// 		return saveLoadBar(index, new TItem());
// 	}

// 	TItem* loadSpecific(int index)
// 	{
// 		assert(state->isReading());
// 		dynamic_cast<StateBinFile::BinStateReader*>(state.get())->moveIndex(index);
// 		return saveLoadBar(index, new TItem());
// 	}

// 	void save(TItem* item, int index)
// 	{
// 		assert(!state->isReading());
// 		saveLoadBar(index, item);
// 	}

// 	bool canRead() const
// 	{
// 		return !state->ended();
// 	}

// 	void close()
// 	{
// 		state.reset(nullptr);
// 	}

// private:
// 	virtual TItem* saveLoadBar(int& index, TItem* rsitem = NULL) = 0;
// };


// export class GeoBarRasterCache : public GeoBar<bc::Baritem, bc::Barcontainer>
// {
// private:
// 	bc::Baritem* saveLoadBar(int& index, bc::Baritem* rsitem = NULL)
// 	{
// 		bc::barlinevector& vec = rsitem->barlines;
// 		const bool isReading = state->isReading();

// 		state->beginItem();

// 		index = state->pInt(index); // Index
// 		rsitem->setType((BarType)state->pType(rsitem->getType())); // BarType
// 		size_t linesCount = state->pArray(vec.size());

// 		// Parent read/write stuff
// 		uint counterId = 0;
// 		std::vector<bc::barline*> ids;
// 		ids.resize(linesCount * 2);
// 		std::fill(ids.begin(), ids.end(), nullptr);
// 		MMMAP<size_t, uint> writeIds;

// 		// Begin
// 		state->beginArray(vec, linesCount);
// 		for (size_t i = 0; i < linesCount; ++i)
// 		{
// 			bc::barline* line;
// 			if (!isReading)
// 			{
// 				// Write
// 				line = vec[i];

// 				uint couId;
// 				auto p = writeIds.find((size_t)line);
// 				if (p != writeIds.end())
// 				{
// 					couId = p->second;
// 				}
// 				else
// 				{
// 					couId = counterId++;
// 					writeIds.insert({ (size_t)line, couId });
// 				}

// 				state->pInt(couId);

// 				uint parId;
// 				p = writeIds.find((size_t)line->parent);
// 				if (p != writeIds.end())
// 				{
// 					parId = p->second;
// 				}
// 				else
// 				{
// 					parId = counterId++;
// 					writeIds.insert({ (size_t)line->parent, parId });
// 				}

// 				state->pInt(parId);
// 			}
// 			else
// 			{
// 				// Read

// 				// Main line
// 				uint couId = state->pInt(0);
// 				line = ids[couId];
// 				if (line == nullptr)
// 				{
// 					line = new bc::barline();
// 					ids[couId] = line;
// 				}
// 				vec[i] = line;

// 				// Parent
// 				uint parId = state->pInt(0);
// 				bc::barline* par = ids[parId];
// 				if (par == nullptr)
// 				{
// 					par = new bc::barline();
// 					ids[parId] = par;
// 				}

// 				line->setparent(par);
// 			}

// 			line->start = state->pBarscalar(line->start);
// 			line->m_end = state->pBarscalar(line->end());

// 			//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
// 			uint matrSize = state->pArray(line->matr.size());
// 			state->beginArray(line->matr, matrSize);
// 			for (uint j = 0; j < matrSize; ++j)
// 			{
// 				bc::barvalue& v = line->matr[j];
// 				v.index = state->pInt(v.index);
// 				v.value = state->pBarscalar(v.value);
// 			}
// 		}
// 		state->endItem();

// 		return rsitem;
// 	}
// };


// export class GeoBarRasterClassiferCache : public GeoBar <bc::Baritem, bc::Barcontainer>
// {
// private:
// 	bc::Baritem* saveLoadBar(int& index, bc::Baritem* rsitem = NULL)
// 	{
// 		state->beginItem();

// 		state->pInt(index); // Index
// 		rsitem->setType((BarType)state->pType(rsitem->getType())); // BarType

// 		uint realIndex = 0;
// 		bc::barlinevector& vec = rsitem->barlines;
// 		MMMAP<size_t, uint> ids;

// 		size_t linesCount = state->pArray(vec.size());
// 		state->beginArray(vec, linesCount);
// 		for (size_t i = 0; i < vec.size(); ++i)
// 		{
// 			bc::barline* line = vec[i];

// 			if (state->isReading())
// 			{
// 				line = new bc::barline();
// 			}

// 			//rid = state->pInt(rid);						// Write int (id)
// 			line->start = state->pBarscalar(line->start);			// Write barscalar
// 			line->m_end = state->pBarscalar(line->end());			// Write barscalar
// 			state->pInt(line->getDeath());			// Write int (depth)

// 			assert(line->matr.size() < 4294967295);
// 			uint matrSize = state->pArray(line->matr.size());
// 			state->beginArray(line->matr, matrSize);
// 			for (uint j = 0; j < matrSize; ++j)
// 			{
// 				bc::barvalue& v = line->matr[j];
// 				v.index = state->pInt(v.index);
// 				v.value = state->pBarscalar(v.value);
// 			}

// 			BarcodeHolder barhold;
// 			line->getChilredAsList(barhold.lines, false, false);
// 			assert(barhold.lines.size() < 4294967295);

// 			size_t arrSize2 = state->pArray(barhold.lines.size());
// 			for (size_t jk = 0; jk < arrSize2; ++jk)
// 			{
// 				bc::barline* line2 = barhold.lines[jk];

// 				line2->start = state->pBarscalar(line2->start);	// Write barscalar * N
// 				line2->m_end = state->pBarscalar(line2->end());	// Write barscalar * N
// 			}
// 			barhold.lines.clear();
// 		}
// 		state->endItem();

// 		return rsitem;
// 	}
// };

// export class GeoBarCloudCache : public GeoBar<BarcodeHolder, BarcodesHolder>
// {
// private:
// 	BarcodeHolder* saveLoadBarBody(BarcodeHolder* rsitem = NULL)
// 	{
// 		bc::barlinevector& vec = rsitem->lines;

// 		size_t linesCount = state->pArray(vec.size());
// 		state->beginArray(vec, linesCount);
// 		for (size_t i = 0; i < linesCount; ++i)
// 		{
// 			if (state->isReading())
// 				vec[i] = new bc::barline();

// 			bc::barline* line = vec[i];

// 			state->pInt(i);
// 			line->start = state->pBarscalar(line->start);
// 			line->m_end = (state->pBarscalar(line->end()));
// 		}

// 		//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
// 		uint matrSize = state->pArray(rsitem->matrix.size());
// 		state->beginArray(rsitem->matrix, matrSize);
// 		for (uint j = 0; j < matrSize; ++j)
// 		{
// 			bc::barvalue& v = rsitem->matrix[j];
// 			v.index = state->pInt(v.index);
// 			v.value = state->pBarscalar(v.value);
// 		}

// 		return rsitem;
// 	}

// 	BarcodeHolder* saveLoadBar(int& index, BarcodeHolder* rsitem = NULL)
// 	{
// 		state->beginItem();
// 		state->pInt(index);
// 		state->pType(rsitem->matrix.size() > 0 ? rsitem->matrix[0].value.type : BarType::FLOAT32_1);

// 		rsitem = saveLoadBarBody(rsitem);
// 		state->endItem();
// 		return rsitem;
// 	}

// public:

// 	CloudItem* saveLoadBars(int& index, CloudItem* rsitem = NULL)
// 	{
// 		state->beginItem();
// 		index = state->pInt(index);
// 		auto& lines = rsitem->lines;
// 		state->pType(lines.size() > 0 && lines[0]->matrix.size() > 0 ? lines[0]->matrix[0].value.type : BarType::FLOAT32_1);

// 		uint linesSize = state->pArray(rsitem->lines.size());
// 		state->beginArray(rsitem->lines, linesSize);
// 		for (size_t i = 0; i < linesSize; i++)
// 		{
// 			if (state->isReading())
// 				rsitem->lines[i] = new BarcodeHolder();

// 			saveLoadBarBody(rsitem->lines[i]);
// 		}
// 		state->endItem();
// 		return rsitem;
// 	}
// };



// export class GeoBarCloudHolderCache : public GeoBar<::BarcodesHolder, ::BarcodesHolder>
// {
// private:
// 	BarcodeHolder* saveLoadBarBody(BarcodeHolder* rsitem = NULL)
// 	{
// 		uint counterId = 0;
// 		std::vector<bc::barline*> ids;
// 		MMMAP<size_t, uint> writeIds;

// 		bc::barlinevector& vec = rsitem->lines;
// 		size_t linesCount = state->pArray(vec.size());

// 		ids.resize(linesCount * 2);
// 		std::fill(ids.begin(), ids.end(), nullptr);

// 		const bool isReading = state->isReading();

// 		state->beginArray(vec, linesCount);
// 		for (size_t i = 0; i < linesCount; ++i)
// 		{
// 			uint couId;
// 			uint parId;
// 			if (!isReading)
// 			{
// 				auto p = writeIds.find((size_t)vec[i]);
// 				if (p != writeIds.end())
// 				{
// 					couId = p->second;
// 				}
// 				else
// 					couId = counterId++;

// 				state->pInt(couId);

// 				p = writeIds.find((size_t)vec[i]->parent);
// 				if (p != writeIds.end())
// 				{
// 					parId = p->second;
// 				}
// 				else
// 					parId = counterId++;

// 				state->pInt(parId);
// 			}
// 			else
// 			{
// 				couId = state->pInt(0);

// 				bc::barline* p = ids[couId];
// 				if (p)
// 				{
// 					vec[i] = p;
// 				}
// 				else
// 					vec[i] = new bc::barline();

// 				parId = state->pInt(0);

// 				p = ids[parId];
// 				bc::barline* par;
// 				if (p)
// 				{
// 					par = p;
// 				}
// 				else
// 					par = new bc::barline();

// 				vec[i]->setparent(par);
// 			}

// 			bc::barline* line = vec[i];
// 			line->start = state->pBarscalar(line->start);
// 			line->m_end = (state->pBarscalar(line->end()));
// 		}


// 		rsitem->depth = state->pInt(rsitem->depth);

// 		//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
// 		uint matrSize = state->pArray(rsitem->matrix.size());
// 		state->beginArray(rsitem->matrix, matrSize);
// 		for (uint j = 0; j < matrSize; ++j)
// 		{
// 			bc::barvalue& v = rsitem->matrix[j];
// 			v.index = state->pInt(v.index);
// 			v.value = state->pBarscalar(v.value);
// 		}

// 		return rsitem;
// 	}

// 	BarcodesHolder* saveLoadBar(int& index, BarcodesHolder* rsitem = NULL)
// 	{
// 		state->beginItem();
// 		index = state->pInt(index);
// 		auto& lines = rsitem->lines;
// 		state->pType(lines.size() > 0 && lines[0]->matrix.size() > 0 ? lines[0]->matrix[0].value.type : BarType::FLOAT32_1);

// 		uint linesSize = state->pArray(rsitem->lines.size());
// 		state->beginArray(rsitem->lines, linesSize);
// 		for (size_t i = 0; i < linesSize; i++)
// 		{
// 			if (state->isReading())
// 				rsitem->lines[i] = new BarcodeHolder();

// 			saveLoadBarBody(rsitem->lines[i]);
// 		}
// 		state->endItem();
// 		return rsitem;
// 	}
// };
