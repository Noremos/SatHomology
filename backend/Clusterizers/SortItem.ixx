module;
#include <cassert>
#include <algorithm>
#include <vector>
#include "../../Bind/Common.h"

export module SortItem;


import ClusterInterface;
import Platform;
//
import CachedBarcode;
import MLSettings;
//
//
export class SortClass : public ICluster
{
public:
	float sqr(int a) const
	{
		return a * a;
	}
	SortClass() : bar()
	{ }

	SortClass(const CachedBarline* bar) : bar(*bar)
	{
	}

	SortClass(const SortClass& sm) : bar(sm.bar)
	{
	}

	SortClass(SortClass&& sm) : bar(std::move(sm.bar))
	{
		//sm.bar = nullptr;
	}

	SortClass& operator=(const SortClass& sm)
	{
		bar = sm.bar;
		return *this;
	}

	SortClass& operator=(SortClass&& sm)
	{
		bar = std::move(sm.bar);
		//sm.bar = nullptr;
		return *this;
	}

	CachedBarline bar;
	const CachedBarline& getBar() const
	{
		return bar;
	}
	float getDiff(const SortClass* other) const
	{
		int a1 = getBar().getChildrenCount();
		int a2 = other->getBar().getChildrenCount();

		int b1 = getBar().getDeath();
		int b2 = other->getBar().getDeath();
		return sqrt(sqr(a1 - a2) + sqr(b1 - b2));
	}

	const bc::barvector& getMatrix() const
	{
		return getBar().getMatrix();
	}
};

export class SortCollection : public IClusterItemValuesHolder<SortClass>
{
	using Base = IClusterItemValuesHolder<SortClass>;
protected:

public:
	SortCollection() : Base(false)
	{
		Base::settings =
		{
		};
	}

	CachedBaritemHolder lines;
	virtual void perform()
	{
		std::sort(Base::items.begin(), Base::items.end(), [](const SortClass& a, const SortClass& b)
			{
				int ac = a.getBar().getChildrenCount();
				int bc = b.getBar().getChildrenCount();
				if (ac == bc)
				{
					return a.getBar().getDeath() > b.getBar().getDeath();
				}

				return a.getBar().getChildrenCount() > b.getBar().getChildrenCount();
			});

		assert(Base::items[0].getBar().getChildrenCount() > 0);
	}


	virtual void addItem(const CachedBarline& line)
	{
		//lines.addUpdateRoot(line);
		//lines.getLastItem().root = &lines;
		Base::items.push_back({});
		Base::items.back().bar = line;
	}
};


export class SplitClassifier : public IBarClusterizer
{
	int n;
	std::vector<unsigned long> cachedAssignments;
public:
	SplitClassifier()
	{
		IBarClusterizer::settings =
		{
			{"n_clusters", 3},
			{"radius", 5.0}
		};
	}
	const BackString name() const
	{
		return "SPLIT";
	}

	void setClassesCount(int size)
	{
		n = size;
	}

	int getClusters()
	{
		return n;
	}

	bool predict(const IClusterItemHolder& allItems)
	{
		float avgDiff = 0; int diffc = 0;
		std::vector<std::pair<int, int>> biggestRadius;
		size_t end = allItems.getItemsCount() - 1;
		for (size_t i = 1; i < allItems.getItemsCount() - 1; i++)
		{
			const SortClass* item = static_cast<const SortClass*>(allItems.getItem(i));
			if (item->getBar().getChildrenCount() < 2)
			{
				end = i;
				break;
			}

			const SortClass* next = static_cast<const SortClass*>(allItems.getItem(i + 1));
			float d = item->getDiff(next);
			if (d > 0)
			{
				avgDiff += d;
				++diffc;
			}
			biggestRadius.push_back(std::make_pair<int, int>(i, d));
		}

		std::sort(biggestRadius.begin(), biggestRadius.end(), [](const auto& a, const auto& b)
			{
				return a.second > b.second;
			});

		n = *IBarClusterizer::settings.getInt("n_clusters");
		if (n == 0)
		{
			float radius = *IBarClusterizer::settings.getInt("radius");
			avgDiff /= diffc;
			for (int i = 0; i < biggestRadius.size(); i++)
			{
				if (biggestRadius[i].second < avgDiff)
				{
					biggestRadius.resize(i);
					break;
				}
			}
			n = biggestRadius.size() + 1;
			if (n <= 0)
			{
				for (size_t i = 0; i < allItems.getItemsCount(); i++)
				{
					cachedAssignments.push_back(-1);
				}
				return true;
			}
		}
		else
		{
			biggestRadius.resize(n - 1);
		}

		std::sort(biggestRadius.begin(), biggestRadius.end(), [](const auto& a, const auto& b)
			{
				return a.first < b.first;
			});
		assert(biggestRadius.size() < 2 || (biggestRadius[0] < biggestRadius[1]));


		int curId = -1;
		int toId = 0;

		size_t total = allItems.getItemsCount();
		for (size_t i = 0; i < end; i++)
		{
			cachedAssignments.push_back(curId);
			if (i == toId)
			{
				++curId;
				if (curId == biggestRadius.size())
				{
					toId = total;
				}
				else
					toId = biggestRadius[curId].first;
			}
		}
		for (size_t i = end; i < total; i++)
		{
			cachedAssignments.push_back(-1);
		}

		return true;
	}

	int test(size_t itemId)
	{
		return cachedAssignments[itemId];
	}
};


GlobalClusterRegister<SortClass, SortCollection, SplitClassifier> d0("sored ");
