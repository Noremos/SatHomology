module;
#include <cassert>
#include <algorithm>
#include <vector>
#include <iostream>
#include "Barcode/PrjBarlib/include/barstrucs.h"
#include "Common.h"

#include "../CachedBarcode.h"
#include "../MLSettings.h"
#include "../Interfaces/ICluster.h"
export module SortItem;


//import BackBind;
// import ClusterInterface;
import Platform;
//
// import CachedBarcode;
// import MLSettings;
//
//
//using SortClass = CachedBarline;
export class SortClass : public ICluster
{
public:
	const CachedBarline* bar;

	float sqr(int a) const
	{
		return a * a;
	}

	SortClass() : bar(nullptr)
	{ }

	SortClass(const CachedBarline* bar) : bar(bar)
	{
	}

	//SortClass(const SortClass& sm) : bar(sm.bar)
	//{
	//}

	//SortClass(SortClass&& sm) : bar(std::move(sm.bar))
	//{
	//	//sm.bar = nullptr;
	//}

	//SortClass& operator=(const SortClass& sm)
	//{
	//	bar = sm.bar;
	//	return *this;
	//}

	//SortClass& operator=(SortClass&& sm)
	//{
	//	bar = std::move(sm.bar);
	//	//sm.bar = nullptr;
	//	return *this;
	//}

	const CachedBarline& getBar() const
	{
		return *bar;
	}
	float getDiff(const SortClass* other) const
	{
		int a1 = getBar().getChildrenCount();
		int a2 = other->getBar().getChildrenCount();

		int b1 = getBar().getDeath();
		int b2 = other->getBar().getDeath();
		return sqrt(sqr(a1 - a2) + sqr(b1 - b2));
	}

	void getSignature(BackString& line) const override
	{
	}

	const bc::barvector& getMatrix() const override
	{
		return getBar().getMatrix();
	}
};

export class SortCollection : public IClusterItemHolder
{
	using Base = IClusterItemHolder;
protected:
	CachedBaritemHolder holder;
public:
	SortCollection() : Base(false)
	{
		Base::settings =
		{
		};
	}

	static int compareTree(const CachedBarline& a, const CachedBarline& b)
	{
		int ac = a.getChildrenCount();
		int bc = b.getChildrenCount();
		if (ac == bc)
		{
			for (int i = 0; i < ac; i++)
			{
				int ret = compareTree(*a.getChild(i), *b.getChild(i));
				if (ret != 0)
					return ret;
			}

			// Do not compare depths here beacus of recusive child cmp
			return 0;
		}

		return ac > bc ? 1 : -1;
	};

	virtual void perform()
	{
		std::sort(holder.getItems().begin(), holder.getItems().end(), [](const CachedBarline& a, const CachedBarline& b)
			{
				const int ret = compareTree(a, b);
				if (ret == 0)
				{
					return a.getDeath() > b.getDeath();
				}

				return ret > 0;
			});

		assert(holder.getItem(0)->getChildrenCount() > 0);
	}


	virtual void addItem(const CachedBarline& line)
	{
		//lines.addUpdateRoot(line);
		//lines.getLastItem().root = &lines;
		holder.getItems().push_back(line);
		holder.getLastItem().root = &holder;
	}
	const ICluster* getCItem(size_t id) const
	{
		static SortClass a(nullptr);
		a.bar = &holder.getItems()[id];
		return &a;
	}

	const CachedBarline* getRItem(size_t id) const
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
		// n = size;
	}

	int getClusters()
	{
		return n;
	}
	static float sqr(int a)
	{
		return a * a;
	}

	static float getDiff(const CachedBarline* a, const CachedBarline* b)
	{
		int a1 = a->getChildrenCount();
		int a2 = b->getChildrenCount();

		int b1 = a->getDeath();
		int b2 = b->getDeath();
		return sqrt(sqr(a1 - a2) + sqr(b1 - b2));
	}

	bool predict(const IClusterItemHolder& allItems)
	{
		n = *IBarClusterizer::settings.getInt("n_clusters");

		const SortCollection* col = dynamic_cast<const SortCollection*>(&allItems);
		float avgDiff = 0; int diffc = 0;
		std::vector<std::pair<int, int>> biggestRadius;
		size_t end = allItems.getItemsCount() - 1;
		for (size_t i = 1; i < allItems.getItemsCount() - 1; i++)
		{
			const CachedBarline* item = col->getRItem(i);
			if (item->getChildrenCount() < 2)
			{
				end = i;
				break;
			}

			const CachedBarline* next = col->getRItem(i + 1);
			float d = getDiff(item, next);
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

		if (n == 0)
		{
			[[maybe_unused]] float radius = *IBarClusterizer::settings.getInt("radius");
			avgDiff /= diffc;
			std::cout << "Avg diff: " << avgDiff << std::endl;
			for (int i = 0; i < biggestRadius.size(); i++)
			{
				if (biggestRadius[i].second < avgDiff)
				{
					biggestRadius.resize(i);
					break;
				}
			}
			n = biggestRadius.size() + 1;
			std::cout << "Classes count: " << n << std::endl;
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
