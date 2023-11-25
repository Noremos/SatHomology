# import argparse
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.cluster import KMeans, DBSCAN, OPTICS

import json
import numpy as np
import sys
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

def apply_spectral_biclustering(X, n_clusters):
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

def read_array_from_file(file_path):
    collection = []
    try:
        with open(file_path, 'r') as file:
            # Чтение строки из файла и разделение чисел по пробелам
            for line in file:
                array_str = line.strip()
               # Преобразование строковых чисел в фактические числа
                collection.append([float(num) for num in array_str.split()])
    except FileNotFoundError:
        print(f"ERROR: Файл '{file_path}' не найден.")
        return None
    except Exception as e:
        print(f"ERROR: Произошла ошибка при чтении файла: {e}")
        return None

    if len(collection[-1]) == 0:
        collection.pop()
    return np.array(collection)

def main(method, jsonAsString, data_file):
    # Загрузка данных из файла
    X = read_array_from_file(data_file)
    try:
        jsonAsString = jsonAsString[1:-1]
        params = json.loads(jsonAsString)
    except Exception as e:
        print(f"ERROR: Str: {jsonAsString}\n {e}")
        return;
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
        labels = apply_mean_shift(X)
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


    # Вывод предсказанных меток в stdout
    # print("Predicted Labels:")
    n_clusters_ = len(set(labels)) - (1 if -1 in labels else 0)
    # np.set_printoptions(threshold=sys.maxsize)
    # print(str(n_clusters_) + ' ')
    import tempfile
    import os
    fileName = os.path.join(tempfile.gettempdir(), "out_array.txt")
    # Open the file for writing.
    with open(fileName, mode='w') as f:
        f.write(str(n_clusters_) + ' ')
        for l in labels:
            f.write(' ' + str(l))

    print(fileName)

if __name__ == "__main__":
    # Вызов основной функции
    dataFile = sys.argv[1]
    method = sys.argv[2]
    params = sys.argv[3]
    # print("ERR: " + params)
    # print(params)
    main(method, params, dataFile)