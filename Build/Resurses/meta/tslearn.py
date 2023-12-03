import tslearn
# from tslearn.utils import to_time_series_dataset
import cpio
import sys
# https://tslearn.readthedocs.io/en/stable/variablelength.html
def kmeans(X, params):
    from tslearn.clustering import KernelKMeans
    gak_km = KernelKMeans(n_clusters=params["n_clusters"], kernel="gak")
    return gak_km.fit_predict(X)

def tskmeans_dtw(X, params):
    from tslearn.clustering import TimeSeriesKMeans
    km = TimeSeriesKMeans(n_clusters=params["n_clusters"], metric = "dtw") # "dtw", "softdtw"
    return km.fit_predict(X)

def tskmeans_softdtw(X, params):
    from tslearn.clustering import TimeSeriesKMeans
    km = TimeSeriesKMeans(n_clusters=params["n_clusters"], metric ="softdtw") # "dtw", "softdtw"
    return km.fit_predict(X)

def silhouette_score(X, params):
    from tslearn.clustering import TimeSeriesKMeans, silhouette_score
    km = TimeSeriesKMeans(n_clusters=params["n_clusters"], metric = "dtw") # "dtw", "softdtw"
    labels = km.fit_predict(X)
    return silhouette_score(X, labels, metric="dtw")

methods ={
    "kmeans" : kmeans,
    "tskmeans_dtw" : tskmeans_dtw,
    "tskmeans_softdtw" : tskmeans_softdtw,
    "silhouette_score" : silhouette_score
}

def main(method, X, params):
    if method in methods:
        return methods[method](X, params)
    else:
        print(f"ERR: No method {method}")
        return None

if __name__ == "__main__":
    # Вызов основной функции
    # print("ERR: " + params)
    # print(params)
    import tslearn.utils.utils
    method, X, params = cpio.parseArgs(sys.argv)
    labels = main(method,  tslearn.utils.utils.to_time_series_dataset(X), params)
    cpio.writeResult(labels)
