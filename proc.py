
from BinState import *

import numpy as np
from scipy.cluster.hierarchy import linkage, fcluster
from sklearn.cluster import SpectralClustering, KMeans, DBSCAN, AffinityPropagation
from sklearn.manifold import MDS


def cluster_data(distance_matrix, method='hierarchical', num_clusters=2):
	if method == 'hierarchical':
		# Иерархическая кластеризация (агломеративная)
		Z = linkage(distance_matrix, 'ward')
		clusters = fcluster(Z, num_clusters, criterion='maxclust')
	elif method == 'spectral':
		# Спектральная кластеризация
		sc = SpectralClustering(n_clusters=num_clusters, affinity='precomputed')
		clusters = sc.fit_predict(distance_matrix)
	elif method == 'mds_kmeans':
		# MDS + KMeans
		mds = MDS(n_components=2, dissimilarity='precomputed', random_state=0)
		X_transformed = mds.fit_transform(distance_matrix)
		kmeans = KMeans(n_clusters=num_clusters, random_state=0)
		clusters = kmeans.fit_predict(X_transformed)
	elif method == 'kmedoids':
		# K-medoids (используем библиотеку scikit-learn-extra)
		from sklearn_extra.cluster import KMedoids
		kmedoids = KMedoids(n_clusters=num_clusters, metric='precomputed', random_state=0)
		clusters = kmedoids.fit_predict(distance_matrix)
	elif method == 'dbscan':
		# DBSCAN
		dbscan = DBSCAN(metric='precomputed')
		clusters = dbscan.fit_predict(distance_matrix)
	elif method == 'affinity':
		# Affinity Propagation
		ap = AffinityPropagation(affinity='precomputed', random_state=0)
		clusters = ap.fit_predict(distance_matrix)
	else:
		raise ValueError("Unknown method: {}".format(method))

	return clusters

if __name__ == "__main__":
	# Генерируем случайную матрицу расстояний (Для примера)
	np.random.seed(0)
	reader = BinStateReader()
	reader.open("dist.bin")
	n = reader.p_int()
	data = []
	for i in range(n):
		data.append([])
		for j in range(n):
			data[i].append(reader.p_float())

	distance_matrix = data

	method = 'hierarchical'  # Замените на нужный метод
	num_clusters = 2  # Количество кластеров
	clusters = cluster_data(distance_matrix, 'hierarchical', num_clusters)
	print(clusters)
	# clusters = cluster_data(distance_matrix, 'spectral', num_clusters)
	# print(clusters)
	clusters = cluster_data(distance_matrix, 'mds_kmeans', num_clusters)
	print(clusters)
	clusters = cluster_data(distance_matrix, 'kmedoids', num_clusters)
	print(clusters)
	# clusters = cluster_data(distance_matrix, 'dbscan', num_clusters)
	# print(clusters)
	# clusters = cluster_data(distance_matrix, 'affinity', num_clusters)
	# print(clusters)
	# Визуализация результатов
	# plt.scatter(data[:, 0], data[:, 1], c=clusters, cmap='viridis', marker='o')
	# plt.title(f"Clustering result using {method}")
	# plt.xlabel('Feature 1')
	# plt.ylabel('Feature 2')
	# plt.show()