#include <algorithm>
#include <cassert>
#include <memory>
#include <random>
#include <vector>
#include <unordered_map>

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
		if(x < t->key)
		{
			if (t->left)
				return findClosest(t->left, x);
			else
			 	return t;
		}
		else if(x > t->key)
		{
			if (t->right)
				return findClosest(t->right, x);
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

	float value;
};

template <int TLen>
class Trainer
{
public:
	using Hash = HashK<TLen>;

	struct Line
	{
		GetByChance value;
		int id = 0;
		// int marks = 0;
		// int nodesId[TLen];
	};

	class Node
	{
	public:
		// Line* line = nullptr;
		Node(Line* l, float edge)
		{
			addLine(l);
			this->edge = edge;
		}

		void addLine(Line* l)
		{
			lines.push_back(l);
		}
		std::vector<Line*> lines;
		float edge; // Edge
	};

	struct KeyColumn
	{
		std::vector<Node*> nodes;
		// std::vector<Line*> lines;
		BST<float, Node*> nodesIndex;

		void addNode(Node* node, int) // id
		{
			nodes.push_back(node);
			nodesIndex.insert(node->edge, node);
		}

		void sort()
		{
			std::sort(nodes.begin(), nodes.end(), [](const Node* a,const Node* b) {
				// for (int i = 0; i < 9; ++i)
				// {
				// 	if (a.key[i] != b.key[i])
				// 		return a.key[i] < b.key[i];
				// }
				return a->edge < b->edge;
			});
		}

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
			Node* val = cache[j] = temp ? *temp : nullptr;

			if (val == nullptr)
			{
				same = false;
				break;
			}

			if (j == 0)
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
			assert(found.size() == 1);
			found[0]->value.add(scase.value);  // TODO: reduce .00001 to 0.1
			return;
		}

		linesCollector.push_back(std::make_unique<Line>());
		Line* line = linesCollector.back().get();
		line->id = linesCollector.size() - 1;
		line->value.add(scase.value);

		for (int j = 0; j < TLen; ++j)
		{
			KeyColumn& column = columns[j];
			Node* val = cache[j];
			if (val == nullptr)
			{
				nodeCollector.push_back(std::make_unique<Node>(line, scase.key[j]));
				val = nodeCollector.back().get();
				column.nodesIndex.insert(scase.key[j], val);
			}
			else
				val->lines.push_back(line);

			// line->nodesId[j] = val;
		}
	}

	void train()
	{ }

	int getCloser(const Hash& scase)
	{
		std::unordered_map<int, int> closesLines;
		for (int j = 0; j < TLen; ++j)
		{
			columns[j].findCloser(scase.key[j], closesLines);
		}
		auto max = std::max_element(closesLines.begin(), closesLines.end());

		// std::vector<std::pair<int, float>> diffs;
		// for (int j = 0; j < closesLines.size(); ++j)
		// {
		// 	float diff = 0;
		// 	auto& closestTrain = train[closesLines[j]->id];
		// 	for (int j = 0; j < TLen; ++j)
		// 	{
		// 		float a = closestTrain.key[j] - scase.key[j];
		// 		diff += a * a;
		// 	}
		// 	diff = sqrt(diff);
		// 	diffs.push_back({j, diff});

		// 	closesLines[j]->marks = 0;
		// }

		// auto y = std::min_element(diffs.begin(), diffs.end(), [](auto a, auto b) {
		// 	return a.second < b.second;
		// });


		return max->first;
	}

	// std::vector<Hash> trainData;
	KeyColumn columns[TLen];
	std::vector<std::unique_ptr<Line>> linesCollector;
	std::vector<std::unique_ptr<Node>> nodeCollector;
};
