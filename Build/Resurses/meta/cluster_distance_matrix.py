
from BinState import *

import numpy as np
from scipy.cluster.hierarchy import linkage, fcluster
from sklearn.cluster import SpectralClustering, KMeans, DBSCAN, AffinityPropagation
from sklearn.manifold import MDS
import sys
import cpio


def correctIds(clusters):
	for i in range(len(clusters)):
		clusters[i] -= 1
	return clusters

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
		return clusters
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

	return correctIds(clusters)

if __name__ == "__main__":
	# Генерируем случайную матрицу расстояний (Для примера)
	np.random.seed(0)

	filename = sys.argv[1]
	method = sys.argv[2]
	num_clusters = int(sys.argv[3])


	reader = BinStateReader()
	reader.open(filename)
	n = reader.p_int()
	data = []
	for i in range(n):
		data.append([])
		for j in range(n):
			data[i].append(reader.p_float())

	distance_matrix = data

	import warnings
	warnings.filterwarnings("ignore")

	clusters = cluster_data(distance_matrix, method, num_clusters)

	print("0", end=" ") # dummy
	for i in range(len(clusters)):
		print(clusters[i], end=" ")
	print()
