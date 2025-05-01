# from Build.Resurses.meta import cluster
import cv2
import os
import numpy as np

images = []
labels = []

clusters = ["desert", "intersection", "railway", "commercial_area"]
datasets = "/Users/sam/Edu/datasets/"
srcleab = datasets + "objects/NWPU-RESISC45/"

maxImages = 100

clusterId = 0
for cluster in clusters:
	clusterImagesPath = srcleab + cluster

	loaded = 0
	for entry in os.listdir(clusterImagesPath):
		path = os.path.join(clusterImagesPath, entry)
		img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
		images.append(img)
		labels.append(clusterId)

		loaded += 1
		if loaded == maxImages:
			break


	clusterId += 1


def cl_lib():
	from clustimage import Clustimage
	X = np.array(images)
	cl = Clustimage(method='pca',
					embedding='tsne',
					grayscale=True,
					dim=(256,256),
					params_pca={'n_components':0.95},
					verbose=50)


	results = cl.fit_transform(X,
							cluster='agglomerative',
							evaluate='silhouette',
							metric='euclidean',
							linkage='ward',
							min_clust=2,
							max_clust=2,
							cluster_space='high')

	print(results)

	cl.results.keys()
	# Silhouette plots
	cl.clusteval.plot()
	cl.clusteval.scatter(X)

	# PCA explained variance plot
	cl.pca.plot()

	# Plot unique image per cluster
	cl.plot_unique(img_mean=False)

	# Scatterplot
	cl.scatter(zoom=3, img_mean=False)
	cl.scatter(zoom=None, img_mean=False)

	# Plot images per cluster or all clusters
	#cl.plot(cmap='binary', labels=[1,2])

	# Dendrogram
	cl.dendrogram()



def cl_kmeans():
	from sklearn.cluster import KMeans
	import matplotlib.pyplot as plt
	from sklearn.metrics import accuracy_score, confusion_matrix
	from scipy.stats import mode
	from sklearn.metrics import confusion_matrix
	import seaborn as sns
	import matplotlib.pyplot as plt

	# Convert to NumPy array
	prep = []
	for image in images:
		prep.append(image.flatten())

	image_data = np.array(prep)

	# Normalize pixel values (optional)
	image_data = image_data / 255.0

	# Apply K-means clustering
	num_clusters = len(clusters)  # Set the number of clusters
	kmeans = KMeans(n_clusters=num_clusters, random_state=42, n_init=10)
	predicted_clusters = kmeans.fit_predict(image_data)



	xlabels = np.array(labels)
	# Map cluster assignments to actual labels
	def map_clusters_to_labels(pred_clusters, true_labels):
		label_mapping = {}
		for i in range(num_clusters):
			mask = (pred_clusters == i)
			if np.any(mask):  # Only process if the mask is not empty
				most_common_label = mode(true_labels[mask], keepdims=True).mode[0]
				label_mapping[i] = most_common_label

		return np.array([label_mapping[c] for c in pred_clusters])

	mapped_predictions = map_clusters_to_labels(predicted_clusters, xlabels)

	# Compute accuracy
	accuracy = accuracy_score(xlabels, mapped_predictions)
	print(f"Clustering Accuracy: {accuracy * 100:.2f}%")

	# Plot confusion matrix

	# Compute confusion matrix
	conf_matrix = confusion_matrix(xlabels, mapped_predictions)

	# Plot confusion matrix
	plt.figure(figsize=(10, 8))
	sns.heatmap(conf_matrix, annot=True, fmt='d', cmap='Blues', xticklabels=clusters, yticklabels=clusters)
	plt.xlabel('Предсказанные классы ')
	plt.ylabel('Реальные метки')
	plt.title('Матрица ошибок')
	plt.show()


	# # Visualizing Clusters
	# plt.hist(predicted_clusters, bins=num_clusters, edgecolor='black')
	# plt.xlabel("Кластеризатор")
	# plt.ylabel("Количесвто изобраений")
	# plt.title("Точность кластеризации")
	# plt.show()

	# Print first 10 labels to check clustering
	print("Cluster assignments:", predicted_clusters[:10])


cl_kmeans()