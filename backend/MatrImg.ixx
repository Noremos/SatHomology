module;
#include <cassert>
#include <algorithm>
#include <cmath>
#include "Barcode/PrjBarlib/include/barstrucs.h"
#include "Usings.h"

#include "fpng/fpng.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include "stb_image_write.h"

export module MatrModule;

import BackBind;
// import BarTypes;
// import BarScalarModule;

export class BackImage : public bc::DatagridProvider
{
public:
	void fill(const Barscalar &s)
	{
		for (size_t i = 0, total = length(); i < total; ++i)
		{
			setLiner(i, s);
		}
	}
	typedef Barscalar *bar_iterator;
	typedef const Barscalar *const_bar_iterator;

public:
	buchar *data = nullptr;

private:
	mutable bc::CachedValue cachedMax;
	mutable bc::CachedValue cachedMin;

protected:
	int _wid;
	int _hei;
	int _channels;
	int TSize;

	bool _deleteData = false;

public:
	bool diagReverce = false;
	virtual int width() const { return _wid; }
	virtual int height() const { return _hei; }
	inline int wid() const override  { return _wid; }
	inline int hei() const override  { return _hei; }
	inline int getLineIndex(int x, int y) const { return y * _wid + x; }

protected:
	void setMetadata(int width, int height, int chnls, BarType btype = BarType::NONE)
	{
		this->_wid = width;
		this->_hei = height;
		this->_channels = chnls;
		if (btype == BarType::NONE)
		{
			if (chnls == 3)
			{
				type = BarType::BYTE8_3;
				TSize = 3;
			}
			else if (chnls == 4)
			{
				type = BarType::BYTE8_3;
				TSize = 4;
			}
			else
			{
				type = BarType::BYTE8_1;
				TSize = 1;
			}
		}
		else
		{
			type = btype;
			switch (btype)
			{
			case BarType::INT32_1:
			case BarType::FLOAT32_1:
				TSize = 4;
				assert(_channels == 1);
				break;
			case BarType::BYTE8_4:
				type = BarType::BYTE8_3;
				TSize = 4;
				assert(_channels == 4);
				break;
			case BarType::BYTE8_3:
				TSize = _channels;
				assert(_channels == 3 || _channels == 4);
				break;
			case BarType::BYTE8_1:
				TSize = 1;
				assert(_channels == 1);
				break;
			default:
				assert(false);
				break;
			}
		}

		diagReverce = false;
	}

	void valZerofy()
	{
		memset(data, 0, this->length() * TSize);
	}

	void valInit()
	{
		valDelete();
		data = new buchar[this->length() * TSize];
		_deleteData = true;
		valZerofy();
	}
	//copy
	Barscalar *valGetDeepCopy() const
	{
		Barscalar *newVals = new Barscalar[this->length()];
		memcpy(newVals, data, this->length() * TSize);
		return newVals;
	}

	void valAssignCopyOf(buchar *newData)
	{
		valDelete();
		data = new buchar[this->length() * TSize];
		_deleteData = true;
		memcpy(data, newData, this->length() * TSize);
	}

	void valDelete()
	{
		if (data != nullptr && _deleteData)
		{
			delete[] data;
			data = nullptr;
		}
		cachedMax.isCached = false;
		cachedMin.isCached = false;
	}

	void valAssignInstanceOf(buchar *newData, bool deleteData = true)
	{
		valDelete();
		_deleteData = deleteData;
		data = newData;
	}

	//std::unique_ptr<Barscalar*> data;



public:
	BackImage(int width = 1, int height = 1, int chnls = 1, BarType btype = BarType::NONE)
	{
		reinit(width, height, chnls, btype);
	}

	BackImage(int width, int height, BarType btype)
	{
		int chnls = 1;
		switch (btype)
		{
		case BarType::INT32_1:
		case BarType::FLOAT32_1:
			chnls = 1;
			break;
		case BarType::BYTE8_4:
			chnls = 4;
			break;
		case BarType::BYTE8_3:
			chnls = 3;
			break;
		case BarType::BYTE8_1:
			chnls = 1;
			break;
		default:
			assert(false);
			break;
		}
		reinit(width, height, chnls, btype);
	}

