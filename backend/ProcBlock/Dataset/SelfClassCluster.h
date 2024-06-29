#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>

class SelfCluster
{
	struct DiffItem
	{
		size_t srcI;
		size_t closerI;
		float closerDiff;
	};
	std::vector<int> results;


	int maxAllowed = 2;
	std::vector<int> output;
public:
	int test(int id)
	{
		return output[id];
	}

	void setClasses(int n)
	{
		maxAllowed = n;
		results.resize(n, 0);
	}

	struct Cluster
	{
		float sumDiff;
		std::vector<float> path1;
	};

	void predict(std::vector<std::vector<float>>& land)
	{
		std::vector<DiffItem> diffs;
		std::fill(results.begin(), results.end(), 0);

		int n = land[0].size();
		for (size_t i = 0; i < land.size() - 1; i++)
		{
			std::vector<float>& path1 = land[i];
			size_t closer = i + 1;
			float closestDiff = std::numeric_limits<float>::max();
			for (size_t j = i + 1; j < land.size(); j++)
			{
				std::vector<float>& path2 = land[j];

				float curDiff = 0.f;
				for (size_t k = 0; k < n; k++)
				{
					float doff = path1[k] - path2[k];
					curDiff += doff * doff;
				}

				// diffs.push_back({i, j, curDiff});

				curDiff = sqrt(curDiff);
				if (curDiff < closestDiff)
				{
					closer = j;
					closestDiff = curDiff;
				}
			}

			diffs.push_back({i, closer, closestDiff});
		}

		// Sort ids
		std::sort(diffs.begin(), diffs.end(), [](auto& a, auto& b)
			{
				return a.closerDiff < b.closerDiff;
			});

		int usedClusters = 0;
		output.resize(land.size());
		std::fill(output.begin(), output.end(), -1);
		for (size_t i = 0; i < diffs.size(); i++)
		{
			DiffItem& d = diffs[i];

			int a = d.srcI;
			int b = d.closerI;

			if (output[a] != -1)
			{
				if (output[b] != -1)
					continue;
				else // output[b] == -1
				{
					output[b] = output[a];
				}
			}
			else
			{
				if (output[b] != -1)
				{
					output[a] = output[b];
				}
				else// if (usedClusters < maxAllowed)
				{
					output[a] = output[b] = usedClusters++; // Clusters numbers with 0
				}
			}

		}

		if (usedClusters <= maxAllowed)
			return;

		std::vector<std::vector<float>> ot(usedClusters);
		std::vector<int> counterSize(usedClusters);
		std::vector<std::vector<int>> idsFromOld(usedClusters);
		for (size_t i = 0; i < diffs.size(); i++)
		{
			int id = output[i];

			if (counterSize[id] == 0)
			{
				idsFromOld[id].push_back(i);
				ot[id] = land[i];
			}
			else
			{
				// Add vectors ot[id] += land[i];
				for (size_t j = 0; j < land[i].size(); j++)
				{
					ot[id][j] += land[i][j];
				}
			}

			++counterSize[id];
		}

		for (size_t i = 0; i < usedClusters; i++)
		{
			for (size_t j = 0; j < land[i].size(); j++)
			{
				ot[i][j] /= counterSize[i];
			}
		}


		// TODO:
		auto prot = output;
		predict(ot);
		assert(output.size() == usedClusters);
		for (size_t i = 0; i < usedClusters; i++)
		{
			int id = output[i];
		}
	}
};


class SelfClass
{
	using Path = std::vector<float>;
	struct ClassSet
	{
		int id;
		std::vector<Path> etalons;

		void add(Path&& p)
		{
			etalons.push_back(std::move(p));
		}

		static float calcDiff(const Path& a, const Path& b)
		{
			assert(a.size() == b.size());
			float diff = 0;
			for (size_t i = 0; i < a.size(); i++)
			{
				float d = a[i] - b[i];
				diff += d * d;
			}

			return sqrt(diff);
		}

		float calcDistance(const Path& a)
		{
			float minDiff = 999999999999999.0f;
			for (size_t i = 0; i < etalons.size(); i++)
			{
				Path& e = etalons[i];

				float diff = calcDiff(a, e);
				if (diff < minDiff)
				{
					minDiff = diff;
				}
			}
			return sqrt(minDiff);
		}
	};

	std::vector<ClassSet> classes;
	using ClassId = int;
	std::vector<ClassId> output;
public:
	void setClasses(int n)
	{
		classes.resize(n);
	}

	void addToTrain(int id, std::vector<float>&& path)
	{
		assert(id < classes.size());
		classes[id].add(std::forward<Path>(path));
	}

	int test(int inputId)
	{
		assert(inputId < output.size());
		return output[inputId];
	}

	void predict(std::vector<std::vector<float>>& land)
	{
		const size_t count = land.size();
		output.resize(count);
		for (size_t l = 0; l < count; l++)
		{
			Path& p = land[l];
			ClassId minClass = 0;
			float minDiff = std::numeric_limits<float>::max();
			for (size_t i = 0; i < classes.size(); i++)
			{
				auto& c = classes[i];

				float diff = c.calcDistance(p);
				if (diff < minDiff)
				{
					minDiff = diff;
					minClass = i;
				}
			}
			output[l] = minClass;
		}
	}
};
