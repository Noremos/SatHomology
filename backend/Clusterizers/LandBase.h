#pragma once

#include <vector>
#include <cassert>
#define MEXPORT


template<class T>
constexpr T sqr(T a)
{
	return a * a;
}


template<class T>
constexpr double p2Norm(const std::vector<T>& a)
{
	double sum = 0;
	for (auto& el : a)
	{
		sum += static_cast<double>(sqr(el));
	}

	return sqrt(sum);
}


// Point-base
MEXPORT struct LandPoint
{
	//float x; // satrt
	//float y; // height

	float x; // start
	float y; // end

	float getLength() const
	{
		return y - x;
	}


	float operator-(const LandPoint& other) const
	{
		float a = x - other.x;
		float b = y - other.y;
		float r = a * a + b * b;
		if (r == 0)
			return 0;

		return sqrt(a * a + b * b);
	}


	bool operator=(const LandPoint& other) const
	{
		return x == other.x && y == other.y;
	}
};

struct LyambdaLine
{
	std::vector<LandPoint> points;
	int k = 0;

	size_t size() const
	{
		return points.size();
	}

	float getSup() const
	{
		float r = 0;
		auto max = std::max_element(points.begin(), points.end(), [](const LandPoint& a, const LandPoint& b)
		{
			return a.y < b.y;
		});
		return max->y;
	}
};
using Landscape = std::vector<LyambdaLine>;


// Iter-base
struct LyambdaIterLine
{
	std::vector<float> points;
	int k;

	size_t size() const
	{
		return points.size();
	}

	double get2pNorm() const
	{
		double sum = 0;
		for (size_t i = 0; i < points.size(); i++)
		{
			sum += sqr(points[i]);
		}
		return sqrt(sum);
	}
	double getInfNorm() const
	{
		double max = 0;
		for (size_t i = 0; i < points.size(); i++)
		{
			if (max < points[i])
				max = points[i];
		}
		return max;
	}
};

using IterLandscape = std::vector<LyambdaIterLine>;

inline double iterLandDistanceEvklid(const LyambdaIterLine& a, const LyambdaIterLine& b,float = 0)
{
	assert(a.size() == b.size());


	int dist = 0;
	for (size_t i = 0; i < a.size(); i++)
	{
		dist += sqr(a.points[i] - b.points[i]);
	}

	dist = sqrt(dist);
	return dist;
}


inline double iterLandDistance2p(const LyambdaIterLine& a, const LyambdaIterLine& b,float = 0)
{
	assert(a.size() == b.size());

	double diff = 0;
	auto ap = a.get2pNorm();
	auto bp = b.get2pNorm();
	return abs(ap - bp);
}

inline double iterLandDistance2p(const IterLandscape& a, const IterLandscape& b,float = 0)
{
	assert(a[0].size() == b[0].size());

	double diff = 0;
	const size_t minsize = std::min(a.size(), b.size());
	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaIterLine& lineA = a[i];
		const LyambdaIterLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		diff += iterLandDistance2p(lineA, lineB);
	}

	const IterLandscape& maxiter =  a.size() > b.size() ? a : b;


	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaIterLine& line = maxiter[i];

		diff += line.get2pNorm();
	}

	return diff / maxiter.size();
}


inline double iterLandDistanceEvklid(const IterLandscape& a, const IterLandscape& b,float = 0)
{
	assert(a[0].size() == b[0].size());

	double diff = 0;
	const size_t minsize = std::min(a.size(), b.size());
	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaIterLine& lineA = a[i];
		const LyambdaIterLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		diff += iterLandDistanceEvklid(lineA, lineB);
	}

	const IterLandscape& maxiter =  a.size() > b.size() ? a : b;


	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaIterLine& line = maxiter[i];

		diff += line.get2pNorm();
	}

	return sqrt(diff);
}



inline double iterLandDistanceSupPlusP2(const IterLandscape& a, const IterLandscape& b,float = 0)
{
	assert(a[0].size() == b[0].size());
	const size_t N = a.size();

	const size_t minsize = std::min(a.size(), b.size());
	LyambdaIterLine sup;
	sup.points.resize(N, 0);

	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaIterLine& lineA = a[i];
		const LyambdaIterLine& lineB = b[i];
		for (size_t k = 0; k < N; k++)
		{
			sup.points[k] = std::max(sup.points[k], abs(lineA.points[k] - lineB.points[k]));
		}
	}

	const IterLandscape& maxiter =  a.size() > b.size() ? a : b;

	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaIterLine& line = maxiter[i];
		for (size_t k = 0; k < N; k++)
			sup.points[k] = std::max(sup.points[k], line.points[k]);
	}

	return p2Norm(sup.points);
}


