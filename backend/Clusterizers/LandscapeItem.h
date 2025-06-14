#ifdef USE_MODULE
module;
#else
#pragma once
#endif

//#include "../../side/kde.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "Barcode/PrjBarlib/include/barstrucs.h"
#include "Common.h"

#ifdef USE_MODULE
export module ConvertItem;
#undef MEXPORT
#define MEXPORT export
import ClusterInterface;
import BackBind;
import CachedBarcode;
import Platform;
import ExteranlReader;
import Sklearn;
#else
#undef MEXPORT
#define MEXPORT
#include "../CachedBarcode.h"
#include "../Interfaces/ICluster.h"
#include "../Bind/Framework.h"
#include "ExteranlReader.h"
#include "SKLearnInterface.h"
#include "LandBase.h"
#endif

MEXPORT class LandscapeClass : public ICluster
{
public:
	bc::barvector matrix;
	Landscape landscape;

	int depth = 0;
	// mutable float maxEnd = 0;
//	bool addMiddleDepth;

	// virtual void add(float x, float y, bool round)
//
	const bc::barvector& getMatrix() const override
	{
		return matrix;
	}

	void add(float x, float y, bool round)
	{
		x =  round ? roundf(x) : x;
		if (landscape.back().size() != 0)
		{
			auto& curVec = landscape.back();
			auto& b = curVec.points.back();
			assert(b.x <= x);
			// assert(b.x != x || b.y != y); //TODO: FIXME
			if (b.x == x && b.y == y)
				return;
		}

		assert(std::isfinite(y));
		landscape.back().points.push_back({x, y});
	}
//
	size_t id;
	LandscapeClass(size_t id = -1) : id(id)
	{ }
//
	size_t getId() const override
	{
		return id;
	}

	virtual int getDeath() const override
	{
		return depth;
	}

	//float getLength() const
	//{
	//	return pack.back().x - pack[0].x;
	//}

	explicit LandscapeClass(const LandscapeClass& other) //: id(id)
	{
		id = other.id;
		depth = other.depth;
		matrix = other.matrix;
		landscape = other.landscape;
	}

	explicit LandscapeClass(LandscapeClass&& other) //: id(id)
	{
		id = other.id;
		depth = other.depth;
		matrix = std::move(other.matrix);
		landscape = std::move(other.landscape);
	}

	Barscalar start() const override
	{
		return landscape[0].points[0].x;
	}

	Barscalar end() const override
	{
		return landscape[0].points[0].y;
	}


	void getSignature(BackString& line) const override
	{
	}

private:
};

struct InputLandLine
{
	float start;
	float len;
	bc::barvector matrix;

	float getStart() const
	{
		return start;
	}
	float getMiddle() const
	{
		return start + len / 2;
	}
	float getHeight() const
	{
		return len / 2;
	}
	float getEnd() const
	{
		return start + len;
	}
};

struct InputLandData
{
	std::vector<InputLandLine> lands;
	void sort()
	{
		std::sort(lands.begin(), lands.end(), [](const InputLandLine& a, const InputLandLine& b) {
			if (a.start == b.start)
				return a.len > b.len; // First the longest

			return a.start < b.start;
		});
	}

	InputLandLine& get(size_t id)
	{
		assert(id < lands.size());
		return lands[id];
	}

	InputLandLine* getByValue(size_t firstId, float endLimit)
	{
		if (firstId >= lands.size())
			return nullptr;

		auto& cur = lands[firstId];
		if (cur.getStart() <= endLimit && cur.getEnd() > endLimit)
			return &lands[firstId];

		return nullptr;
	}

	void addExpr(Barscalar st, Barscalar end, float resolution = 1.f)
	{
		if (st > end)
			std::swap(st, end);

		assert(st != end);

		float length = (end - st).getAvgFloat();
		assert(length > 0.f);
		lands.push_back({st.getAvgFloat() * resolution, length  * resolution});
	}

