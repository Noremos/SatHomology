#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <random>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "Barcode/PrjBarlib/include/barscalar.h"

/*
** Binary Search Tree implementation in C++
** Harish R
*/
// #include<iostream>
// using namespace std;

template<class TKey, class TValue>
class BST {

	struct node
	{
		TKey key;
		TValue value;
		node* left = nullptr;
		node* right = nullptr;
	};
	node* root = nullptr;

	node* makeEmpty(node* t)
	{
		if(t == nullptr)
			return nullptr;

		makeEmpty(t->left);
		makeEmpty(t->right);
		delete t;

		return nullptr;
	}

	node* insert(TKey x, TValue val, node* t)
	{
		if (t == nullptr)
		{
			t = new node{x, val};
		}
		else if (x < t->key)
			t->left = insert(x, val, t->left);
		else if (x > t->key)
			t->right = insert(x, val, t->right);
		return t;
	}

	node* findMin(node* t)
	{
		if(t == nullptr)
			return nullptr;
		else if(t->left == nullptr)
			return t;
		else
			return findMin(t->left);
	}

	node* findMax(node* t)
	{
		if(t == nullptr)
			return nullptr;
		else if(t->right == nullptr)
			return t;
		else
			return findMax(t->right);
	}

	node* remove(TKey x, node* t)
	{
		node* temp;
		if(t == nullptr)
			return nullptr;
		else if(x < t->data)
			t->left = remove(x, t->left);
		else if(x > t->data)
			t->right = remove(x, t->right);
		else if(t->left && t->right)
		{
			temp = findMin(t->right);
			t->data = temp->data;
			t->right = remove(t->data, t->right);
		}
		else
		{
			temp = t;
			if(t->left == nullptr)
				t = t->right;
			else if(t->right == nullptr)
				t = t->left;
			delete temp;
		}

		return t;
	}

	// void inorder(node* t)
	// {
	// 	if(t == nullptr)
	// 		return;

	// 	inorder(t->left);
	// 	cout << t->data << " ";
	// 	inorder(t->right);
	// }

	static node* findClosest(node* t, TKey x)
	{
		float curDiff = abs(x - t->key);
		if (curDiff == 0)
			return t;


		if (x < t->key)
		{
			if (t->left)
			{
				node* node = findClosest(t->left, x);
				if (abs(node->key - x) < curDiff)
					return node;
				else
				 	return t;
			}
			else
			 	return t;
		}
		else if(x > t->key)
		{
			if (t->right)
			{
				node* node = findClosest(t->right, x);
				if (abs(node->key - x) < curDiff)
					return node;
				else
				 	return t;
			}
			else
			 	return t;
		}
		else
			return t;
	}

	static node* findExacly(node* t, TKey x)
	{
		if (t == nullptr)
			return nullptr;
		else if (x < t->key)
			return findExacly(t->left, x);
		else if (x > t->key)
			return findExacly(t->right, x);
		else // if (x == t->key)
			return t;
	}

	TKey getNodeChanceValue(node* t, TValue val)
	{
		val -= t->value;
		if (val <= 0)
			return t->key;

		if (t->left)
			return getNodeChanceValue(t->left, val);
		else if (t->right)
			return getNodeChanceValue(t->right, val);
		else
			return t->key;
	}

public:
	~BST()
	{
		root = makeEmpty(root);
	}

	void insert(TKey x, TValue val)
	{
		root = insert(x, val, root);
	}

	void remove(TKey x)
	{
		root = remove(x, root);
	}

	void display()
	{
		inorder(root);
		std::cout << std::endl;
	}

	TValue search(TKey x)
	{
		return findClosest(root, x)->value;
	}

	TValue* get(TKey x)
	{
		node* found = findExacly(root, x);
		return found ? &found->value : nullptr;
	}

	TKey getChanceValue(TValue val)
	{
		return getNodeChanceValue(root, val);
	}
};


struct GetByChance
{
	BST<float, int> values;
	int totalChanges;

	float get(std::mt19937& gen)
	{
		static std::uniform_int_distribution<> distrib(0, totalChanges);
		return values.getChanceValue(distrib(gen));
	}

	void add(float value)
	{
		++totalChanges;
		int* count = values.get(value);
		if (count != nullptr)
		{
			++count;
			return;
		}

		values.insert(value, 1);
	}
};


template <int TLen>
struct HashK
{
	float key[TLen];

	HashK() = default;
	HashK(std::initializer_list<float> vals)
	{
		assert(vals.size() == TLen);
		for (int i = 0; i < TLen; i++)
		{
			key[i] = vals.begin()[i];
		}
	}

	float value;
};

template <int TLen>
class Trainer
{
protected:
	const bool debugDraw;

public:
	using Hash = HashK<TLen>;
	Trainer(bool debugDraw = false) : debugDraw(debugDraw)
	{ }

	struct Line
	{
		GetByChance value;
		int id = 0;
		// int marks = 0;
		float edge[TLen];
	};

	class Node
	{
	public:
		// Line* line = nullptr;
		Node(Line* l, float edge, int id)
		{
			addLine(l, id);
			this->edge = edge;
		}

