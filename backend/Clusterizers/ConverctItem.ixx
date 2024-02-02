module;
//#include "../../side/kde.h"

#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>

#include "../../Bind/Common.h"
//#include "../../side/KmeansPlusPlus/kmeansPlusPlus.h"

export module ConvertItem;

import ClusterInterface;
import TreeSignClass;
import ExteranlReader;
import Platform;



struct landres
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
	std::vector<landres> path;
//	bool addMiddleDepth;
//
	const bc::barvector& getMatrix() const
	{
		return matrix;
	}
	void add(float x, float y)
	{
		path.push_back({x, y});
	}
//
	ConvertClass()
	{ }
//

	//float getLength() const
	//{
	//	return pack.back().x - pack[0].x;
	//}

	explicit ConvertClass(const ConvertClass& other) //: id(id)
	{
		matrix = other.matrix;
		path = other.path;
	}

	explicit ConvertClass(ConvertClass&& other) //: id(id)
	{
		matrix = std::move(other.matrix);
		path =   std::move(other.path);
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

			return a.start > b.start;
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
//
export class ConvertCollection : public IClusterItemValuesHolder<ConvertClass>
{
	using Base = IClusterItemValuesHolder<ConvertClass>;
protected:
//

public:
	ConvertCollection() : Base(false)
	{
	}



	void convert(Landscape& landscape)
	{
		landscape.sort();
		// По birth
		for (size_t i = 0; i < landscape.size(); i++)
		{
			auto& line = landscape.get(i);
			
			float start = line.getStart();
			float middle = line.getMiddle();
			float h = line.getHeight();
			float end = line.getEnd();
			float minLength = 0;

			//std::vector<landres> landPath = { {start, 0}, {middle, h} };
			
			Base::items.push_back({});
			ConvertClass& cl = Base::items.back();
			//cl.matrix = line.matrix;

			int startI = 0;

			size_t curI = i;
			while (true)
			{
				// Asc
				for (size_t ascI = 0; ascI < curI; ascI++)
				{
					auto& land = landscape.get(ascI);
					if (land.getStart() < start && start < land.getEnd() && land.getEnd() < end && land.len > minLength)
					{
						end = land.getEnd();
						auto& chm = land.matrix; // srcLine.getChild(i)->getMatrix();
						for (auto& val : chm)
						{
							if (val.value >= end)
								break;
							
							cl.matrix.push_back(val);
						}
						break;
					}
				}
				// dsc
				bool found = false;
				startI = curI;
				for (; startI < landscape.size(); startI++)
				{
					auto& land = landscape.get(startI);

					if (land.getStart() < end && land.getEnd() > end)
					{
						//float crossGip = end - land->getStart();
						//float leh = crossGip / 2;

						minLength = end - land.getStart();
						start = land.getStart();
						end = land.getEnd();
						found = true;
						curI = startI;

						auto& chm = land.matrix; //srcLine.getChild(startI)->getMatrix();
						for (auto& val : chm)
						{
							if (val.value >= end)
								break;

							cl.matrix.push_back(val);
						}

						break;
					}

					//landPath.push_back({ land->getStart() +  leh, leh});
					//auto lastH = landPath.back();
					//cl.paths.push_back({ lastH.x + lastH.y, 0 });
					//cl.paths.push_back({ lastH.x + lastH.y, 0 });
				}
				if (!found)
					break;
			}
			cl.add(start, end);
		}
	}


	Landscape land;
	virtual void addItem(const CachedBarline& line)
	{
		//lines.addUpdateRoot(line);
		//lines.getLastItem().root = &lines;
		//for (int i = 0; i < line.getChildrenCount(); i++)
		//{
		//	CachedBarline* child = line.getChild(i);
		//	land.addExpr(child->start().getAvgFloat(), child->end().getAvgFloat());
		//}

		land.addExpr(line.start().getAvgFloat(), line.end().getAvgFloat());
	}


	virtual void perform()
	{
		convert(land);
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


export class LandClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	LandClassifier()
	{
		IBarClusterizer::settings =
		{
			{"n_clusters", 3},
		};
	}

	const BackString name() const
	{
		return "Land";
	}

	void setClassesCount(int size)
	{
		n = size;
	}

	int getClusters()
	{
		return n;
	}

	//// Функция для выполнения алгоритма K-means
	//void kmeans(const std::vector<Point>& data, int k)
	//{
	//	// Инициализация центроидов
	//	std::vector<Point> centroids;
	//	for (int i = 0; i < k; ++i) {
	//		centroids.push_back(data[i]);
	//	}

	//	// Присваивание точек к ближайшим центроидам и пересчет центроидов
	//	bool centroidsChanged = true;
	//	while (centroidsChanged)
	//	{
	//		centroidsChanged = false;

	//		// Список кластеров с точками
	//		std::vector<std::vector<Point>> clusters(k);

	//		// Присваивание точек к ближайшим центроидам
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

	//		// Пересчет центроидов
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

	//	// Вывод результатов
	//	for (int i = 0; i < k; ++i) {
	//		std::cout << "Кластер " << i + 1 << " содержит точки:";

	//		for (const auto& point : clusters[i]) {
	//			std::cout << " (" << point.x << ", " << point.y << ")";
	//		}

	//		std::cout << std::endl;
	//	}
	//}



	bool predict(const IClusterItemHolder& iallItems)
	{
		const ConvertCollection& allItems = dynamic_cast<const ConvertCollection&>(iallItems);

		BackString filePath = get_temp_file_path();
		std::ofstream tempFile(filePath, std::ofstream::out | std::ofstream::trunc);
		if (!tempFile.is_open())
		{
			std::cerr << "Unable to open temporary file for writing." << std::endl;
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < allItems.getItemsCount(); i++)
		{
			auto& item = allItems.getItem(i);

			for (const auto& num : item.path)
			{
				tempFile << num.getLength() << " ";
			}
			tempFile.seekp(-1, tempFile.cur); // Удаляем последнюю запятую
			tempFile << std::endl;

		}
		tempFile.close();


		//// Функция для вычисления Евклидового расстояния между двумя точками
		//double euclideanDistance(const Point & p1, const Point & p2) {
		//	return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
		//}

		// Пример данных
		//kmeans(data, k);

		//return 0;

		BackString execCmd = "python.exe ";
		execCmd += (Variables::metaPath / "landscape.py").string() + " ";
		execCmd += filePath;
		execCmd += " 'a' ";

		execCmd += " '";
		execCmd += getPythonSettings(settings);
		execCmd += "'";
		return exec(execCmd, cachedAssignments, n);
	}

	int test(size_t itemId)
	{
		if (itemId >= cachedAssignments.size())
			return -1;

		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<ConvertClass, ConvertCollection, LandClassifier> c("Land");