	BackImage(int width, int height, int chnls, buchar *_data, bool copy = true, bool deleteData = true)
	{
		if (copy)
		{
			copyFromRawData(width, height, chnls, _data);
		}
		else
			assignRawData(width, height, chnls, _data, deleteData);
	}


	BackImage(int width, int height, int chnls, BarType type, buchar *_data, bool copy = true, bool deleteData = true)
	{
		if (copy)
		{
			copyFromRawData(width, height, chnls, _data, type);
		}
		else
			assignRawData(width, height, chnls, _data, deleteData, type);
	}


	void reinit(int width = 1, int height = 1, int chnls = 1, BarType btype = BarType::NONE)
	{
		setMetadata(width, height, chnls, btype);
		valInit();
	}

	void reintAsInt()
	{
		assert(TSize == 4);
		type = BarType::INT32_1;
	}

	void assign(const BackImage &copy) { assignCopyOf(copy); }

	// move
	BackImage(BackImage &&moveImg) noexcept
	{
		assignInstanceOf(moveImg);
		moveImg._deleteData = false;
		moveImg.data = nullptr;
		moveImg.type = BarType::NONE;
	}

	BackImage &operator=(BackImage &&moveImg) noexcept
	{
		if (&moveImg != this)
		{
			assignInstanceOf(moveImg);
			moveImg.data = nullptr;
			moveImg._deleteData = false;
			moveImg.type = BarType::NONE;
		}

		return *this;
	}

// copy


	BackImage(const BackImage &copyImg)
	{
		assignCopyOf(copyImg);
	}


	BackImage(const BackImage &copyImg, bool copy)
	{
		if (copy)
			assignCopyOf(copyImg);
		else
			assignInstanceOf(copyImg);
	}

	//Перегрузка оператора присваивания
	BackImage &operator=(const BackImage &drob)
	{
		if (&drob != this)
			assignCopyOf(drob);

		return *this;
	}


	virtual ~BackImage() { valDelete(); }

	buchar *getData() const { return data; }

	void maxAndMin(Barscalar &_min, Barscalar &_max) const override
	{
		_max = getLiner(0);
		_min = getLiner(0);
		for (size_t i = 1; i < this->length(); i++)
		{
			Barscalar val = getLiner(i);
			if (val > _max)
				_max = val;
			if (val < _min)
				_min = val;
		}
		cachedMax.set(_max);
		cachedMin.set(_min);
	}

	Barscalar max() const
	{
		if (cachedMax.isCached)
			return cachedMax.val;

		Barscalar _max = getLiner(0);
		for (size_t i = 1; i < this->length(); i++)
		{
			Barscalar val = getLiner(i);
			if (val > _max)
				_max = val;
		}

		cachedMax.set(_max);
		return _max;
	}

	Barscalar min() const
	{
		if (cachedMin.isCached)
			return cachedMin.val;

		Barscalar _min = getLiner(0);
		for (size_t i = 1; i < this->length(); i++)
		{
			if (getLiner(i) < _min)
				_min = getLiner(i);
		}

		cachedMin.set(_min);
		return _min;
	}

	Barscalar MaxMinMin() const
	{
		if (cachedMin.isCached && cachedMax.isCached)
		{
			return cachedMax.val - cachedMin.val;
		}

		Barscalar _min = getLiner(0);
		Barscalar _max = getLiner(0);
		for (size_t i = 1; i < this->length(); i++)
		{
			const Barscalar v = getLiner(i);
			if (v < _min)
				_min = v;

			if (getLiner(i) > _max)
				_max = v;
		}
		cachedMin.set(_min);
		cachedMax.set(_max);

		return _max - _min;
	}

	void copyFromRawData(int width, int height, int chnls, buchar *rawData, BarType type = BarType::NONE)
	{
		setMetadata(width, height, chnls, type);
		valAssignCopyOf(rawData);
	}

