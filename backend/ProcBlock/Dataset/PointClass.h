#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "../../Clusterizers/ConverctItem.h"


class PointCluster
{

	// const int N = 7;
	// int results[7];

	// std::unordered_map<int,int> used;
public:
	int test(int id)
	{
		return output[id];
	}

	struct PredClass
	{
		int id;
		int count;
	};

	struct Distance
	{
		int to;
		float distance;
	};

	using LineId = int;
	using VertId = int; // Point
	using ClassId = int; // Point

	std::vector<ClassId> output;

	union Move
	{
		struct Cpm
		{
			VertId point;
			LineId line;
		} cmp;
		size_t id;
	};

	static size_t getIt(VertId point, LineId line)
	{
		Move::Cpm cp = {point, line};
		return Move{cp}.id;
	}

	struct Graph
	{
		std::vector<LineId> vertToLine; // id is a vertex id, map vert -> line
		std::vector<float> distances;
		std::vector<ClassId> output;

		// std::multimap<VertId, Distance> connections;
		std::unordered_set<size_t> used;


		void setLines(int count)
		{
			output.resize(count);
			distances.resize(count * count);
		}

		VertId vertCounter = 0;
		VertId addVert(LineId lineId)
		{
			vertToLine.push_back(lineId);
			return vertCounter++;
		}

		// void add(VertId a, VertId b, float distance)
		// {
		// 	assert(a != b);
		// 	if (a > b)
		// 		std::swap(a, b);

		// 	connections.insert(std::make_pair(a, {b, distance}));
		// }

		// bool has(VertId a, VertId b) const
		// {
		// 	assert(a != b);
		// 	if (a > b)
		// 		std::swap(a, b);
		// 	return connections.count(a) != 0;
		// }

		// bool active(VertId a, VertId b) const
		// {
		// 	assert(a != b);
		// 	if (a > b)
		// 		std::swap(a, b);

		// 	return used.count(std::make_pair(a, b)) == 0;
		// }

		bool hasConnection(VertId a, LineId dest) const
		{
			return used.count(getIt(a, dest)) != 0;
		}

		void connect(VertId a, VertId b, float distance)
		{
			LineId la = vertToLine[a];
			LineId lb = vertToLine[b];
			if (la == lb)
				return;

			if (hasConnection(a, lb))
				return;

			if (hasConnection(b, la))
				return;

			used.insert({getIt(a, lb), getIt(b, la)});

			const int n = output.size();

			if (la > lb)
				std::swap(la, lb);

			assert(lb + n * la < distances.size());
			distances[la + n * lb] += distance;
			assert(la + n * lb < distances.size());
			distances[lb + n * la] += distance;
		}

		void predict(int maxClusters)
		{
			// Sort ids
			std::vector<size_t> ids;
			for (size_t i = 0; i < distances.size(); i++)
			{
				ids.push_back(i);
			}

			std::sort(ids.begin(), ids.end(), [this](size_t aId, size_t bId)
				{
					return distances[aId] < distances[bId];
				});

			// std::sort(ids.begin(), ids.end(), [distances](size_t aId, size_t bId)
			// 	{
			// 		return distances[aId].diff < distances[bId].diff;
			// 	});


			int usedClusters = 0;
			const int n = output.size();
			std::fill(output.begin(), output.end(), 0);
			for (size_t i = 0; i < ids.size(); i++)
			{
				size_t id = ids[i];

				const LineId a = id / n;
				const LineId b = id % n;

				if (output[a] != 0)
				{
					if (output[b] != 0)
						continue;
					else
					{
						output[b] = output[a];
					}
				}
				else
				{
					if (output[b] != 0)
					{
						output[a] = output[b];
					}
					else if (usedClusters < maxClusters)
					{
						output[a] = output[a] = usedClusters++;
					}
				}
			}
		}
	};

	struct DiffItem
	{
		VertId aId;
		VertId bId;
		float diff;
	};
	using PointLine = std::vector<landres>;

	// Combine
	void predict(std::vector<PointLine>& land)
	{
		std::vector<DiffItem> diffs;

		Graph graph;
		graph.setLines(land.size());

		for (size_t i = 0; i < land.size() - 1; i++)
		{
			std::unordered_map<float, float> combinedPoints;

			PointLine& path = land[i];// all points of the landscape
			for (auto& point : path)
			{
				float y = point.y;
				auto it = combinedPoints.find(point.x);
				if (it == combinedPoints.end())
					combinedPoints.insert({y, 1.f});
				else
					it->second += y;
			}

			path.clear();
			for (auto &l : combinedPoints)
			{
				path.push_back( {l.first, l.second});
			}
		}

		for (size_t i = 0; i < land.size() - 1; i++)
		{
			const PointLine& path1 = land[i];
			for (size_t j = i + 1; j < land.size(); j++)
			{
				const PointLine& path2 = land[j];

				float curDiff = 0.f;
				for (size_t k0 = 0; k0 < path1.size(); k0++)
				{
					VertId vert0 = graph.addVert(i);
					for (size_t k1 = 0; k1 < path2.size(); k1++)
					{
						float curDiff = path1[k1] - path2[k0];

						VertId vert1 = graph.addVert(j);
						diffs.push_back({vert0, vert1, curDiff});
					}
				}
			}
		}
	}

	void predictAll(std::vector<PointLine>& land)
	{
		std::vector<DiffItem> diffs;

		Graph graph;
		graph.setLines(land.size());
		for (size_t i = 0; i < land.size() - 1; i++)
		{
			const PointLine& path1 = land[i];
			for (size_t j = i + 1; j < land.size(); j++)
			{
				const PointLine& path2 = land[j];

				float curDiff = 0.f;
				for (size_t k0 = 0; k0 < path1.size(); k0++)
				{
					VertId vert0 = graph.addVert(i);
					for (size_t k1 = 0; k1 < path2.size(); k1++)
					{
						float curDiff = path1[k1] - path2[k0];

						VertId vert1 = graph.addVert(j);
						diffs.push_back({vert0, vert1, curDiff});
					}
				}
			}
		}

		// Sort ids
		// std::vector<int> ids;
		// for (size_t i = 0; i < diffs.size(); i++)
		// {
		// 	ids.push_back(i);
		// }

		// std::sort(ids.begin(), ids.end(), [&diffs](size_t aId, size_t bId)
		// 	{
		// 		return diffs[aId].diff < diffs[bId].diff;
		// 	});

		std::sort(diffs.begin(), diffs.end(), [](const DiffItem& a, const DiffItem& b)
		{
			return a.diff < b.diff;
		});

		for (size_t i = 0; i < diffs.size(); i++)
		{
			const DiffItem& d = diffs[i];
			graph.connect(d.aId, d.bId, d.diff);
		}

		graph.predict(7);
		output = std::move(graph.output);
	}
};


struct Path
{
	std::vector<float> inter;
	float diff(const std::vector<float>& rhs)
	{
		assert(inter.size() == rhs.size());
		float diff;
		for (size_t i = 0; i < inter.size(); i++)
		{
			float a = inter[i] - rhs[i];
			diff = a * a;
		}

		return sqrt(diff);
	}

	int classId;
};
