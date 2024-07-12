#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>
#include "../../Clusterizers/LandBase.h"


class LandscapeCluster
{
	int maxAllowed = 2;
	std::vector<unsigned long> output;
public:
	int test(int id)
	{
		return output[id];
	}

	void setClasses(int n)
	{
		maxAllowed = n;
	}

	struct Cluster
	{
		float sumDiff;
		std::vector<float> path1;
	};


	int curFUnc = 0;
	const char* methodName = "hierarchical";
	float iterationStep = 0.5f;
	void predict(std::vector<Landscape>& landscapes);

};