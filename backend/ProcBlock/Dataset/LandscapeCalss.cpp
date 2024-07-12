#include "LandscapeCalss.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <memory>
#include <set>
#include "hclust-cpp/fastcluster.h"
#include "../../Clusterizers/ExteranlReader.h"
#include "Barcode/PrjBarlib/modules/StateBinFile.h"


struct Point {
    double x, y;

    Point(double x, double y) : x(x), y(y) {}
};

inline double distance(const Point& a, const Point& b) {
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
		double& value = data[i * cols + j];
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


	void updateDistances(int source, int cooked)
	{
		double maxVal = get(source, cooked);
		for (size_t i = 0; i < rows; i++)
		{
			setBoth(i, cooked,  std::max(get(i, cooked), maxVal));
		}

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
		assert(rows > 0);
		assert(data.size() > 0);
		using namespace std;
		for (int i = 0; i < rows; ++i)
		{
			for (int j = i + 1; j < cols; ++j)
			{
				double val = get(i, j);
				if (val < 0.0) // skip -1.0
					continue;

				// cout << val <<  " vs " << minDistance << endl;
				if (val < minDistance)
				{
					minDistance = val;
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

inline std::vector<int> hierarchicalClustering(const MatrixXd& distanceMatrix, int maxAllowed)
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
		// std::cout << "Объединяются кластеры " << minPair.first << " и " << minPair.second << std::endl;

		auto& a = clusters.at(minPair.first);
		Cluster* b = clusters.at(minPair.second).get(); // DONOT USE SHARTED PTR!
		if (a.get() == b)
		{
			distances.setBoth(minPair.first, minPair.second, -1);
			continue;
		}

		assert(a->points.size());
		assert(b->points.size());
		a->merge(*b);

		auto points = std::move(b->points);
		double maxVal = distances.get(minPair.first, minPair.second);
		for (size_t i = 0, total = points.size(); i < total; i++)
		{
			// distances.updateDistances(points[i], minPair.second);
			clusters.at(points[i]) = a;
		}

		distances.setBoth(minPair.first, minPair.second, -1);
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

using FuncPtr = double(const Landscape&, const Landscape&, float, int);
void LandscapeCluster::predict(std::vector<Landscape>& landscapes)
{
	std::vector<FuncPtr*> fucns
	{
		iterDistance<Distance::Iter::Inf>,
		iterDistance<Distance::Iter::InfSum>,
		iterDistance<Distance::Iter::Norm2>,
		iterDistance<Distance::Iter::SumNorm2>
	};

	int n = landscapes.size();

	MatrixXd matrix(n, n);


	FuncPtr& func = *fucns[curFUnc];

	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = i + 1; j < n; j++)
		{
			// iterLandDistanceSumInf // 14
			// iterLandDistanceSum2 // 17
			// iterLandDistanceInf // 11
			// iterLandDistance2 // 4
			// iterLandDistanceMdpiInf // 5
			// iterLandDistanceMdpi2 // 18
			matrix.setBoth(i, j, func(landscapes[i], landscapes[j], 0.5f, -1));
			// matrix.setBoth(j, i, lineDistance<Distance::Line::AreaInf>(landscapes[i], landscapes[j]));
		}
		matrix(i, i) = 0;
	}


	BackString filePath = get_temp_file_path();
	StateBinFile::BinStateWriter writer;
	filePath.replace(filePath.size() - 3, 3, "bin");
	writer.open(filePath);
	writer.pInt(n);
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			writer.pFloat(matrix.get(i, j));
		}
	}
	writer.close();


	std::vector<BackString> execCmd;
	execCmd.push_back(getPythonExe());
	execCmd.push_back((Variables::metaPath / "cluster_distance_matrix.py").string());
	execCmd.push_back(filePath);
	execCmd.push_back(methodName);
	execCmd.push_back(to_string(maxAllowed));

	int dummyN = n;
	exec(execCmd, output, dummyN, false);



	// int opt_method = HCLUST_METHOD_COMPLETE; //HCLUST_METHOD_SINGLE  HCLUST_METHOD_COMPLETE  HCLUST_METHOD_AVERAGE HCLUST_METHOD_MEDIAN

	// int* merge = new int[2*(n-1)];
	// double* height = new double[n-1];
	// auto temp = matrix.data;
	// hclust_fast(n, temp.data(), opt_method, merge, height);

	// output.resize(n);
	// cutree_k(n, merge, maxAllowed, output.data());

	// delete[] height;
	// delete[] merge;

	// output = hierarchicalClustering(matrix, maxAllowed);
}