	void assignRawData(int width, int height, int chnls, buchar *rawData, bool deleteData = true, BarType type = BarType::NONE)
	{
		setMetadata(width, height, chnls, type);

		valAssignInstanceOf(rawData, deleteData);
	}

	inline int channels() const override { return _channels; }

	inline size_t typeSize() const override { return TSize; }


	inline Barscalar get(int x, int y) const override
	{
		buchar* off = data + (y * _wid + x) * TSize;
		switch (type)
		{
		case BarType::INT32_1:
			return Barscalar(*reinterpret_cast<int*>(off), type);
		case BarType::FLOAT32_1:
			return Barscalar(*reinterpret_cast<float*>(off), type);
		case BarType::BYTE8_1:
			return Barscalar(off[0]);
		case BarType::BYTE8_3:
			return Barscalar(off[0], off[1], off[2]);
		case BarType::BYTE8_4:
			return Barscalar(off[0], off[1], off[2], off[3]);
		default:
			throw;
		}
	}

	inline buchar* lineOffet(int y, int x = 0) const
	{
		return data + (y * _wid + x) * TSize;
	}

	inline int bytesInLine() const
	{
		return _wid * TSize;
	}

	inline void set(int x, int y, const Barscalar& val)
	{
		buchar* off = data + (y * _wid + x) * TSize;
		if (type == BarType::BYTE8_1)
		{
			*off = val.getAvgUchar();
		}
		else
		{
			if (TSize == 4)
				off[3] = 255;

			switch (val.type)
			{
			case BarType::BYTE8_1:
				off[0] = val.data.b1;
				off[1] = val.data.b1;
				off[2] = val.data.b1;
				break;
			case BarType::BYTE8_4:
				if (TSize == 4)
					off[3] = val.data.b3[3];

			case BarType::BYTE8_3:
				off[0] = val.data.b3[0];
				off[1] = val.data.b3[1];
				off[2] = val.data.b3[2];
				break;
			case BarType::FLOAT32_1:
				off[0] = val.data.f;
				off[1] = val.data.f;
				off[2] = val.data.f;
				break;
			case BarType::INT32_1:
				off[0] = val.data.i;
				off[1] = val.data.i;
				off[2] = val.data.i;
				break;
				//			assert(false);
			default:
				assert(false);
				break;
			}

		}
		cachedMin.isCached = false;
		cachedMax.isCached = false;
	}

	struct m_rgbfill
	{
		buchar rgb[3];
	};
	struct m_rgbafill
	{
		buchar rgb[4];
	};

	inline void setRow(int x, int y, int xwid, const Barscalar& val)
	{
		//if (diagReverce)
		//	values[x * _wid + y] = val;
		//else
		assert(x < _wid);
		assert(y < _hei);
		assert(x + xwid <= _wid);
		assert(type == val.type);

		buchar* off = data + (y * _wid + x) * TSize;
		switch (type)
		{
		case BarType::BYTE8_1:
			memset(off, val.data.b1, xwid * TSize);
			break;
		case BarType::BYTE8_3:
			if (TSize == 4)
			{
				m_rgbafill valtof{ val.data.b3[0],  val.data.b3[1],  val.data.b3[2], 255 };
				std::fill_n((m_rgbafill*)off, xwid, valtof);
			}
			else
			{
				m_rgbfill valtof{ val.data.b3[0],  val.data.b3[1],  val.data.b3[2] };
				std::fill_n((m_rgbfill*)off, xwid, valtof);
			}
			break;
		case BarType::INT32_1:
			std::fill_n((int*)off, xwid, val.data.i);
			break;
		default:
			assert(false);
			break;
		}

		cachedMin.isCached = false;
		cachedMax.isCached = false;
	}

