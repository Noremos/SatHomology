module;

#define LOG
#include <vector>
#include <assert.h>
#include <functional>
#include <string.h>

#include <fstream> // for std::filebuf
#include <iterator> // for std::{i,o}streambuf_iterator
#include <filesystem>
#include <iostream>

#include "../Bind/Common.h"
#include "../Side/fast_float.h"


export module Obejct3DModule;
import ImgReader;
import IOCore;


typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;

//typedef unsigned long long offu64;
typedef unsigned int offu64;

typedef unsigned int objoff;


//2 ����� �� �������
constexpr const int resol = 2;
namespace
{
#ifdef _WIN32
#define nl "\r\n"
#elif defined macintosh // OS 9
#define nl "\r"
#else
#define nl "\n" // linux | Mac OS X uses \n
#endif
}



#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

typedef std::pair<objoff, objoff> NullIndex;



struct PrjgBarCallback
{
	std::function<void(int)> cbIncrValue;
	std::function<void(int)> cbSetMax;
	volatile bool& stopAction;
	PrjgBarCallback(volatile bool& stopAction) :stopAction(stopAction)
	{}
};


struct FileBuffer
{
private:
	BackString buffer;
	BackFileWriter outfile;
	int maxBufferSize = 10000;
public:
	bool openFileStream(const BackPathStr& path, int maxBufferSize = 10000, int openFlags = std::ios::out | std::ios::trunc)
	{
		outfile.open(path, openFlags);
		if (!outfile.is_open())
			return false;

		this->maxBufferSize = maxBufferSize;

		return true;
	}

	void write(const BackString& part)
	{
		if (!outfile.is_open())
			return;

		buffer.append(part);
		if (buffer.size() > maxBufferSize)
		{
			dowrite();
		}
	}

	void writeLine(const BackString& part = "")
	{
		if (!outfile.is_open())
			return;

		buffer.append(part);
		buffer.append(nl);
		if (buffer.size() > maxBufferSize)
		{
			dowrite();
		}
	}

	void dowrite()
	{
		outfile << buffer;
		buffer.clear();
	}
	void close()
	{
		if (outfile.is_open())
		{
			dowrite();
			outfile.close();
		}
	}

	~FileBuffer()
	{
		close();
	}
};



template<class T>
struct staticArray
{
private:
	T* m_buffer = nullptr;
	uint m_size = 0;


public:
	staticArray() {}
	staticArray(const staticArray& other) /*: s(other.s)*/
	{
		allocate(other.m_size);
		std::copy(other.m_buffer, other.m_buffer + other.m_size, m_buffer);
	}
	/*std::cout << "move failed!\n";*/
	staticArray(staticArray&& other) /*: s(std::move(o.s))*/
	{
		m_buffer = std::exchange(other.m_buffer, nullptr); // leave other in valid state
		m_size = std::exchange(other.m_size, 0);
	}

	T* data()
	{
		return m_buffer;
	}

	T* extract()
	{
		T* temp = std::exchange(m_buffer, nullptr);
		m_size = 0;

		return temp;
	}

	void allocate(uint nsize)
	{
		//		assert(size != 0);

		release();
		this->m_size = nsize;
		m_buffer = new T[nsize];
	}

	uint size() { return m_size; }

	void setData(T* newData, size_t size)
	{
		release();
		m_size = size;
		m_buffer = newData;
	}

	void setToZero()
	{
		memset(m_buffer, 0, m_size * sizeof(T));
	}

	T& operator[](std::size_t idx)
	{
		assert(idx < m_size);
		return m_buffer[idx];
	}

	// copy assignment
	staticArray& operator=(const staticArray& other)
	{
		// Guard self assignment
		if (this == &other)
			return *this;

		allocate(other.m_size);

		std::copy(other.m_buffer, other.m_buffer + other.m_size, m_buffer);
		return *this;
	}