struct PointsIterator
{
	const std::vector<LandPoint>& points;
	float t = 1.f;
	size_t curPoint = 0;
	float endX = 0;
	float x = 1;
	float y = 0;
	float yIter = 0;

	void reset()
	{
		curPoint = 0;
		endX = 0;
		x = 1;
		y = 0;
		yIter = 0;
	}

	bool ended() const
	{
		return (curPoint >= points.size());
	}
	// Current state is after execute
	float executeLymbda()
	{
		if (ended())
			return 0;

		if (x >= endX)
		{
			++curPoint;
			if (ended())
				return 0;

			const auto& prev = points[curPoint - 1];
			const auto& cur = points[curPoint];

			y = prev.y;

			float startX = x = prev.x;
			endX = cur.x;

			assert(startX <= endX);

			const float width = endX - startX;
			const float height = cur.y - prev.y;
			if (height == 0 || width == 0)
			{
				yIter = 0;
				return y;
			}
			yIter = height / width;

			return y;
		}

		x += t;
		y += yIter * t;
		assert(std::round(y) >= 0);
		return y;
	}
};



struct CorrectPointsIterator
{
	const std::vector<LandPoint>& points;
	size_t curPoint = 0;
	float startX = 0;
	float endX = 0;
	float x = 0;
	const float xIter = 0;
	float startY;
	bool asc;


	CorrectPointsIterator(const std::vector<LandPoint>& points, float t) :
		points(points), xIter(t)
	{
		updateLine();
	}

	bool ended() const
	{
		return (x > endX && curPoint + 1 >= points.size());
	}

	void updateLine(bool skipZero = false)
	{
		++curPoint;
		const LandPoint& prev = points[curPoint - 1];
		const LandPoint& cur = points[curPoint];

		if (skipZero && (prev.y == 0 && cur.y == 0))
		{
			updateLine(true);
			return;
		}
		asc = (prev.y < cur.y);

		startY = prev.y;

		startX = prev.x;
		endX = cur.x;
	}
	// Current state is after execute
	float executeLymbda()
	{
		if (ended())
			return 0;

		float orgX = x;
		x += xIter;

		while (orgX >= endX)
		{
			if (curPoint + 1 >= points.size())
				return 0;

			updateLine(true);
		}

		if (orgX < startX)
			return 0;

		float diff = orgX - startX;
		float outY = startY + ( asc ? diff : -diff );
		assert(std::round(outY) >= 0);
		return outY;
	}
};

// sup по всему
inline double iterLandDistanceInf(const Landscape& a, const Landscape& b, float)
{
	assert(a[0].size() == b[0].size());

	double maxDiff = 0;
	const size_t minsize = std::min(a.size(), b.size());
	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		maxDiff = std::max<double>(maxDiff, abs(lineA.getSup() - lineB.getSup()));
	}

	const Landscape& maxiter =  a.size() > b.size() ? a : b;

	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaLine& line = maxiter[i];

		maxDiff = std::max<double>(maxDiff, line.getSup());
	}

	return maxDiff;
}


inline double iterLandDistance2(const Landscape& a, const Landscape& b, float)
{
	assert(a[0].size() == b[0].size());

	double maxDiff = 0;
	const size_t minsize = std::min(a.size(), b.size());
	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		maxDiff += sqr(abs(lineA.getSup() - lineB.getSup()));
	}

	const Landscape& maxiter =  a.size() > b.size() ? a : b;

	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaLine& line = maxiter[i];

		maxDiff += sqr(line.getSup());
	}

	return sqrt(maxDiff);
}


