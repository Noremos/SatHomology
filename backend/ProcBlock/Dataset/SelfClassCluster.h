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
	int results[NC];

	std::vector<int> output;
public:
	int test(int id)
	{
		return output[id];
	}

	void predict(std::vector<std::vector<float>>& land)
	{
		std::vector<DiffItem> diffs;
		std::fill_n(results, N, 0);

		int n = land[0].size();
		for (size_t i = 0; i < land.size() - 1; i++)
		{
			std::vector<float>& path1 = land[i];
			size_t closer = i + 1;
			float closestDiff = 999999999999999.0f;
			for (size_t j = i + 1; j < land.size(); j++)
			{
				std::vector<float>& path2 = land[j];

				float curDiff = 0.f;
				for (size_t k = 0; k < n; k++)
				{
					float doff = path1[i] - path2[k];
					curDiff +=  doff * doff;
				}

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
		std::vector<int> ids;
		for (size_t i = 0; i < diffs.size(); i++)
		{
			ids.push_back(i);
		}

		std::sort(ids.begin(), ids.end(), [&diffs](size_t a, size_t b)
			{
				return diffs[a].closerDiff < diffs[b].closerDiff;
			});

		int usedClusters = 0;
		output.resize(land.size());
		std::fill(output.begin(), output.end(), -1);
		for (size_t i = 0; i < ids.size(); i++)
		{
			int id = ids[i];
			DiffItem& d = diffs[id];

			int a = d.srcI;
			int b = d.closerI;

			if (output[a] != -1)
			{
				if (output[b] != -1)
					continue;
				else
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
				else if (usedClusters < NC)
				{
					output[a] = output[b] = usedClusters++; // Clusters numbers with 0
				}
			}

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

	ClassSet classes[NC];

	using ClassId = int;
	std::vector<ClassId> output;
public:
	void addToTrain(int id, std::vector<float>&& path)
	{
		assert(id < NC);
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
			for (size_t i = 0; i < NC; i++)
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
