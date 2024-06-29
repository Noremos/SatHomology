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
};

struct LyambdaLine
{
	std::vector<LandPoint> points;
	int k;

	size_t size() const
	{
		return points.size();
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

inline double iterLandDistanceEvklid(const LyambdaIterLine& a, const LyambdaIterLine& b)
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


inline double iterLandDistance2p(const LyambdaIterLine& a, const LyambdaIterLine& b)
{
	assert(a.size() == b.size());

	double diff = 0;
	auto ap = a.get2pNorm();
	auto bp = b.get2pNorm();
	return abs(ap - bp);
}

inline double iterLandDistance2p(const IterLandscape& a, const IterLandscape& b)
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


inline double iterLandDistanceEvklid(const IterLandscape& a, const IterLandscape& b)
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



inline double iterLandDistanceSupPlusP2(const IterLandscape& a, const IterLandscape& b)
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
