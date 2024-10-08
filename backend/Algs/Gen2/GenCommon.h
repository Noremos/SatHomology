#include <vector>

template <class T>
struct Field
{
public:
	Field(int w, int h) : width(w), height(h)
	{
		field.resize(width * height);
	}

	std::vector<T> field;
	int width;
	int height;

	T& get(int x, int y)
	{
		return field[y * width + x];
	}
	void set(int x, int y, const T& c)
	{
		field[y * width + x] = c;
	}

	void setLiner(int i, const T& c)
	{
		field[i] = c;
	}

	T& getLiner(int pos)
	{
		return field[pos];
	}

	const T& getLiner(int pos) const
	{
		return field[pos];
	}

	size_t length() const
	{
		return field.size();
	}
};
