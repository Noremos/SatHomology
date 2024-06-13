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
#define MEXPORT export
import ClusterInterface;
import BackBind;
import CachedBarcode;
import Platform;
import ExteranlReader;
import Sklearn;
#else
#define MEXPORT
#include "../CachedBarcode.h"
#include "../Interfaces/ICluster.h"
#include "../Bind/Framework.h"
#include "ExteranlReader.h"
#include "SKLearnInterface.h"
#endif


MEXPORT struct landres
{
	//float x; // satrt
	//float y; // height

	float x; // start
	float y; // end

	float getLength() const
	{
		return y - x;
	}


	float operator-(const landres& other) const
	{
		float a = x - other.x;
		float b = y - other.y;
		float r = a * a + b * b;
		if (r == 0)
			return 0;

		return sqrt(a * a + b * b);
	}
};

MEXPORT class ConvertClass : public ICluster
{
public:
	bc::barvector matrix;
	std::vector<std::vector<landres>> pathset;
	float AddE = 0;
	mutable float maxEnd = 0;
//	bool addMiddleDepth;
//
	const bc::barvector& getMatrix() const override
	{
		return matrix;
	}

	void add(float x, float y)
	{
		// assert(pathset.back().size() == 0 || pathset.back().back().x != x);
		pathset.back().push_back({x, y});
	}
//
	size_t id;
	ConvertClass(size_t id = -1) : id(id)
	{ }
//
	size_t getId() const override
	{
		return id;
	}

	//float getLength() const
	//{
	//	return pack.back().x - pack[0].x;
	//}

	explicit ConvertClass(const ConvertClass& other) //: id(id)
	{
		id = other.id;
		matrix = other.matrix;
		pathset = other.pathset;
		AddE = other.AddE;
		maxEnd = other.maxEnd;
	}

	explicit ConvertClass(ConvertClass&& other) //: id(id)
	{
		id = other.id;
		matrix = std::move(other.matrix);
		pathset =   std::move(other.pathset);
		AddE = other.AddE;
		maxEnd = other.maxEnd;
	}

	Barscalar start() const override
	{
		return pathset[0][0].x;
	}

	Barscalar end() const override
	{
		return pathset[0][0].y;
	}


	static void itrateOverPoints(const std::vector<landres>& points, std::vector<float>& total, float AddE)
	{
		for (size_t j = 1; j < points.size(); j++)
		{
			const auto& prev = points[j - 1];
			const auto& cur = points[j];
			float startY = prev.y;

			const size_t startX = round(prev.x * AddE); // Из-за округления может быть равным с предыдущем
			const size_t endX = round(cur.x * AddE);

			assert(startX <= endX);

			const float width = endX - startX;
			const float height = abs(cur.y - prev.y);
			if (height == 0 || width == 0)
			{
				total[startX] += round(startY);
				continue;
			}
			float iter = height / (width);

			float y = startY;
			assert(endX < total.size());
			for (size_t i = startX; i < endX; i++)
			{
				total[i] += round(y);
				y += iter;
			}
		}
	}

	void getSignatureAsVector(std::vector<float>& total) const
	{
		total.resize(256.f * AddE); // 1 because of rounding and 1 because of including range [0, maxEnd]
		std::fill(total.begin(), total.end(), 0);

		float N = pathset.size();

		// For each lyambda
		for (const auto& landset : pathset)
		{
			itrateOverPoints(landset, total, AddE);
		}

	}

	void getCombinedPoints(std::vector<landres>& total) const
	{
		float N = pathset.size();

		// For each lyambda
		std::unordered_map<float, float> combinedPoints;
		for (const auto& path : pathset)
		{
			for (auto &point : path)
			{

				float y = point.y;
				auto it = combinedPoints.find(point.x);
				if (it == combinedPoints.end())
					combinedPoints.insert({y, 1.f});
				else
					it->second += y;
			}
		}

		total.clear();
		for (auto &l : combinedPoints)
		{
			total.push_back( {l.first, l.second});
		}

		std::sort(total.begin(), total.end(), [](const auto& a, const auto& b) {
			return a.x < b.x;
		});
	}

	void getCombinedPointsAsHist(std::vector<float>& total) const
	{
		std::vector<landres> points;
		getCombinedPoints(points);

		total.clear();
		total.resize(256.f * AddE);
		for (auto &l : points)
		{
			total[round(l.x * AddE)] += l.y;
		}
	}

	void getCombinedPointsAsSignature(std::vector<float>& total) const
	{
		std::vector<landres> points;
		getCombinedPoints(points);

		total.clear();
		total.resize(256.f * AddE);
		itrateOverPoints(points, total, AddE);
	}

	void getSignature(BackString& line) const override
	{
		std::vector<float> total;
		getSignatureAsVector(total);

		line.clear();
		for (size_t i = 0; i < total.size(); i++)
		{
			// total[i] /= N;
			line += std::to_string(total[i]);
			line += " ";
		}
	}


private:
};