	void addExprRaw(float st, float end, float resolution = 1.f)
	{
		if (st > end)
			std::swap(st, end);

		assert(st != end);

		float length = (end - st);
		assert(length > 0.f);
		lands.push_back({st, length});
	}


	size_t size() const
	{
		return lands.size();
	}
};

template<class T>
constexpr bool between(const T& a, const T& minNotEnc, const T& maxNotEncl)
{
	return minNotEnc < a && a < maxNotEncl;
}
//
MEXPORT class LandscapeCollection : public IClusterItemValuesHolder<LandscapeClass>
{
	using Base = IClusterItemValuesHolder<LandscapeClass>;
protected:
//
public:
	float maxEnd = 0;
	bool round = true;
	LandscapeCollection() : Base(false)
	{
		Base::settings =
		{
			{"Compare Only Mode", true},
			{"Resolution", 10}
		};
	}

	void performOnPerform()
	{
		Base::settings.setBool("Compare Only Mode", false);
	}

	bool isCmpMode() const
	{
		bool cmpMode = *Base::settings.getBool("Compare Only Mode");
		return cmpMode;
	}

	void convertToLandscape(InputLandData& landscape, LandscapeClass* source = nullptr);

	struct DIK
	{
		struct Point
		{
			size_t lineId;
			float hei;
			float lineHei;
		};

		std::vector<Point> tops;

		void sort()
		{
			std::sort(tops.begin(), tops.end(), [](const Point& a, const Point& b) {
				if (a.hei == b.hei)
					return a.lineHei < b.lineHei;

				return a.hei < b.hei;
			});
		}

		size_t size() const
		{
			return tops.size();
		}

		Point pop(size_t curLineId)
		{
			assert(size() > 0);
			auto id = tops.back();
			tops.pop_back();

			// for (size_t i = 0; i < tops.size(); i++)
			// {
			// 	if (tops[i].lineId == curLineId)
			// 	{
			// 		tops[i].hei = id.hei;
			// 		break;
			// 	}
			// }
			// while (tops.size() > 0 && std::round(tops.back().hei * 100) == std::round(id.hei * 100))
			// {
			// 	tops.pop_back();
			// }

			// if (id.hei == 0)
			// 	tops.clear();

			return id;
		}
	};

	void convertToLandscape_it(InputLandData& landscape, LandscapeClass* source = nullptr)
	{
		landscape.sort();
		int resolution =  *Base::settings.getInt("Resolution");

		std::vector<DIK> diks(256 * resolution + 1);
		// std::vector<DIK> tops(256 * resolution);

		int start = 256 * resolution;
		int end = 0;

		// birth
		for (size_t lineId = 0; lineId < landscape.size(); lineId++)
		{
			auto* line = &landscape.get(lineId);

			float lstart = line->getStart() * resolution;
			float lend = line->getEnd() * resolution;
			float length = lend - lstart;
			float half = length / 2;
			for (int j = 0, totol = std::round(length); j <= totol; j++)
			{
				int x = std::floor(lstart) + j;
				float y = j < half ? j : totol - j;
				assert(y >= 0);
				diks[x].tops.push_back({ lineId, y, line->getHeight() });
			}

			start = std::min<int>(start, std::floor(lstart));
			end = std::max<int>(end, std::round(lend));
		}
		for (size_t i = 0; i < diks.size(); i++)
		{
			diks[i].sort();
		}

		bool hasLine = true;
		int k = 0;
		while (hasLine)
		{
			if (source == nullptr)
			{
				Base::items.push_back({});
				// Base::items.back().maxEnd = maxEnd;
			}
			LandscapeClass& cl = source ? *source : Base::items.back();

			hasLine = false;
			for (int iterator = start; iterator <= end; iterator++)
			{
				if (diks[iterator].size() == 0)
					continue;

				if (!hasLine)
				{
					cl.id = 0;
					cl.landscape.push_back({});
					cl.landscape.back().k = k++;
					hasLine = true;
				}

				int lineId = diks[iterator].pop(-1).lineId;

				auto* l = &landscape.get(lineId);

				int lstart = iterator++;
				// int mid = l->getMiddle();
				// assert(start == std::round(l->getStart()));

				cl.add(l->start, 0, round);
				float prevHei = 0.f;
				bool acs = true;
				bool addMiddle = true;
				// cl.add(mid, l->getHeight(), round);

				while (diks[iterator].size() != 0)
				{
					auto p = diks[iterator].pop(lineId);
					int nextLineId = p.lineId;
					if (nextLineId != lineId)
					{
						auto* nl = &landscape.get(nextLineId);
						float crossLength;
						float corssX;
						if (iterator > l->getMiddle())
						{
							crossLength = l->getEnd() - nl->getStart();
							corssX =  l->getEnd() - crossLength / 2.f;
							if (addMiddle)
								cl.add(l->getMiddle(), l->getHeight(), round); // Middle top
						}
						else
						{
							crossLength = nl->getEnd() - l->getStart();
							corssX =  nl->getEnd() - crossLength / 2.f;
						}

						addMiddle = (nl->getMiddle() > corssX);

						cl.add(corssX, crossLength / 2.f, round);
						// cl.add(nl->getMiddle(), nl->getHeight(), round);
						l = nl;


						lineId = nextLineId;
					}

					// float hei = diks[iterator].popLineHei();
					// if (acs)
					// {
					// 	if (prevHei >= hei)
					// 	{
					// 		acs = false;
					// 		cl.add(iterator - 1, prevHei, round);
					// 	}
					// }
					// else
					// {
					// 	if (prevHei <= hei)
					// 	{
					// 		acs = true;
					// 		cl.add(iterator, prevHei, round);
					// 	}
					// }
					// prevHei = p.hei;

					++iterator;
				}
				// cl.add(iterator - 1, 0, round);

				if (addMiddle)
					cl.add(l->getMiddle(), l->getHeight(), round);
				cl.add(l->getEnd(), 0.f, round);
			} // for
		} // while

		if (source == nullptr)
		{
			if (Base::items.back().landscape.size() == 0)
				Base::items.pop_back();
		}
	}

	void collectChildren(const CachedBarline& line)
	{
		int resolution =  *Base::settings.getInt("Resolution");
		convertLand.addExpr(line.start(), line.end(), resolution);
		convertLand.lands.back().matrix = line.getMatrix();
		// maxEnd = std::max(maxEnd, line.end().getAvgFloat());

		for (int i = 0; i < line.getChildrenCount(); i++)
		{
			collectChildren(*line.getChild(i));
		}
	}

	void setTestEnv()
	{
		*settings.getInt("Resolution") = 1;
		round = false;
	}

	InputLandData convertLand;
	virtual void addItem(const CachedBarline& line)
	{
		bool cmpMode = *Base::settings.getBool("Compare Only Mode");

		if (cmpMode)
		{
			collectChildren(line);
			Base::items.push_back({});
			auto& source = Base::items.back();
			convertToLandscape(convertLand, &source);

			convertLand.lands.clear();
			source.matrix = line.getMatrix();
		}
		else
		{
			int resolution =  *Base::settings.getInt("Resolution");
			convertLand.addExpr(line.start(), line.end(), resolution);
			convertLand.lands.back().matrix = line.getMatrix();
		}
	}

	void addAllLines(const CachedBaritemHolder& holder)
	{
		const std::vector<CachedBarline>& items = holder.getItems();

		size_t size = holder.getItemsCount();
				// Skip created root
		if (holder.getRoot()->length() == 0)
		{
			size--;
		}

		int resolution =  *Base::settings.getInt("Resolution");
		for (size_t i = 0; i < size; i++)
		{
			const CachedBarline& line = items[i];
			// if (line.length() < 10)
			// 	continue;

			convertLand.addExpr(line.start(), line.end(), resolution);
			assert(line.length().getAvgFloat() > 0);
			// maxEnd = std::max(maxEnd, convertLand.lands.back().getEnd());
		}

		// maxEnd = 255;

		Base::items.push_back({});
		auto& source = Base::items.back();
		convertToLandscape(convertLand, &source);

		convertLand.lands.clear();
		// source.matrix = line.getMatrix();
	}

	virtual void perform()
	{
		bool cmpMode = *Base::settings.getBool("Compare Only Mode");
		if (!cmpMode)
			convertToLandscape(convertLand);
	}

	/*const CachedBarline* getRItem(size_t id) const
	{
		return &holder.getItems()[id];
	}

	virtual size_t getItemsCount() const
	{
		return holder.getItemsCount();
	}

	virtual void clear()
	{
		holder.getItems().clear();
	}*/
};