	// move assignment
	staticArray& operator=(staticArray&& other) noexcept
	{
		// Guard self assignment
		if (this == &other)
			return *this; // delete[]/size=0 would also be ok

		m_buffer = std::exchange(other.m_buffer, nullptr); // leave other in valid state
		m_size = std::exchange(other.m_size, 0);
		return *this;
	}

	void release()
	{
		if (m_buffer)
		{
			delete[] m_buffer;
			m_buffer = nullptr;
		}
		m_size = 0;
	}

	~staticArray()
	{
		release();
	}
};

using vbuffer = staticArray<uchar>;


class Face3d
{
	std::vector<objoff> nums;
public:
	Face3d()
	{

	}
	Face3d(objoff* faces, int len) { add(faces, len); }
	void add(objoff* faces, int len)
	{
		for (int i = 0; i < len; ++i)
			nums.push_back(faces[i]);
	}
	void clear()
	{
		nums.clear();
	}
	void add(int f) { nums.push_back(f); }

	static void createVstr(objoff* faces, objoff len, BackString& out)
	{
		out = "f ";
		for (objoff i = 0; i < len - 1; ++i)
			out += intToStr(faces[i]) + " ";

		out += intToStr(faces[len - 1]) + nl;
	}
	static void createVstr(objoff* faces, objoff* texts, objoff len, BackString& out)
	{
		out = "f ";
		for (objoff i = 0; i < len - 1; ++i)
			out += intToStr(faces[i]) + "/" + intToStr(texts[i]) + " ";

		out += intToStr(faces[len - 1]) + "/" + intToStr(texts[len - 1]) + nl;
	}
	BackString buildStr(objoff offset = 0)
	{
		BackString st = "f ";
		for (auto v : nums)
			st += intToStr(v + offset) + " ";

		return st.substr(st.length() - 1);
	}

	~Face3d()
	{
		clear();
	}
};



struct lvertex
{
	lvertex(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z)
	{}
	float x, y, z;
};

export struct vertex
{
	vertex(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z), texX(_x), texY(_y)
	{}

	float x, y, z;
	float texX, texY;
};
struct face
{
	unsigned int v1, v2, v3;
	face(unsigned int _v1 = 0, int _v2 = 0, int _v3 = 0) : v1(_v1), v2(_v2), v3(_v3)
	{

	}
};

export class Object3d
{
public:
	Object3d();
	std::vector<vertex> vetexes;
	std::vector<lvertex> lvetexes;
	std::vector<face> faces;

	void readFile(const BackPathStr& path);
	void readFastfile(const PrjgBarCallback& pbCallback, const BackPathStr& path);
	static int getWord(const char* base);
};


#define USE_ROW


struct Point3f
{
	Point3f(float x, float y, float z) : X(x), Y(y), Z(z) {}
	float X, Y, Z;
};

enum class ProcessMode {
	performance = 0,
	speed
};

class Obj3d
{
	int width, height;
	ProcessMode mode = ProcessMode::performance;
	ImageReader* reader;
	BackString name;
	rowptr data[2];
#ifdef USE_ROW
	staticArray<objoff> currNullRow;
	staticArray<objoff> prevNullRow;
	//	objoff* nullRow[2];
	objoff getIndex(int x, int y, int /*realH*/)
	{
		return y == 0 ? prevNullRow[x / step] : currNullRow[x / step];
	}
#else
	std::vector<NullIndex> nulls;

	objoff getIndex(int x, int y, int realH)
	{
		if (data[y][x] == NAN_VALUE)
			return 0;

		objoff st = width * realH / (step * step) + x;

		objoff nullCount = 0;
		objoff poz = 1, total = nulls.size();
		for (; poz < total; ++poz)
			if (st < nulls[poz].first)
				break;

		nullCount = nulls[poz - 1].second;
		return st - nullCount;
	}
#endif

	int step = 1;
	float NAN_VALUE = -9999;
public:
	void setStep(int step) { this->step = step; }