inline double iterLandDistanceMdpiInf(const Landscape& a, const Landscape& b, float t)
{
	double maxDiff = 0;
	const size_t minsize = std::min(a.size(), b.size());
	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		PointsIterator ait{lineA.points, t};
		PointsIterator bit{lineB.points, t};

		while (!ait.ended() && !bit.ended())
		{
			maxDiff = std::max<double>(maxDiff, abs(ait.executeLymbda() - bit.executeLymbda()));
		}
	}

	const Landscape& maxiter = a.size() > b.size() ? a : b;

	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaLine& line = maxiter[i];
		PointsIterator it{line.points, t};

		while (!it.ended())
		{
			maxDiff = std::max<double>(maxDiff, it.executeLymbda());
		}
	}

	return maxDiff;
}


inline double iterLandDistanceMdpi2(const Landscape& a, const Landscape& b, float t = 0.1f)
{
	assert(a[0].size() == b[0].size());

	double maxDiff = 0;
	const size_t minsize = std::min(a.size(), b.size());

	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		PointsIterator ait{lineA.points, t};
		PointsIterator bit{lineB.points, t};

		float loc = 0;
		while (!ait.ended() && !bit.ended())
		{
			loc += abs(ait.executeLymbda() - bit.executeLymbda());
		}
		maxDiff += sqr(loc);
	}

	const Landscape& maxiter = a.size() > b.size() ? a : b;
	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaLine& line = maxiter[i];
		PointsIterator it{line.points, t};

		float loc = 0;
		while (!it.ended())
		{
			loc += abs(it.executeLymbda());
		}
		maxDiff += sqr(loc);
	}

	return sqrt(maxDiff);
}

// Дистанция по сумму норм
inline double iterLandDistanceSum2(const Landscape& a, const Landscape& b, float t = 0.1f)
{
	double maxDiff = 0;
	const size_t minsize = std::min(a.size(), b.size());

	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		PointsIterator ait{lineA.points, t};
		PointsIterator bit{lineB.points, t};

		float locD = 0;
		while (!ait.ended() && !bit.ended())
		{
			locD += sqr(abs(ait.executeLymbda() - bit.executeLymbda()));
		}
		maxDiff += sqrt(locD);
	}

	const Landscape& maxiter = a.size() > b.size() ? a : b;
	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaLine& line = maxiter[i];
		PointsIterator it{line.points, t};

		float locD = 0;
		while (!it.ended())
		{
			locD += sqr(it.executeLymbda());
		}

		maxDiff += sqrt(locD);
	}

	return maxDiff;
}



// Дистанция по сумму норм
inline double iterLandDistanceSumInf(const Landscape& a, const Landscape& b, float t = 0.1f)
{
	double maxDiff = 0;
	const size_t minsize = std::min(a.size(), b.size());

	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		PointsIterator ait{lineA.points, t};
		PointsIterator bit{lineB.points, t};

		float locD = 0;
		while (!ait.ended() && !bit.ended())
		{
			locD = std::max<double>(locD, abs(ait.executeLymbda() - bit.executeLymbda()));
		}
		maxDiff += locD;
	}

	const Landscape& maxiter = a.size() > b.size() ? a : b;

	for (size_t i = minsize; i < maxiter.size(); i++)
	{
		const LyambdaLine& line = maxiter[i];
		PointsIterator it{line.points, t};

		float locD = 0;
		while (!it.ended())
		{
			locD = std::max<double>(locD, it.executeLymbda());
		}
		maxDiff += locD;
	}

	return maxDiff;
}



template<typename TBlock>
concept BlockDistanceConc = requires(TBlock&& block, const Landscape& a, const Landscape& b, float t, int maxK)
{
    { block.startCmp() } -> std::same_as<void>;
    { block.endCmp() } -> std::same_as<void>;
    { block.cmp(a[0], b[0]) } -> std::same_as<void>;
    { block.cmp(a[0], 0) } -> std::same_as<void>;
    { block.result() } -> std::same_as<double>;
};


