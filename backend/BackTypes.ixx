module;
#include <algorithm>
#include <cassert>

export module BackTypes;


export template<class T>
struct StaticArray
{
private:
	T* m_buffer = nullptr;
	size_t m_size = 0;


public:
	StaticArray() {}
	StaticArray(const StaticArray& other) /*: s(other.s)*/
	{
		allocate(other.m_size);
		std::copy(other.m_buffer, other.m_buffer + other.m_size, m_buffer);
	}
	/*std::cout << "move failed!\n";*/
	StaticArray(StaticArray&& other) /*: s(std::move(o.s))*/
	{
		m_buffer = std::exchange(other.m_buffer, nullptr); // leave other in valid state
		m_size = std::exchange(other.m_size, 0);
	}

	T* data() const
	{
		return m_buffer;
	}

	size_t size() const { return m_size; }

	T* extract() const
	{
		T* temp = std::exchange(m_buffer, nullptr);
		m_size = 0;

		return temp;
	}

	void allocate(size_t nsize)
	{
		//		assert(size != 0);

		release();
		this->m_size = nsize;
		m_buffer = new T[nsize];
	}

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
	StaticArray& operator=(const StaticArray& other)
	{
		// Guard self assignment
		if (this == &other)
			return *this;

		allocate(other.m_size);

		std::copy(other.m_buffer, other.m_buffer + other.m_size, m_buffer);
		return *this;
	}

	// move assignment
	StaticArray& operator=(StaticArray&& other) noexcept
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

	~StaticArray()
	{
		release();
	}
};

export using vbuffer = StaticArray<unsigned char>;



//struct FileBuffer
//{
//private:
//	BackString buffer;
//	QFile outfile;
//	QTextStream* stream = nullptr;
//	int maxBufferSize = 10000;
//public:
//	bool openFileStream(BackString path, int maxBufferSize = 10000, QFile::OpenMode openFlags = QFile::WriteOnly | QFile::Truncate)
//	{
//		outfile.setFileName(path);
//
//		//		if (outfile.exists())
//		//		{
//		//			QFile::rename(path,  path.replace("bds.lst", "bds old.lst"));
//		//			//			outfile.setFileName(path);
//		//		}
//
//		if (!outfile.open(openFlags))
//			return false;
//
//		if (stream != nullptr)
//			delete stream;
//		stream = new QTextStream(&outfile);
//		this->maxBufferSize = maxBufferSize;
//
//		return true;
//	}
//
//	void write(const BackString& part)
//	{
//		if (stream == nullptr)
//			return;
//
//		buffer.append(part);
//		if (buffer.size() > maxBufferSize)
//		{
//			dowrite();
//		}
//	}
//
//	void writeLine(const BackString& part = "")
//	{
//		if (stream == nullptr)
//			return;
//
//		buffer.append(part);
//		buffer.append(nl);
//		if (buffer.size() > maxBufferSize)
//		{
//			dowrite();
//		}
//	}
//
//	void dowrite()
//	{
//		stream->operator<<(buffer);
//		buffer.clear();
//	}
//	void close()
//	{
//		if (stream != nullptr)
//		{
//			if (outfile.isOpen())
//			{
//				dowrite();
//				outfile.close();
//			}
//
//			delete stream;
//			stream = nullptr;
//		}
//	}
//	~FileBuffer()
//	{
//		close();
//	}
//};