	inline void add(int x, int y, const Barscalar& val)
	{
		//if (diagReverce)
		//	values[x * _wid + y] += val;
		//else
		if (type == BarType::BYTE8_1)
		{
			data[(y * _wid + x) * TSize] += val.data.b1;
		}
		else
		{
			buchar *off = data + (y * _wid + x) * TSize;
			if (val.type == BarType::BYTE8_3)
			{
				off[0] += val.data.b3[0];
				off[1] += val.data.b3[1];
				off[2] += val.data.b3[2];
			}
			else
			{
				off[0] += val.data.b1;
				off[1] += val.data.b1;
				off[2] += val.data.b1;
			}
		}

		cachedMin.isCached = false;
		cachedMax.isCached = false;
	}


	inline void minus(int x, int y, const Barscalar& val)
	{
		//assert(values[y * _wid + x] >= val);
		//if (diagReverce)
		//	values[x * _wid + y] -= val;
		//else
		if (type == BarType::BYTE8_1)
		{
			assert(data[(y * _wid + x) * TSize] >= val.data.b1);
			data[(y * _wid + x) * TSize] -= val.data.b1;
		}
		else
		{
			buchar *off = data + (y * _wid + x) * TSize;
			if (val.type == BarType::BYTE8_3)
			{
				off[0] -= val.data.b3[0];
				off[1] -= val.data.b3[1];
				off[2] -= val.data.b3[2];
			}
			else
			{
				off[0] -= val.data.b1;
				off[1] -= val.data.b1;
				off[2] -= val.data.b1;
			}
		}

		cachedMin.isCached = false;
		cachedMax.isCached = false;
	}


	inline void setLiner(size_t pos, const Barscalar& val)
	{
		if (type == BarType::BYTE8_1)
		{
			data[pos * TSize] = val.data.b1;
		}
		else
		{
			buchar *off = data + pos * TSize;
			if (val.type == BarType::BYTE8_3)
			{
				off[0] = val.data.b3[0];
				off[1] = val.data.b3[1];
				off[2] = val.data.b3[2];
			}
			else
			{
				off[0] = val.data.b1;
				off[1] = val.data.b1;
				off[2] = val.data.b1;
			}
		}
		cachedMin.isCached = false;
		cachedMax.isCached = false;
	}

	virtual Barscalar getLiner(size_t pos) const override
	{
		if (type == BarType::BYTE8_1)
		{
			return Barscalar(data[pos * TSize]);
		}
		else
		{
			buchar *off = data + pos * TSize;
			return Barscalar(off[0], off[1], off[2]);
		}
	}

	void assignCopyOf(const BackImage& copy)
	{
		setMetadata(copy._wid, copy._hei, copy._channels);
		valAssignCopyOf(copy.data);

		this->diagReverce = copy.diagReverce;
		this->type = copy.type;
	}

	void assignInstanceOf(const BackImage& inst)
	{
		setMetadata(inst._wid, inst._hei, inst._channels);
		valAssignInstanceOf(inst.data);

		this->type = inst.type;
		this->diagReverce = inst.diagReverce;
	}

	void resize(int new_width, int new_height)
	{
		const float x_ratio = (float)width() / new_width;
		const float y_ratio = (float)height() / new_height;

		unsigned char* new_image = new unsigned char[new_width * new_height * channels()];

		for (int y = 0; y < new_height; y++)
		{
			for (int x = 0; x < new_width; x++)
			{
				int x_floor = (int)std::floor(x * x_ratio);
				int y_floor = (int)std::floor(y * y_ratio);
				int x_ceil = std::min(x_floor + 1, width() - 1);
				int y_ceil = std::min(y_floor + 1, height() - 1);
				float x_weight = x * x_ratio - x_floor;
				float y_weight = y * y_ratio - y_floor;

				for (int c = 0; c < _channels; c++)
				{
					const int new_index = (y * new_width + x) * _channels + c;
					int old_index = (y_floor * width() + x_floor) * _channels + c;
					const float top = data[old_index] * (1 - x_weight) + data[old_index + _channels] * x_weight;
					old_index = (y_ceil * width() + x_floor) * _channels + c;
					assert(old_index < length() * _channels);
					const float bottom = data[old_index] * (1 - x_weight) + data[old_index + _channels] * x_weight;
					assert(new_index < new_height * new_width * _channels);
					new_image[new_index] = (unsigned char)(top * (1 - y_weight) + bottom * y_weight);
				}
			}
		}

		valDelete();
		_deleteData = true;
		data = new_image;
		_wid = new_width;
		_hei = new_height;
	}