		void addLine(Line* l, int id)
		{
			l->edge[id] = edge;
			lines.push_back(l);
		}
		std::vector<Line*> lines;
		float edge; // Edge
	};

	struct KeyColumn
	{
		BST<float, Node*> nodesIndex;

		void findCloser(float value, std::unordered_map<int, int>& clines)
		{
			Node* fnd = nodesIndex.search(value);

			// float diff = abs(value - closer->edge);
			for (auto* line : fnd->lines)
			{
				auto it = clines.find(line->id);
				if (it != clines.end())
				{
					it->second++;
				}
				else
					clines[line->id] = 1;
			}
		}
	};

	void add(const Hash& scase)
	{
		std::vector<Line*> found;
		bool same = true;

		// Find exists
		Node* cache[TLen];
		std::fill(cache, cache + TLen, nullptr);
		for (int j = 0; j < TLen; ++j)
		{
			KeyColumn& column = columns[j];
			Node** temp = column.nodesIndex.get(scase.key[j]);
			Node* val = cache[j] = (temp ? *temp : nullptr);

			if (val == nullptr)
			{
				same = false;
				break;
			}

			if (found.empty())
			{
				found = val->lines;
				continue;
			}

			std::vector<Line*> cross;
			for (int i = 0; i < found.size(); i++)
			{
				for (int j = 0; j < val->lines.size();++j)
				{
					if (found[i] == val->lines[j])
					{
						cross.push_back(found[i]);
						break;
					}
				}
			}

			if (cross.size() == 0)
			{
				same = false;
				break;
			}
			else
			{
				found = std::move(cross);
			}
		}

		if (same)
		{
			if (debugDraw)
			{
				std::cout << found[0]->id << "(same): ";
				for (int j = 0; j < TLen; ++j)
				{
					std::cout << scase.key[j] << " ";
				}
				std::cout << "-> " << scase.value << std::endl;
			}
			assert(found.size() == 1);
			found[0]->value.add(scase.value);  // TODO: reduce .00001 to 0.1
			return;
		}

		linesCollector.push_back(std::make_unique<Line>());
		Line* line = linesCollector.back().get();
		line->id = linesCollector.size() - 1;
		line->value.add(scase.value);

		if (debugDraw)
		{
			std::cout << line->id << " (new): ";
			for (int j = 0; j < TLen; ++j)
			{
				std::cout << scase.key[j] << " ";
			}
			std::cout << "-> " << scase.value << std::endl;
		}

		for (int j = 0; j < TLen; ++j)
		{
			KeyColumn& column = columns[j];
			Node* val = cache[j];
			if (val == nullptr)
			{
				// Search for existence line
				Node** exists = column.nodesIndex.get(scase.key[j]);
				if (exists)
				{
					val = *exists;
					val->addLine(line, j);
				}
				else
				{
					// Create new
					nodeCollector.push_back(std::make_unique<Node>(line, scase.key[j], j));
					val = nodeCollector.back().get();
					column.nodesIndex.insert(scase.key[j], val);
				}
			}
			else
				val->addLine(line, j);
		}
	}

	void train()
	{ }

	// void calcDiff(Line* line, const Hash& scase)
	// {
	// 	float diff = 0;
	// 	for (int j = 0; j < TLen; ++j)
	// 	{
	// 		float a = line->edge[j] - scase.value[j];
	// 		diff += a * a;
	// 	}

	// 	diff = sqrt(double(diff));
	// 	return diff;
	// }

	int getCloser(const Hash& scase)
	{
		std::unordered_map<int, int> closesLines;
		for (int j = 0; j < TLen; ++j)
		{
			if (debugDraw)
				std::cout << scase.key[j] << " ";

			columns[j].findCloser(scase.key[j], closesLines);
		}
		// auto maxel = std::max_element(closesLines.begin(), closesLines.end(), [](auto a, auto b) {return a.second < b.second; });
		// return maxel->first;


		int minId = closesLines.begin()->first;
		int minCounter = closesLines.begin()->second;
		int minVal = 99999;
		std::vector<std::pair<int, float>> diffs;
		for (auto it : closesLines)
		{
			if (it.second > minCounter)
			{
				minId = it.first;
				minCounter = it.second;
				continue;
			}
			else if (it.second == minCounter)
			{
				if (it.first == minId)
					continue;

				float diff1 = 0;
				float diff2 = 0;
				Line* line1 = linesCollector[it.first].get();
				Line* line2 = linesCollector[minId].get();
				for (int j = 0; j < TLen; ++j)
				{
					float a = line1->edge[j] - scase.key[j];
					diff1 += a * a;

					a = line2->edge[j] - scase.key[j];
					diff2 += a * a;
				}
				diff1 = sqrt(diff1);
				diff2 = sqrt(diff2);
				if (diff1 < diff2)
				{
					minId = it.first;
					minCounter = it.second;
				}
			}
		}

		return minId;
	}

	// std::vector<Hash> trainData;
	KeyColumn columns[TLen];
	std::vector<std::unique_ptr<Line>> linesCollector;
	std::vector<std::unique_ptr<Node>> nodeCollector;
};