template<typename TBlock>
// requires BlockDistanceConc<TBlock>
inline double iterDistance(const Landscape& a, const Landscape& b, float t = 0.1f, int maxK = -1)
{
	const size_t minsize = maxK == -1 ? std::min(a.size(), b.size()) : (size_t)maxK;

	TBlock block;
	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		CorrectPointsIterator ait{lineA.points, t};
		CorrectPointsIterator bit{lineB.points, t};

		block.startCmp();
		while (!ait.ended() && !bit.ended())
		{
			block.cmp(ait.executeLymbda(), bit.executeLymbda());
			assert(abs(ait.x - bit.x) < 1.f);
		}
		block.endCmp();
	}

	const Landscape& maxiter = a.size() > b.size() ? a : b;
	const size_t secondSize = maxK == -1 ? maxiter.size() : (size_t)maxK;
	for (size_t i = minsize; i < secondSize; i++)
	{
		const LyambdaLine& line = maxiter[i];
		CorrectPointsIterator it{line.points, t};

		block.startCmp();
		while (!it.ended())
		{
			block.cmp(it.executeLymbda(), 0);
		}
		block.endCmp();
	}

	return block.result();
}

template<typename TBlock>
inline double lineDistance(const Landscape& a, const Landscape& b, int maxK = -1)
{
	TBlock block;
	const size_t minsize = maxK == -1 ? std::min(a.size(), b.size()) : (size_t)maxK;
	for (size_t i = 0; i < minsize; i++)
	{
		const LyambdaLine& lineA = a[i];
		const LyambdaLine& lineB = b[i];
		assert(lineA.k == lineB.k);

		block.cmp(lineA, lineB);
	}

	const Landscape& maxiter =  a.size() > b.size() ? a : b;
	const size_t secondSize = maxK == -1 ? maxiter.size() : (size_t)maxK;
	for (size_t i = minsize; i < secondSize; i++)
	{
		const LyambdaLine& line = maxiter[i];

		block.cmp(line);
	}

	return block.result();
}


namespace Distance
{
	struct DummyStartEnd
	{
		void startCmp() {}
		void endCmp() {}
	};

	namespace Iter
	{

	// Сумма норм 2 для каждого К
	struct SumNorm2
	{
		double sum = 0;

		double locD;
		void startCmp()
		{
			locD = 0;
		}

		void cmp(float a, float b)
		{
			locD += sqr(a - b);
		}

		void endCmp()
		{
			sum += sqrt(locD);
		}

		double result()
		{
			return sum;
		}
	};



	// Норма 2 по K и T
	struct Norm2 : public DummyStartEnd
	{
		double sum = 0;
		void cmp(float a, float b)
		{
			sum += sqr(a - b);
		}

		double result()
		{
			return sqrt(sum);
		}
	};

	// Норма inf по K и T
	struct Inf : public DummyStartEnd
	{
		double maxDiff = 0;
		void cmp(float a, float b)
		{
			maxDiff = std::max<double>(maxDiff, abs(a - b));
		}

		double result()
		{
			return maxDiff;
		}
	};

	// Считаем  норму inf для каждого К и суммируем
	struct InfSum : public DummyStartEnd
	{
		double sum = 0;
		float locD;
		void startCmp()
		{
			locD = 0;
		}

		void cmp(float a, float b)
		{
			locD = std::max<float>(locD, abs(a - b));
		}

		void endCmp()
		{
			sum += locD;
		}

		double result()
		{
			return sum;
		}
	};

	}


	namespace Line
	{
		inline double polygonArea(const std::vector<LandPoint>& vertices)
		{
			double area = 0.0;
			int n = vertices.size();

			for (int i = 0; i < n; ++i) {
				int j = (i + 1) % n;
				area += vertices[i].x * vertices[j].y;
				area -= vertices[j].x * vertices[i].y;
			}

			area = std::abs(area) / 2.0;
			return area;
		}

		struct AreaInf
		{
			double maxDiff = 0.f;


			void cmp(const LyambdaLine& a)
			{
				maxDiff = std::max(maxDiff, abs(polygonArea(a.points)));
			}

			void cmp(const LyambdaLine& a, const LyambdaLine& b)
			{
				maxDiff = std::max(maxDiff, abs(polygonArea(a.points) - polygonArea(b.points)));
			}

			double result()
			{
				return maxDiff;
			}
		};

		struct AreaNorm2
		{
			double diff = 0.f;

			void cmp(const LyambdaLine& a)
			{
				diff += sqr(polygonArea(a.points));
			}

			void cmp(const LyambdaLine& a, const LyambdaLine& b)
			{
				diff += sqr(polygonArea(a.points) - polygonArea(b.points));
			}

			double result()
			{
				return std::sqrt(diff);
			}
		};
	} // Line

} // Distance