	//	Obj3d(int wid, int hei, int step = 1) : width(wid), height(hei), step(step)
	//	{
	//		//		width = 500;
	//		//		height = 500;
	//		data = new float *[hei];
	//	}
	Obj3d(ImageReader* reader)
	{
		this->reader = reader;
		//		data[0] = nullptr;
		//		data[1] = nullptr;

		width = reader->width();
		height = reader->height();


		//		data = new float *[hei];
	}
	bool checkSuper(objoff w)
	{
		objoff wp = (w - 1) / step;
		objoff wc = w / step;
		float d1 = data[0].getFloat(wp);
		return d1 == data[0].getFloat(wc) && d1 == data[1].getFloat(wp) && d1 == data[1].getFloat(wc) && d1 != NAN_VALUE;
	}

	bool check(objoff face[3]) { return face[0] != 0 && face[1] != 0 && face[2] != 0; }
	void write(const PrjgBarCallback& pbCallback, const BackPathStr& path, int startRow, int lastRow)
	{
		std::remove(path.string().c_str());

		FileBuffer sw;
		if (!sw.openFileStream(path))
			return;

		sw.writeLine("o Main");

		float min = 9999;
		float max = -9999;
		int k = 0;

		NAN_VALUE = reader->getNullValue();

		//		data[0] = nullptr;
		data[0].release();
		data[1].release();
		const objoff sWidth = (width + step - 1) / step; //width / this->step;
#ifndef USE_ROW
		objoff nullCounter = 0;
		objoff lastCount = 0;
		nulls.clear();
		// starts with 1
		nulls.push_back(NullIndex(0, -1));
#else
		objoff counter = 0;

		prevNullRow.allocate(sWidth + 1);
		currNullRow.allocate(sWidth + 1);

		prevNullRow.setToZero();
		currNullRow.setToZero();
#endif

		float scale = 1.f / step;
		//		objoff offset = 0;
		int ends = (lastRow == 0 ? height : (lastRow + 1));

		ends = MIN(ends, height);
		startRow = MIN(0, height - 1);
		pbCallback.cbSetMax(ends / step);
		std::cout << startRow << " " << ends;

		for (int h = (startRow == 0 ? 0 : (startRow - 1)); h < ends; h += step)
		{
			//			if (h % step == 0)
			//			{
			pbCallback.cbIncrValue(1);
			if (pbCallback.stopAction)
				break;
			//			}


			if (h != 0)
			{
#ifndef USE_ROW
				nulls.remove(1, lastCount);
				lastCount = nulls.size() - 1;
#else
				auto temp = std::move(prevNullRow);
				prevNullRow = std::move(currNullRow);
				currNullRow = std::move(temp);
				// objoff *temp = prevNullRow.buffer;
				// prevNullRow.buffer = currNullRow.buffer;
				// currNullRow.buffer = temp;
				currNullRow.setToZero();
#endif
				data[0].release();
				data[0] = std::move(data[1]);
			}
			// not cached row, allocated memory, need to delete
			data[1] = reader->getRowData(h);

			BackString vers;
			for (int w = 0; w < width; w += step)
			{
				float value = data[1].getFloat(w);

				if (value == NAN_VALUE)
				{
#ifndef USE_ROW
					++nullCounter;
#endif
					continue;
				}

#ifndef USE_ROW
				if (nullCounter != 0)
				{
					int add = nulls[nulls.size() - 1].second;
					nulls.push_back(NullIndex(sWidth * h / step + w, nullCounter + add));
					nullCounter = 0;
				}
#endif

				if (value < min)
					min = value;
				if (value > max)
					max = value;

				sw.writeLine("v " + normConv(w, scale) + " " + normConv(value, scale) +
					" " + normConv(h, scale));
				sw.writeLine("vt " + intToStr((float)w / width) + " " + intToStr(1.f - (float)h / height));

#ifdef USE_ROW

				currNullRow[w / step] = ++counter;
#endif

				if (h > 0 && w > 0)
				{
					//0*
					//00
					objoff i_tr = getIndex(w, 0, h - 1);
					//00
					//0*
#ifndef USE_ROW
					objoff i_br = (sWidth * h + w - nulls[nulls.size() - 1].second);
#else
					objoff i_br = getIndex(w, step, h);
#endif
					//*0
					//00
					objoff i_tl = getIndex(w - step, 0, h - 1);

					//00
					//*0
					objoff i_bl = getIndex(w - step, step, h);

					//32
					//01
					objoff face[4]{ i_br, i_tr, i_tl, i_bl };
					//					if (checkSuper(w))
					//					{
					//						Face3d::createVstr(face, face, 4, vers);
					//						sw.write(vers);
					//						++k;
					//					}
					//					else
					{
						if (check(face))
						{
							Face3d::createVstr(face, face, 3, vers);
							sw.write(vers);
						}

						//10
						//23
						objoff face1[3]{ i_tl, i_bl, i_br };
						if (check(face1))
						{
							Face3d::createVstr(face1, face1, 3, vers);
							sw.write(vers);
						}
					}
				}
			}

			//			if (h % 50 == 0)
			//				std::cout << h;
		}

		//reader->max = max;
		//reader->min = min;
		//std::cout << "Min:" << min << " Max:" << max;
		std::cout << "Optimized:" << k;

		sw.dowrite();
		sw.close();

		data[0].release();
		data[1].release();
#ifdef USE_ROW
		prevNullRow.release();
		currNullRow.release();
#else
		nulls.clear();
#endif
	}