MEXPORT class LandClassifier : public ISklearnClassifier
{
	//// ������� ��� ���������� ��������� K-means
	//void kmeans(const std::vector<Point>& data, int k)
	//{
	//	// ������������� ����������
	//	std::vector<Point> centroids;
	//	for (int i = 0; i < k; ++i) {
	//		centroids.push_back(data[i]);
	//	}

	//	// ������������ ����� � ��������� ���������� � �������� ����������
	//	bool centroidsChanged = true;
	//	while (centroidsChanged)
	//	{
	//		centroidsChanged = false;

	//		// ������ ��������� � �������
	//		std::vector<std::vector<Point>> clusters(k);

	//		// ������������ ����� � ��������� ����������
	//		for (const auto& point : data) {
	//			double minDistance = std::numeric_limits<double>::max();
	//			int closestCentroid = 0;

	//			for (int i = 0; i < k; ++i) {
	//				double distance = euclideanDistance(point, centroids[i]);

	//				if (distance < minDistance) {
	//					minDistance = distance;
	//					closestCentroid = i;
	//				}
	//			}

	//			clusters[closestCentroid].push_back(point);
	//		}

	//		// �������� ����������
	//		for (int i = 0; i < k; ++i)
	//		{
	//			double sumX = 0.0;
	//			double sumY = 0.0;

	//			for (const auto& point : clusters[i]) {
	//				sumX += point.x;
	//				sumY += point.y;
	//			}

	//			if (!clusters[i].empty())
	//			{
	//				double newCentroidX = sumX / clusters[i].size();
	//				double newCentroidY = sumY / clusters[i].size();

	//				if (newCentroidX != centroids[i].x || newCentroidY != centroids[i].y)
	//				{
	//					centroidsChanged = true;
	//					centroids[i].x = newCentroidX;
	//					centroids[i].y = newCentroidY;
	//				}
	//			}
	//		}
	//	}

	//	// ����� �����������
	//	for (int i = 0; i < k; ++i) {
	//		std::cout << "������� " << i + 1 << " �������� �����:";

	//		for (const auto& point : clusters[i]) {
	//			std::cout << " (" << point.x << ", " << point.y << ")";
	//		}

	//		std::cout << std::endl;
	//	}
	//}



	virtual void writeToTemp(const IClusterItemHolder& iallItems, BackFileWriter &tempFile)
	{
		const LandscapeCollection& allItems = dynamic_cast<const LandscapeCollection&>(iallItems);
		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& item = allItems.getRItem(i);
			BackString line;
			item.getSignature(line);
			tempFile << line;

			tempFile.seekp(-1, tempFile.cur); // ������� ��������� �������
			tempFile << std::endl;
		}


		//// ������� ��� ���������� ����������� ���������� ����� ����� �������
		//double euclideanDistance(const Point& p1, const Point& p2) {
		//	return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
		//}

		// ������ ������
		//kmeans(data, k);

		//return 0;

		// execCmd += (Variables::metaPath / "landscape.py").string() + " ";

		// execCmd += filePath;
		// execCmd += " 'a' ";

		// execCmd += " '";
		// execCmd += getPythonSettings(settings);
		// execCmd += "'";
		// return exec(execCmd, cachedAssignments, n);
	}
};
