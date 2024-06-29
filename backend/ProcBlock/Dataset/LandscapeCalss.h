#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>
#include "../../Clusterizers/LandBase.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <memory>
#include <set>

struct Point {
    double x, y;

    Point(double x, double y) : x(x), y(y) {}
};

double distance(const Point& a, const Point& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}
class MatrixXd
{
public:
	int rows, cols;
	std::unique_ptr<double[]> data;
	MatrixXd(int rows, int cols) : rows(rows), cols(cols)
	{
		data = std::make_unique<double[]>(rows * cols);
	}

	double& operator()(int i, int j)
	{
		return get(i, j);
	}
	double operator()(int i, int j) const
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		return data[i * cols + j];
	}

	double& get(int i, int j)
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		return data[i * cols + j];
	}

	MatrixXd(const MatrixXd& other) : rows(other.rows), cols(other.cols)
	{
		data = std::make_unique<double[]>(rows * cols);
		std::copy(other.data.get(), other.data.get() + rows * cols, data.get());
	}

	MatrixXd& operator=(const MatrixXd& other)
	{
		rows = other.rows;
		cols = other.cols;
		data = std::make_unique<double[]>(rows * cols);
		std::copy(other.data.get(), other.data.get() + rows * cols, data.get());
		return *this;
	}

	MatrixXd& operator=(MatrixXd&& other)
	{
		if (this == &other)
			return *this;

		rows = other.rows;
		cols = other.cols;
		data = std::move(other.data);
		return *this;
	}


	void updateDistances(int numClusters, int cluster1, int cluster2)
	{
		int newClusterSize = numClusters - 1;
		MatrixXd newDistances(newClusterSize, newClusterSize);
		for (int i = 0; i < newClusterSize; ++i) {
			for (int j = 0; j < newClusterSize; ++j)
			{
				newDistances(i, j) = get(i, j);
			}
		}

		if (cluster2 < newClusterSize && cluster1 < newClusterSize)
		{
			newDistances(cluster1, cluster2) = -1;
			newDistances(cluster2, cluster1) = -1;
		}

		int newCluster = numClusters - 2;
		for (int i = 0; i < newCluster; ++i)
		{
			if (i < cluster1)
			{
				newDistances(i, newCluster) = std::min(get(i, cluster1), get(i, cluster2));
			}
			else if (i < cluster2)
			{
				newDistances(i, newCluster) = std::min(get(cluster1, i), get(i, cluster2));
			}
			else
			{
				newDistances(i, newCluster) = std::min(get(cluster1, i+1), get(cluster2, i+1));
			}
			newDistances(newCluster, i) = newDistances(i, newCluster);
		}

		newDistances(newCluster, newCluster) = 0;
		*this = std::move(newDistances);
	}


	std::pair<int, int> findMinDistance(int numClusters) {
		double minDistance = std::numeric_limits<double>::max();
		std::pair<int, int> minPair = { -1, -1 };
		for (int i = 0; i < numClusters; ++i) {
			for (int j = i + 1; j < numClusters; ++j)
			{
				float val = get(i, j);
				if (val == -1)
					continue;
				if (i == j)
					continue;

				if (val < minDistance)
				{
					minDistance = get(i, j);
					minPair = { i, j };
				}
			}
		}
		return minPair;
	}
};


struct Cluster
{
    std::vector<size_t> points;

    Cluster(const size_t& point)
	{
        points.push_back(point);
    }

    void merge(const Cluster& other)
	{
        points.insert(points.end(), other.points.begin(), other.points.end());
    }

    // Point centroid() const {
    //     double x_sum = 0.0, y_sum = 0.0;
    //     for (const auto& point : points) {
    //         x_sum += point.x;
    //         y_sum += point.y;
    //     }
    //     return Point(x_sum / points.size(), y_sum / points.size());
    // }
};