	BackString normConv(float f, double scale)
	{
		f *= scale;
		//		if (f < .000001 || f > 100000)

		auto str = intToStr(f);
		std::replace(str.begin(), str.end(), ',', '.');
		return str;
	}

	~Obj3d() { clear(); }
	void clear()
	{
	}
	ProcessMode getMode() const;
	void setMode(const ProcessMode& value);
};

ProcessMode Obj3d::getMode() const
{
	return mode;
}

void Obj3d::setMode(const ProcessMode& value)
{
	mode = value;
}

Object3d::Object3d()
{

}
int parceIntFast(const char* num, int len)
{
	int ret = 0;
	std::from_chars(num, num + len, ret);
	return ret;

	//int sum = 0;
	//for (int i = 0; i < len; i++)
	//{
	//	sum = (sum * 10) + (num[i] - '0');
	//}
	//return sum;
}

//static bool getLineFast(std::istream &is, std::string &t)
//{
//	t.clear();

//	std::istream::sentry se(is, true);
//	std::streambuf *sb = is.rdbuf();

//	while (true)
//	{
//		int c = sb->sbumpc();
//		if (c == EOF)
//			return false;
//		if (c == '\n' || c == '\r')
//		{
//			c = sb->sbumpc();
//			if (c == '\n')
//				sb->sbumpc();
//			else if (c == EOF)
//				return false;
//			else
//				sb->sungetc();
//			return true;
//		}
//		t += c;
//	}
//	return true;
//}


inline bool isNotEnd(int c)
{
	return (c != ' ' && c != '\n' && c != '\r');
}

int Object3d::getWord(const char* base)
{
	const char* ref = base;
	int len = 0;
	while (isNotEnd(*ref))
	{
		++ref;
		++len;
	}
	return len;
}

void parceFace(const char* str, unsigned int& f)
{
	const char* razd = str;
	//	int len = 0;
	while (*razd != '/')
	{
		++razd;
		//		++len;
	}
	std::from_chars(str, razd, f);
	--f;
	//	f = parceIntFast(str, len);
}

