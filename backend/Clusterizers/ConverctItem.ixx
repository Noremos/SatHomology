module;
//#include "../../side/kde.h"

#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "Barcode/PrjBarlib/include/barstrucs.h"

export module ConvertItem;

import ClusterInterface;
import Platform;
import BackBind;
import ExteranlReader;
import CachedBarcode;
import Sklearn;



export struct landres
{
	//float x; // satrt
	//float y; // height

	float x; // start
	float y; // end

	float getLength() const
	{
		return y - x;
	}
};

export class ConvertClass : public ICluster
{
public:
	bc::barvector matrix;
	std::vector<std::vector<landres>> pathset;
//	bool addMiddleDepth;
//
	const bc::barvector& getMatrix() const override
	{
		return matrix;
	}

	void add(float x, float y)
	{
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
	}

	explicit ConvertClass(ConvertClass&& other) //: id(id)
	{
		id = other.id;
		matrix = std::move(other.matrix);
		pathset =   std::move(other.pathset);
	}

	Barscalar start() const override
	{
		return pathset[0][0].x;
	}

	Barscalar end() const override
	{
		return pathset[0][0].y;
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
export class ConvertCollection : public IClusterItemValuesHolder<ConvertClass>
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

	bool isCmpMode() const
	{
		bool cmpMode = *Base::settings.getBool("Compare Only Mode");
		return cmpMode;
	}

	void convert(Landscape& landscape, ConvertClass* source = nullptr)
	{
		landscape.sort();
		// �� birth
		for (size_t i = 0; i < landscape.size(); i++)
		{
			auto* line = &landscape.get(i);

			float start = line->getStart();
			float end = line->getEnd();
			// float matrstart = start;
			float beginning = start;
			float crossWithPrev = start;
			// float ending = end;
			// float lastMin = beginning;
			// float lastStart = start;

			//std::vector<landres> landPath = { {start, 0}, {middle, h} };

			if (source == nullptr)
				Base::items.push_back({});

			ConvertClass& cl = source ? *source : Base::items.back();
			cl.id = i;
			cl.pathset.push_back({});
			cl.add(start, 0);

			//cl.matrix = line.matrix;

			size_t startI = 0;
			int curI = i;
			int prevId = -1;
			bool curLineCatechd = false;
			while (true)
			{
				// Asc
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
					if (prevLine.getStart() < start && between(prevLine.getEnd(), crossWithPrev, end))
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

				// dsc
				prevId = curI;
				size_t nextI = static_cast<size_t>(curI) + 1;

				for (; nextI < landscape.size(); nextI++)
				{
					//  CUR->      |-------M------|
					//  NEXT->                   |-------M-------|

					//  CUR->      |-------M------|
					//  NEXT->      |-------M-------|
					auto& convertLand = landscape.get(nextI);
					if (convertLand.getStart() > end)
					{
						break;
					}

					if (convertLand.getStart() < end && convertLand.getEnd() > end)
					{
						//float crossGip = end - convertLand->getStart();
						//float leh = crossGip / 2;

						// from line matrstart(cur line start) to convertLand.start
						// From this line start to the next line start

						float cross = end - convertLand.getStart();
						crossWithPrev = end;
						cl.add(end - cross / 2, cross / 2);

						// lastMin = end;
						start = convertLand.getStart();
						end = convertLand.getEnd();
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
					maxEnd = std::max(maxEnd, end);
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

		for (int i = 0; i < line.getChildrenCount(); i++)
		{
			collectChildren(*line.getChild(i));
		}
	}


	void fillDate(size_t id, BackFileWriter& out) const
	{
		const int AddE =  *Base::settings.getInt("Resolution");;

		std::vector<float> total;
		total.resize(maxEnd * AddE);
		std::fill(total.begin(), total.end(), 0);

		auto& item = getRItem(id);
		float N = item.pathset.size();

		for (const auto& landset : item.pathset)
		{
			int startX = landset[0].x * AddE;
			float startY = landset[0].y;
			for (size_t j = 1; j < landset.size(); j++)
			{
				const auto& land = landset[j];

				const size_t endX = land.x * AddE;
				float iter = (land.y - startY) / (endX - startX);
				for (size_t i = startX; i < endX; i++)
				{
					total[i] += startY;
					startY += iter;
				}
				startX = endX;
			}
		}

		for (size_t i = 0; i < total.size(); i++)
		{
			// total[i] /= N;
			out << total[i] / N << " ";
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
		}
	}

	virtual void perform()
	{
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


export class LandClassifier : public ISklearnClassifier
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
			allItems.fillDate(i, tempFile);

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

GlobalClusterRegister<ConvertClass, ConvertCollection, LandClassifier> c("Land All To New");
// TODO: Land projection of line
