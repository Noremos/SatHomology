#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "../../Clusterizers/LandscapeItem.h"


class PointCluster
{

	// int results[7];

	int maxClusters;
	// std::unordered_map<int,int> used;
public:
	int test(int id)
	{
		return output[id];
	}

	void setClasses(int n)
	{
		maxClusters = n;
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
	using BubbleId = int;

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
			std::fill(distances.begin(), distances.end(), -1.f);
		}

		VertId vertCounter = 0;
		VertId addVert(LineId lineId)
		{
			vertToLine.push_back(lineId);
			return vertCounter++;
		}
		std::vector<VertId> addVerts(LineId lineId, int size)
		{
			std::vector<VertId> out;
			for (int i = 0; i < size; i++)
				out.push_back(addVert(lineId));

			return out;
		}

		void setDifference(LineId la, LineId lb, float distance)
		{
			if (la > lb)
				std::swap(la, lb);

			const int n = output.size();
			// Расстояние от линии la до линии lb
			{
				size_t ladist = lb + n * la;
				assert(ladist < distances.size());
				distances[ladist] = distance;
			}
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

		bool hasConnection(VertId a, LineId l) const
		{
			return used.count(getIt(a, l)) != 0;
		}
		bool hasConnection(size_t id) const
		{
			return used.count(id) != 0;
		}

		void connect(VertId a, VertId b, float distance)
		{
			assert(distance >= 0);

			LineId la = vertToLine[a];
			LineId lb = vertToLine[b];
			if (la == lb)
				return;

			// Проверяем, что уже есть соединение к точки a из линии lb
			// size_t atolb = getIt(a, lb);
			// size_t btola = getIt(b, la);


			if (hasConnection(a) && hasConnection(b))
				return;

			used.insert(a);
			used.insert(b);

			const int n = output.size();

			if (la > lb)
				std::swap(la, lb);

			// Расстояние от линии la до линии lb
			{
				size_t ladist = lb + n * la;
				assert(ladist < distances.size());

				if (distances[ladist] < 0)
					distances[ladist] = distance;
				else
					distances[ladist] += distance;
			}

			// {
			// 	size_t lbdist = la + n * lb;
			// 	assert(lbdist < distances.size());
			// 	distances[lbdist] += distance;
			// }
		}

		struct ProtoClust
		{
			int& usedId;
			int id;
			int size = 0;

			int clusId = -1;
			int maxClausers = 7;

			ProtoClust(int& usedId, int id, int maxClausers) : usedId(usedId), id(id), maxClausers(maxClausers) {}

			void add(int dd = 1)
			{
				size += dd;
				if (size >= 3 && usedId < maxClausers)
				{
					clusId = usedId++;
				}
			}

			bool proto() const
			{
				return clusId == -1;
			}
		};

		void predict(int maxClusters)
		{
			// Sort ids
			const int n = output.size();

			std::vector<size_t> ids;
			for (size_t i = 0; i < distances.size(); i++)
			{
				if (distances[i] < 0)
					continue;

				ids.push_back(i);
			}

			std::sort(ids.begin(), ids.end(), [this](size_t aId, size_t bId)
				{
					return distances[aId] < distances[bId];
				});

			int usedClusters = 0;
			std::fill(output.begin(), output.end(), -1);

			std::vector<std::shared_ptr<ProtoClust>> clusters;
			// std::unordered_map<LineId, int> protos;

			for (size_t i = 0; i < ids.size(); i++)
			{
				size_t id = ids[i];

				const LineId a = id / n;
				const LineId b = id % n;
				assert(a != b);

				if (output[a] != -1)
				{
					if (output[b] != -1)
					{
						if (output[a] == output[b])
							continue;

						ProtoClust* pa = clusters[output[a]].get();
						ProtoClust* pb = clusters[output[b]].get();
						if (pa->proto() && !pb->proto())
						{
							output[a] = output[b];
							pb->add(1);
							pa->size--;
						}
						else if (!pa->proto() && pb->proto())
						{
							output[b] = output[a];
							pa->add(1);
							pb->size--;
						}
						else if (pa->proto() && pb->proto())
						{
							pa->add(pa->size);
							clusters[output[b]] = clusters[output[a]];
						}
					}
					else // b == 0
					{
						output[b] = output[a];
						clusters[output[a]]->add(1);
					}
				}
				else // a == -1
				{
					if (output[b] != -1)
					{
						output[a] = output[b];
						clusters[output[b]]->add(1);
					}
					else // if (usedClusters < maxClusters)
					{
						// clusters.push_back({});
						// auto& pc = clusters.back();
						// pc.items.push_back(a);
						// pc.items.push_back(b);
						int pid = clusters.size();
						auto p = std::make_shared<ProtoClust>(usedClusters, pid, maxClusters);
						clusters.push_back(p);
						p->add(2);
						// protos.insert({a, pid});
						// protos.insert({b, pid});
						output[a] = output[a] = pid; // Clusters numbers with 0
					}
				}
			}


			for (size_t i = 0; i < output.size(); i++)
			{
				output[i] = clusters[output[i]]->clusId;
			}


			// std::unordered_map<int, int> protoIdToRealId;
			// for (size_t i = 0; i < clusters.size(); i++)
			// {
			// 	auto& pc = clusters[i];
			// 	if (pc->proto())
			// 		continue;
			// 	auto p = protoIdToRealId.find(pc->id);
			// 	if (p != protoIdToRealId.end())
			// 		protoIdToRealId.insert({pc->id, usedClusters++});

			// }
		}
	};

	struct DiffItem
	{
		VertId aId;
		VertId bId;
		float diff;
	};
	using PointLine = std::vector<LandPoint>;

	// Combine
	void predict(std::vector<PointLine>& land)
	{
		std::vector<DiffItem> diffs;

		Graph graph;
		graph.setLines(land.size());

		for (LineId i = 0; i < land.size() - 1; i++)
		{
			const PointLine& path1 = land[i];
			std::vector<VertId> verts1 = graph.addVerts(i, path1.size());

			for (LineId j = i + 1; j < land.size(); j++)
			{
				const PointLine& path2 = land[j];
				std::vector<VertId> verts2 = graph.addVerts(j, path2.size());

				for (size_t k1 = 0; k1 < path1.size(); k1++)
				{
					VertId vert1 = verts1[k1];
					for (size_t k2 = 0; k2 < path2.size(); k2++)
					{
						float curDiff = path1[k1] - path2[k2];
						// is nan
						assert(!std::isnan(curDiff));

						VertId vert2 = verts2[k2];
						diffs.push_back({vert1, vert2, curDiff});
					}
				}

				// // Линия 1 может полносттью совпадать с ниней 2, но линия 2 может продолжаться
				// if (path1.size() < path2.size())
				// {
				// 	float dist = 0;
				// 	for (size_t d = path1.size(); d < path2.size(); d++)
				// 	{
				// 		dist += path1.back() - path2[d];
				// 	}
				// 	graph.setDifference(i,j, dist);
				// }
				// else if (path2.size() < path1.size())
				// {
				// 	float dist = 0;
				// 	for (size_t d = path2.size(); d < path1.size(); d++)
				// 	{
				// 		dist += path2.back() - path1[d];
				// 	}
				// 	graph.setDifference(i,j, dist);
				// }
			}
		}

		std::sort(diffs.begin(), diffs.end(), [](const DiffItem& a, const DiffItem& b)
		{
			return a.diff < b.diff;
		});

		for (size_t i = 0; i < diffs.size(); i++)
		{
			const DiffItem& d = diffs[i];
			graph.connect(d.aId, d.bId, d.diff);
		}

		graph.predict(maxClusters);
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