std::vector<int> hierarchicalClustering(const MatrixXd& distanceMatrix, int num_clusters)
{
    int numRows = distanceMatrix.rows;
    int numClusters = numRows;

    MatrixXd distances = distanceMatrix;

	std::vector<Cluster*> clusters;
	for (size_t i = 0; i < numRows; i++)
	{
		clusters.push_back(new Cluster(i));
	}

	while (numClusters > num_clusters)
	{
		std::pair<int, int> minPair = distances.findMinDistance(numClusters);
		std::cout << "Объединяются кластеры " << minPair.first << " и " << minPair.second << std::endl;

		clusters[minPair.first]->merge(*clusters[minPair.second]);
		clusters[minPair.second] = clusters[minPair.first];

		distances.updateDistances(numClusters, minPair.first, minPair.second);
		--numClusters;
	}

	// Keep only uniqe clusters

	std::vector<int> outputIds;
	std::unordered_map<size_t, int> uniqeIds;
	for (size_t i = 0; i < clusters.size(); i++)
	{
		auto it = uniqeIds.find(clusters[i]->points[0]);
		int fid = 0;
		if (it == uniqeIds.end())
		{
			fid = (int)uniqeIds.size();
			uniqeIds.insert(std::pair<size_t, int>{clusters[i]->points[0], fid});
		}
		else
			fid = it->second;

		outputIds.push_back(fid);
	}
	clusters.clear();

	return outputIds;
}


// struct Cluster
// {
//     std::vector<size_t> points;

//     Cluster(const size_t& point)
// 	{
//         points.push_back(point);
//     }

//     void merge(const Cluster& other)
// 	{
//         points.insert(points.end(), other.points.begin(), other.points.end());
//     }

//     Point centroid() const {
//         double x_sum = 0.0, y_sum = 0.0;
//         for (const auto& point : points) {
//             x_sum += point.x;
//             y_sum += point.y;
//         }
//         return Point(x_sum / points.size(), y_sum / points.size());
//     }
// };

// double cluster_distance(const Cluster& a, const Cluster& b) {
//     return distance(a.centroid(), b.centroid());
// }

// void hierarchical_clustering(std::vector<IterLandscape>& landscapes, int num_clusters)
// {
// 	std::vector<Cluster> clusters;

// 	for (size_t i = 0; i < landscapes.size(); i++)
// 	{
//         clusters.emplace_back(i);
// 	}

//     while (static_cast<int>(clusters.size()) > num_clusters) {
//         double min_distance = std::numeric_limits<double>::max();
//         size_t merge_idx1 = 0;
//         size_t merge_idx2 = 0;

//         for (size_t i = 0; i < clusters.size(); ++i) {
//             for (size_t j = i + 1; j < clusters.size(); ++j) {
//                 double dist = cluster_distance(clusters[i], clusters[j]);
//                 if (dist < min_distance) {
//                     min_distance = dist;
//                     merge_idx1 = i;
//                     merge_idx2 = j;
//                 }
//             }
//         }

//         clusters[merge_idx1].merge(clusters[merge_idx2]);
//         clusters.erase(clusters.begin() + merge_idx2);
//     }

//     for (const auto& cluster : clusters) {
//         std::cout << "Cluster: ";
//         for (const auto& point : cluster.points) {
//             std::cout << "(" << point << ", " << point << ") ";
//         }
//         std::cout << std::endl;
//     }
// }

class LandscapeCluster
{
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
	}

	struct Cluster
	{
		float sumDiff;
		std::vector<float> path1;
	};

	void predict(std::vector<Landscape>& landscapes)
	{
		int n = landscapes.size();

		MatrixXd matrix(n, n);
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = i + 1; j < n; j++)
			{
				matrix(j, i) = matrix(i, j) = iterLandDistanceSumInf(landscapes[i], landscapes[j], 0.1f);
			}
		}

		for (size_t i = 0; i < n; i++)
		{
			matrix(i, i) = 0;
		}

		output = hierarchicalClustering(matrix, maxAllowed);
	}
};
