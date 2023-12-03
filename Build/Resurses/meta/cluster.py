# import argparse
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.cluster import KMeans, DBSCAN, OPTICS

import numpy as np
import sys

import cpio
# import argparse
# import pandas as pd
from sklearn.cluster import (
    AffinityPropagation,
    AgglomerativeClustering,
    Birch,
    MiniBatchKMeans,
    MeanShift,
    SpectralClustering,
    SpectralBiclustering,
    SpectralCoclustering,
    # feature_agglomeration,
    DBSCAN,
    FeatureAgglomeration,
    BisectingKMeans,
)

def classify_OPTICS(X, params):
    clustering  = OPTICS(min_samples=params["min_samples"], max_eps = params["eps"]).fit(X)
    return clustering.labels_ #.predict(X)

def classify_kmeans(X, params):
    kmeans = KMeans(n_clusters=params["n_clusters"], n_init='auto')
    kmeans.fit(X)
    return kmeans.labels_ #.predict(X)

def apply_affinity_propagation(X):
    clustering = AffinityPropagation().fit(X)
    return clustering.labels_

def apply_agglomerative_clustering(X, params): # (X, n_clusters):
    clustering = AgglomerativeClustering(n_clusters=params["n_clusters"]).fit(X)
    return clustering.labels_

def apply_birch(X, params): #(X, threshold=0.5, n_clusters=None):
    clustering = Birch(threshold=params["threshold"], n_clusters=params["n_clusters"]).fit(X)
    return clustering.labels_

def apply_mini_batch_kmeans(X, params): #(X, n_clusters):
    # clustering = MiniBatchKMeans(n_clusters=n_clusters).fit(X)
    clustering = MiniBatchKMeans(n_clusters=params["n_clusters"]).fit(X)
    return clustering.labels_

def apply_mean_shift(X, params): #(X):
    clustering = MeanShift().fit(X)
    return clustering.labels_

def apply_spectral_clustering(X, params): #(X, n_clusters):
    clustering = SpectralClustering(n_clusters=params["n_clusters"]).fit(X)
    return clustering.labels_

def apply_spectral_biclustering(X, params):
    clustering = SpectralBiclustering(n_clusters=params["n_clusters"]).fit(X)
    return clustering.row_labels_

def apply_spectral_coclustering(X, params): #(X, n_clusters):
    clustering = SpectralCoclustering(n_clusters=params["n_clusters"]).fit(X)
    return clustering.row_labels_

def apply_dbscan(X, params): #(X, eps, min_samples):
    # clustering = DBSCAN(eps=eps, min_samples=min_samples).fit(X)
    clustering = DBSCAN(eps=params["eps"], min_samples=params["min_samples"]).fit(X)
    return clustering.labels_

def apply_feature_agglomeration(X, params): #(X, n_clusters):
    clustering = FeatureAgglomeration(n_clusters=params["n_clusters"]).fit(X)
    return clustering.labels_

def apply_bisecting_kmeans(X, params): #(X, n_clusters):
    clustering = BisectingKMeans(n_clusters=params["n_clusters"]).fit(X)
    return clustering.labels_

def main(method, X, params):
    # Инициализация и обучение модели
    if method == 'AffinityPropagation':
        labels = apply_affinity_propagation(X)
    elif method == 'AgglomerativeClustering':
        labels = apply_agglomerative_clustering(X, params)
    elif method == 'Birch':
        labels = apply_birch(X, params)
    elif method == 'MiniBatchKMeans':
        labels = apply_mini_batch_kmeans(X, params)
    elif method == 'MeanShift':
        labels = apply_mean_shift(X, params)
    elif method == 'SpectralClustering':
        labels = apply_spectral_clustering(X, params)
    elif method == 'SpectralBiclustering':
        labels = apply_spectral_biclustering(X, params)
    elif method == 'SpectralCoclustering':
        labels = apply_spectral_coclustering(X, params)
    elif method == 'DBSCAN':
        labels = apply_dbscan(X, params)
    elif method == 'FeatureAgglomeration':
        labels = apply_feature_agglomeration(X, params)
    elif method == 'BisectingKMeans':
        labels = apply_bisecting_kmeans(X, params)
    elif method == 'OPTICS':
        labels = classify_OPTICS(X, params)
    elif method == 'KMeans':
        labels = classify_kmeans(X, params)
    else:
        print(f"ERROR: Метод классификации '{method}' не поддерживается.")
        return

    return labels

if __name__ == "__main__":
    # Вызов основной функции
    # print("ERR: " + params)
    # print(params)
    method, X, params = cpio.parseArgs(sys.argv)
    labels = main(method, np.array(X), params)
    cpio.writeResult(labels)
