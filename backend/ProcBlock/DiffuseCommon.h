#pragma once

#include "../Algs/Gen2/GenCommon.h"
#include "../Algs/Gen2/Trainer.h"
#include "Common.h"

import MatrModule;
//import BackBind;

constexpr int keylen = 8;
constexpr static char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 }, {0,0}};


struct ProcField : public Field<float>
{
public:
	ProcField(int w, int h) : Field(w, h)
	{
	}

	ProcField(BackSize size) : Field(size.wid, size.hei)
	{
	}

	ProcField(const ProcField& other) : Field(other.width, other.height)
	{
		std::copy(other.field.begin(), other.field.end(), field.begin());
	}

	void operator=(ProcField&& other)
	{
		field = std::move(other.field);
	}

	void operator=(const ProcField& other)
	{
		field = other.field;
	}

	void operator+=(const ProcField& rhs)
	{
		assert(length() == rhs.length());
		for (size_t i = 0; i < length(); i++)
		{
			field[i] += rhs.field[i];
		}
	}

	void fillRandom()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distrib(0, 1.0);

		for (auto& c : field)
		{
			c = distrib(gen);
			// std::fill_n(c.pos, 8, 0);
		}
	}

	void setRandom(size_t i)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<> distrib(0, 1.0);
		field[i] = distrib(gen);
	}
};

class MinMax
{
public:
	MinMax(BackImage& src) : MinMax()
	{
		// src.maxAndMin(imgmin, imgmax);
		// if (imgmin > imgmax)
		// {
		// 	std::swap(imgmin, imgmax);
		// }

		// diffcolor = imgmax - imgmin;

		// lower = imgmin.getAvgFloat();
		// upper = imgmax.getAvgFloat();
		// ludiff = upper - lower;
	}

	MinMax()
	{
		imgmin = Barscalar(0,0,0);
		imgmax = Barscalar(255,255,255);

		diffcolor = imgmax - imgmin;

		lower = imgmin.getAvgFloat();
		upper = imgmax.getAvgFloat();
		ludiff = upper - lower;
	}

	float getValue(const Barscalar& val) const
	{
		return (val.getAvgFloat() - lower) / ludiff;
	}

	Barscalar getColor(float colorProc) const
	{
		if (colorProc > 1.0)
			colorProc = 1.0;
		else if (colorProc < 0)
			colorProc = 0;

		Barscalar newColor = imgmin + diffcolor * colorProc;
		return newColor;
	}

	void restoreImage(BackImage& out, const ProcField& cells)
	{
		for (int i = 0; i < cells.length(); i++)
		{
			float colorProc = cells.getLiner(i);
			Barscalar newColor = getColor(colorProc);
			out.setLiner(i, newColor);
		}
	}

private:
	Barscalar diffcolor;
	Barscalar imgmin;
	Barscalar imgmax;
	float lower;
	float upper;
	float ludiff;
};

template<int T>
void diffuse(Trainer<T>& train, int steps, ProcField& cells)
{
	train.train();
	if (train.linesCollector.empty())
		return;

	std::random_device rd;
	std::mt19937 gen(rd());

	ProcField newCells(cells);
	std::cout << "Generating..." << std::endl;
	int offset = 0;
	for (int k = 0; k < steps; k++)
	{
		std::cout << "Step " << k + 1 << "/" << steps << "..." << std::endl;
		for (int i = offset; i < cells.length(); i++)
		{
			int x = i % cells.width;
			int y = i / cells.width;

			bool skip = false;
			HashK<keylen> h;
			for (uchar j = 0; j < keylen; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
				{
					h.key[j] = 0;
					skip = true;
					break;
				}

				float srcscalr = cells.get(xi, yi);
				h.key[j] = srcscalr;
			}

			if (skip)
				continue;

			int id = train.getCloser(h);
			auto* line = train.linesCollector[id].get();
			float val = line->value.get(gen);

			float& newCell = newCells.get(x, y);
			// newCell += abs(newCell - val) * adj;
			newCell = val;
			if (train.needDebugDraw())
				std::cout << "id> " << id << "(" << val << "->" << newCell << ")" << std::endl;
		}
		cells = newCells;
	}
	std::cout << "Done" << std::endl;
}