struct landline
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

struct Landscape
{
	std::vector<landline> lands;
	void sort()
	{
		std::sort(lands.begin(), lands.end(), [](const landline& a, const landline& b) {
			if (a.start == b.start)
				return a.len < b.len;

			return a.start < b.start;
		});
	}

	landline& get(size_t id)
	{
		return lands[id];
	}

	landline* getByValue(size_t firstId, float endLimit)
	{
		if (firstId >= lands.size())
			return nullptr;

		auto& cur = lands[firstId];
		if (cur.getStart() <= endLimit && cur.getEnd() > endLimit)
			return &lands[firstId];

		return nullptr;
	}

	void addExpr(float st, float end)
	{
		if (st > end)
			std::swap(st, end);

		lands.push_back({st, end  - st});
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
MEXPORT class ConvertCollection : public IClusterItemValuesHolder<ConvertClass>
{
	using Base = IClusterItemValuesHolder<ConvertClass>;
protected:
//
public:
	float maxEnd = 0;
	ConvertCollection() : Base(false)
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

	void convert(Landscape& landscape, ConvertClass* source = nullptr)
	{
		if (source)
			source->maxEnd = maxEnd;

		landscape.sort();
		// �� birth
		for (size_t i = 0; i < landscape.size(); i++)
		{
			auto* line = &landscape.get(i);

			float start = line->getStart();
			float end = line->getEnd();
			// std::cout << start << " " << end << std::endl;
			// float matrstart = start;
			// float beginning = start;
			float crossWithPrev = start;
			// float ending = end;
			// float lastMin = beginning;
			// float lastStart = start;

			//std::vector<landres> landPath = { {start, 0}, {middle, h} };

			if (source == nullptr)
			{
				Base::items.push_back({});
				Base::items.back().maxEnd = maxEnd;
			}

			ConvertClass& cl = source ? *source : Base::items.back();
			cl.id = i;
			cl.pathset.push_back({});
			cl.add(start, 0);
			cl.AddE = *Base::settings.getInt("Resolution");


			//cl.matrix = line.matrix;

			size_t startI = 0;
			int curI = i;
			int prevId = -1;
			bool curLineCatechd = false;
			while (true)
			{
				// Asc, началось до текущего, окончилос внутри
				bool reachedPeak = true;
				for (int ascI = curI - 1; ascI >= 0; --ascI)
				{
					// c - crossWithPrev
					//  CUR->                C-------------E
					//  PRV->   S-------------E

					//  CUR->                C-------M------E
					//  PRV->   S-----------M-------------E
					// if (ascI == prevId)
					// 	continue;

					auto& prevLine = landscape.get(ascI);
					if (prevLine.getStart() < start && between(prevLine.getEnd(), crossWithPrev, end)) // Между концом предыдущего и концном текущего
					{
						float cross = prevLine.getEnd() - start;
						cl.add(start + cross / 2, cross / 2);
						end = prevLine.getEnd();
						reachedPeak = false;
						break;
					}
				}

				float len = end - start;
				float middle = start + len / 2;

				// Select common values (all when there is no prev)
				// from line.satrt to prevLine.end
				// from end to line->end
				auto& matr = line->matrix; //srcLine.getChild(startI)->getMatrix();
				for (auto& val : matr)
				{
					if (val.value < end)
						break;

					cl.matrix.push_back(val);
				}
				curLineCatechd = true;


				if (reachedPeak)
				{
					auto len = end - start;
					cl.add(middle, len / 2);
				}

				// dsc, те, которые начались внутри и окончились снаружи текущего
				prevId = curI;
				size_t nextI = static_cast<size_t>(curI) + 1;

				for (; nextI < landscape.size(); nextI++)
				{
					//  CUR->      |-------M------|
					//  NEXT->                   |-------M-------|

					//  CUR->      |-------M------|
					//  NEXT->      |-------M-------|
					auto& land = landscape.get(nextI);
					if (land.getStart() > end)
					{
						break;
					}
					if (land.getStart() == start)
					{
						// Когда начинается с одинакового значения
						continue;
					}

					if (land.getStart() < end && end < land.getEnd())
					{
						//float crossGip = end - convertLand->getStart();
						//float leh = crossGip / 2;

						// from line matrstart(cur line start) to convertLand.start
						// From this line start to the next line start

						float cross = end - land.getStart(); // длина пересечения
						crossWithPrev = end; // Смотрим предыдущий, который до окончания текущего
						cl.add(end - cross / 2, cross / 2);

						// lastMin = end;
						start = land.getStart();
						end = land.getEnd();
						curLineCatechd = false;
						curI = nextI;
						line = &landscape.get(nextI);
						// matrstart = line->getStart();

						break;
					}

					//landPath.push_back({ convertLand->getStart() +  leh, leh});
					//auto lastH = landPath.back();
					//cl.paths.push_back({ lastH.x + lastH.y, 0 });
					//cl.paths.push_back({ lastH.x + lastH.y, 0 });
				}
				if (prevId == curI)
				{
					assert(end <= maxEnd);
					cl.add(end, 0);
					break;
				}
			}

			if (!curLineCatechd)
			{
				auto& chm = line->matrix; //srcLine.getChild(startI)->getMatrix();
				for (auto& val : chm)
				{
					cl.matrix.push_back(val);
				}
			}
		}
	}


	void collectChildren(const CachedBarline& line)
	{
		convertLand.addExpr(line.start().getAvgFloat(), line.end().getAvgFloat());
		convertLand.lands.back().matrix = line.getMatrix();
		maxEnd = std::max(maxEnd, line.end().getAvgFloat());

		for (int i = 0; i < line.getChildrenCount(); i++)
		{
			collectChildren(*line.getChild(i));
		}
	}

	Landscape convertLand;
	virtual void addItem(const CachedBarline& line)
	{
		bool cmpMode = *Base::settings.getBool("Compare Only Mode");

		if (cmpMode)
		{
			collectChildren(line);
			Base::items.push_back({});
			auto& source = Base::items.back();
			convert(convertLand, &source);

			convertLand.lands.clear();
			source.matrix = line.getMatrix();
		}
		else
		{
			convertLand.addExpr(line.start().getAvgFloat(), line.end().getAvgFloat());
			convertLand.lands.back().matrix = line.getMatrix();
			maxEnd = std::max(maxEnd, line.end().getAvgFloat());
		}
	}

	void addAllLines(const CachedBaritemHolder& holder)
	{
		bool cmpMode = *Base::settings.getBool("Compare Only Mode");
		const std::vector<CachedBarline>& items = holder.getItems();

		size_t size = holder.getItemsCount();
				// Skip created root
		if (holder.getRoot()->length() == 0)
		{
			size--;
		}

		for (size_t i = 0; i < size; i++)
		{
			const CachedBarline& line = items[i];
			convertLand.addExpr(line.start().getAvgFloat(), line.end().getAvgFloat());
			assert(line.length().getAvgFloat() > 0);
			maxEnd = std::max(maxEnd, convertLand.lands.back().getEnd());
		}

		// maxEnd = 255;

		Base::items.push_back({});
		auto& source = Base::items.back();
		convert(convertLand, &source);

		convertLand.lands.clear();
		// source.matrix = line.getMatrix();
	}

	virtual void perform()
	{
		bool cmpMode = *Base::settings.getBool("Compare Only Mode");
		if (!cmpMode)
			convert(convertLand);
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
		const ConvertCollection& allItems = dynamic_cast<const ConvertCollection&>(iallItems);
		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& item = allItems.getRItem(i);
			BackString line;
			item.maxEnd = allItems.maxEnd;
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

GlobalClusterRegister<ConvertClass, ConvertCollection, LandClassifier> c("Landscape");
// TODO: Land projection of line