	BackImage getRect(int stX, int stRow, int wid, int hei)
	{
		const int typeSize = TSize; // channels * scalar size
		const int destBytesSize = wid * typeSize;

		BackImage r(wid, hei, _channels, type);
		for (int y = 0; y < hei; y++)
		{
			const int srcRowOffset = ((y + stRow) * _wid + stX) * typeSize;
			const int destRowOffset = y * wid * typeSize;
			memcpy(r.data + destRowOffset, data + srcRowOffset, destBytesSize);
		}

		return r;
	}
};


export void FrameworkInit()
{
	fpng::fpng_init();
}

export BackImage imread(const BackString& path)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load(path.c_str(), &width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	// if (chls == 4)
	// {
	// 	const int req = 3;
	// 	unsigned int length = width * height;
	// 	unsigned char* fixedData = new buchar[length * req];
	// 	//memcpy(fixedData, image_data, length * req);
	// 	for (size_t i = 0, destinationIndex = 0; i < length * chls; i += chls, destinationIndex += req)
	// 	{
	// 		fixedData[destinationIndex + 0] = image_data[i + 0];
	// 		fixedData[destinationIndex + 1] = image_data[i + 1];
	// 		fixedData[destinationIndex + 2] = image_data[i + 2];

	// 		/*for (size_t j = 0; j < i + req; ++j)
	// 		{
	// 			fixedData[j] = image_data[destinationIndexj];
	// 		}*/
	// 	}
	// 	delete[] image_data;
	// 	image_data = fixedData;
	// 	chls = req;
	// }
	return BackImage(width, height, chls, image_data, false, true);
}

export BackImage imread(const BackPathStr& path)
{
	return imread(path.string());
}

export void imwrite(const BackString& path, const BackImage& mat)
{
	fpng::fpng_encode_image_to_file(path.c_str(), mat.data, mat.width(), mat.height(), mat.channels());
	// stbi_write_png(path.c_str(), mat.width(), mat.height(), mat.channels(), mat.data, 0); // so slow...
}

export void imwrite(const BackPathStr& path, const BackImage& mat)
{
	imwrite(path.string(), mat);
}


export BackImage imreadFromMemory(const buchar* data, size_t size)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load_from_memory(data, size, &width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	if (chls == 4)
	{
		const int req = 3;
		unsigned int length = width * height;
		unsigned char* fixedData = new buchar[length * req];
		//memcpy(fixedData, image_data, length * req);
		for (size_t i = 0, destinationIndex = 0; i < length * chls; i += chls, destinationIndex += req)
		{
			fixedData[destinationIndex + 0] = image_data[i + 0];
			fixedData[destinationIndex + 1] = image_data[i + 1];
			fixedData[destinationIndex + 2] = image_data[i + 2];
		}
		delete[] image_data;
		image_data = fixedData;
		chls = req;
	}
	return BackImage(width, height, chls, image_data, false, true);
}

export using MemImgData = std::vector<uint8_t>;

export MemImgData imwriteToMemory(const BackImage& mat)
{
	MemImgData out_buf;
	bool r = fpng::fpng_encode_image_to_memory(mat.getData(), mat.width(), mat.height(), mat.channels(), out_buf);
	assert(r);
	return out_buf;
}
	BackImage imread(const BackString& path);
	BackImage imread(const BackPathStr& path);
	BackImage imreadFromMemory(const buchar* data, size_t size);

	void imwrite(const BackString& path, const BackImage& mat);
	void imwrite(const BackPathStr& path, const BackImage& mat);

	MemImgData imwriteToMemory(const BackImage& mat);
