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
	std::vector<double> data;
	MatrixXd(int rows, int cols) : rows(rows), cols(cols)
	{
		data.resize(rows * cols);
	}

	double& operator()(int i, int j)
	{
		return get(i, j);
	}
	double operator()(int i, int j) const
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		auto value = data[i * cols + j];
		assert(value == data[j * cols + i]);
		return value;
	}

	double& get(int i, int j)
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		auto value = data[i * cols + j];
		assert(value == data[j * cols + i]);
		return value;
	}


	void setBoth(int i, int j, double val)
	{
		assert(i >= 0 && i < rows && j >= 0 && j < cols);
		assert(data[i * cols + j] == data[j * cols + i]);
		data[j * cols + i]  = data[i * cols + j] = val;
	}

	std::vector<std::vector<double>> getMatr() const
	{
		std::vector<std::vector<double>> m;
		m.resize(rows);
		for (size_t i = 0; i < rows; i++)
		{
			m[i].resize(rows);

			for (size_t j = 0; j < rows; j++)
			{
				m[i][j] = operator()(i, j);
			}
		}

		return m;
	}

	MatrixXd(const MatrixXd& other) : rows(other.rows), cols(other.cols)
	{
		data = other.data;
	}

	MatrixXd& operator=(const MatrixXd& other)
	{
		rows = other.rows;
		cols = other.cols;
		data = other.data;
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
		double minVal = get(cluster1, cluster2);
		setBoth(cluster1, cluster2, -1);

		// for (int i = 0; i < numClusters; ++i)
		// {
		// 	if (i == cluster1 || i == cluster2)
		// 		continue;

		// 	setBoth(cluster1, i, std::min(get(i, cluster1), minVal));
		// 	setBoth(cluster2, i, std::min(get(i, cluster2), minVal));
		// }
	}


	std::pair<int, int> findMinDistance(int numClusters)
	{
		double minDistance = std::numeric_limits<double>::max();
		std::pair<int, int> minPair = { -1, -1 };
		for (int i = 0; i < rows; ++i)
		{
			for (int j = i + 1; j < cols; ++j)
			{
				float val = get(i, j);
				if (val == -1)
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


using namespace std;

class HierarchicalClustering2 {
public:
    HierarchicalClustering2(const vector<vector<double>>& distMatrix, int numClusters)
        : distanceMatrix(distMatrix), desiredClusters(numClusters) {
        int n = distanceMatrix.size();
        for (int i = 0; i < n; ++i) {
            clusters.push_back({i});
        }
    }

    void cluster() {
        while (clusters.size() > desiredClusters) {
            pair<int, int> closestPair = findClosestClusters();
            mergeClusters(closestPair.first, closestPair.second);
        }
    }

    void printClusters() {
        for (const auto& cluster : clusters) {
            for (int idx : cluster) {
                cout << idx << " ";
            }
            cout << endl;
        }
    }

private:
    vector<vector<double>> distanceMatrix;
    vector<vector<int>> clusters;
    int desiredClusters;

    pair<int, int> findClosestClusters() {
        double minDistance = numeric_limits<double>::max();
        pair<int, int> closestPair = {-1, -1};

        for (size_t i = 0; i < clusters.size(); ++i) {
            for (size_t j = i + 1; j < clusters.size(); ++j) {
                double dist = calculateDistance(clusters[i], clusters[j]);
                if (dist < minDistance) {
                    minDistance = dist;
                    closestPair = {i, j};
                }
            }
        }
        return closestPair;
    }

    double calculateDistance(const vector<int>& clusterA, const vector<int>& clusterB) {
        double minDistance = numeric_limits<double>::max();
        for (int i : clusterA) {
            for (int j : clusterB) {
                minDistance = min(minDistance, distanceMatrix[i][j]);
            }
        }
        return minDistance;
    }

    void mergeClusters(int idxA, int idxB) {
        clusters[idxA].insert(clusters[idxA].end(), clusters[idxB].begin(), clusters[idxB].end());
        clusters.erase(clusters.begin() + idxB);
    }
};

std::vector<int> hierarchicalClustering(const MatrixXd& distanceMatrix, int maxAllowed)
{
	// HierarchicalClustering2 hc(distanceMatrix.getMatr(), maxAllowed);
	// hc.cluster();
    // hc.printClusters();


    int numRows = distanceMatrix.rows;
    int numClusters = numRows;

    MatrixXd distances = distanceMatrix;

	std::vector<std::shared_ptr<Cluster>> clusters;
	for (size_t i = 0; i < numRows; i++)
	{
		clusters.push_back(std::make_shared<Cluster>(i));
	}

	while (numClusters > maxAllowed)
	{
		std::pair<int, int> minPair = distances.findMinDistance(numClusters);
		std::cout << "Объединяются кластеры " << minPair.first << " и " << minPair.second << std::endl;

		auto& a = clusters.at(minPair.first);
		Cluster* b = clusters.at(minPair.second).get(); // DONOT USE SHARTED PTR!
		if (a.get() == b)
		{
			distances.updateDistances(numClusters, minPair.first, minPair.second);
			continue;
		}

		a->merge(*b);

		for (size_t i = 0; i < b->points.size(); i++)
		{
			clusters.at(b->points[i]) = a;
		}

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

	assert(uniqeIds.size() == maxAllowed);
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
				matrix.setBoth(j, i, iterLandDistanceSumInf(landscapes[i], landscapes[j], 0.1f));
			}
		}

		for (size_t i = 0; i < n; i++)
		{
			matrix(i, i) = 0;
		}

		output = hierarchicalClustering(matrix, maxAllowed);
	}
};