void Object3d::readFastfile(const PrjgBarCallback& pbCallback, const BackPathStr& path)
{

	BackFileReader fin;
	fin.open(path);
	if (!fin.is_open())
	{
		std::cout << "File " << path << " not found!";
		return;
	}

	size_t lines = 0;

	faces.clear();
	vetexes.clear();

	std::stringstream errss;
	std::string name;

	char rawtoken[500];
	const char* token;
	size_t readed = 0;
	float min = 9999;

	namespace fs = std::filesystem;
	const size_t ADD_STEP = 10;
	const size_t BYTES_COUNT = static_cast<size_t>(fs::file_size(path)) / ADD_STEP;
	pbCallback.cbSetMax(100);
	std::string line;
	while (!fin.eof())
	{
		if (readed >= BYTES_COUNT)
		{
			pbCallback.cbIncrValue(ADD_STEP);
			if (pbCallback.stopAction)
				break;
			readed = 0;
		}
		std::getline(fin, line);
		readed += line.length();
		token = rawtoken;
		lines++;

		// Skip if empty line.
		if (line.length() == 0)
			continue;

		int len;

		if (token[0] == 'v')
		{
			vertex v;

			token += 2;
			len = getWord(token);
			fast_float::from_chars(token, token + len, v.x);
			token += len + 1;

			len = getWord(token);
			fast_float::from_chars(token, token + len, v.y);
			token += len + 1;

			if (v.y < min)
				min = v.y;

			len = getWord(token);
			fast_float::from_chars(token, token + len, v.z);

			std::getline(fin, line);
			readed += line.length();
			token = rawtoken;

			lines++;

			// vt
			assert(token[0] == 'v' && token[1] == 't');

			token += 3;
			len = getWord(token);

			fast_float::from_chars(token, token + len, v.texX);
			int end = line.length() - 3;
			fast_float::from_chars(token + len + 1, token + end, v.texY);


			// token += len + 1;
			// int end = curline.length() - len - 4;
			// fast_float::from_chars(token, token + end, v.texY);

			this->vetexes.push_back(v);

			continue;
		}
		if (token[0] == 'f')
		{
			face fc;

			token += 2;
			len = getWord(token);
			parceFace(token, fc.v1);

			token += len + 1;
			len = getWord(token);
			parceFace(token, fc.v2);

			token += len + 1;
			//			len = getWord(token);
			parceFace(token, fc.v3);

			faces.push_back(fc);
			continue;
		}
	}
	pbCallback.cbIncrValue(ADD_STEP);

	fin.close();
	std::cout << "Readed " << lines << " lines";
	std::cout << "Vs: " << vetexes.size();
	std::cout << "Fs: " << faces.size();
	std::cout << "Min: " << min;
}


void Object3d::readFile(const BackPathStr& path)
{
	BackFileReader fin;
	fin.open(path);
	if (!fin.is_open())
	{
		std::cout << "File " << path << " not found!";
		return;
	}

	faces.clear();
	lvetexes.clear();

	size_t readed = 0;

	size_t lines = 0;
	std::string line;
	while (std::getline(fin, line))
	{
		readed += line.length();
		const char* token = line.c_str();
		lines++;

		// Skip if empty line.
		if (line.length() == 0)
			continue;

		int len;

		if (token[0] == 'v')
		{
			lvertex v;

			token += 2;
			len = getWord(token);
			fast_float::from_chars(token, token + len, v.x);
			token += len + 1;

			len = getWord(token);
			fast_float::from_chars(token, token + len, v.y);
			token += len + 1;

			len = getWord(token);
			fast_float::from_chars(token, token + len, v.z);

			this->lvetexes.push_back(v);
			continue;
		}
		if (token[0] == 'f')
		{
			face fc;

			token += 2;

			len = getWord(token);
			parceFace(token, fc.v1);

			token += len + 1;
			len = getWord(token);
			parceFace(token, fc.v2);

			token += len + 1;
			parceFace(token, fc.v3);

			faces.push_back(fc);
			continue;
		}
	}
	fin.close();
	//std::cout << "Readed " << lines << " lines";
	//std::cout << "LVs: " << lvetexes.size();
	//std::cout << "Fs: " << faces.size();

	return;
}
