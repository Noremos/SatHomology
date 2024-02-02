import numpy as np
import matplotlib.pyplot as plt
import numpy as np
import cpio
import sys

# def persistence_landscape(lines):
#     # Sort lines by their first coordinates
#     lines = sorted(lines, key=lambda x: x[0])

#     # Initialize persistence landscape
#     pl = np.zeros(len(lines))

#     # Initialize current birth time
#     current_birth_time = 0

#     # Iterate over lines
#     for i, line in enumerate(lines):
#         # If the current line has a smaller first coordinate than the previous line, it is a new feature
#         if line[0] < lines[i - 1][0]:
#             # Update current birth time
#             current_birth_time = line[0]

#         # Calculate persistence
#         persistence = line[1] - current_birth_time

#         # Update persistence landscape
#         pl[i] = persistence

#     return pl

# # Example usage
# lines = [[0, 40], [5, 10], [30, 50]]
# pl = persistence_landscape(lines)
# print(pl)


# def compare_persistence_landscapes(pl1, pl2):
#     # Calculate the L2 norm between the two persistence landscapes
#     l2_norm = np.linalg.norm(pl1 - pl2)

#     # Calculate the correlation coefficient between the two persistence landscapes
#     correlation_coefficient = np.corrcoef(pl1, pl2)[0, 1]

#     # Return the L2 norm and the correlation coefficient
#     return l2_norm, correlation_coefficient

# # Example usage
# pl1 = [40., 10., 50.]
# pl2 = [30., 20., 60.]

# l2_norm, correlation_coefficient = compare_persistence_landscapes(pl1, pl2)
# print("L2 norm:", l2_norm)
# print("Correlation coefficient:", correlation_coefficient)


# import numpy as np

def persistence_landscape_wasserstein_distance(pl1, pl2):
    # Calculate the Wasserstein distance between the two persistence landscapes
    wasserstein_distance = np.inf

    # Initialize current Wasserstein distance
    current_wasserstein_distance = 0

    # Iterate over the two persistence landscapes
    end= min(len(pl1), len(pl2))
    for i in range(end):
        # Calculate the difference in persistence
        persistence_difference = abs(pl1[i] - pl2[i])

        # Update the current Wasserstein distance
        current_wasserstein_distance += persistence_difference

        # Update the Wasserstein distance if necessary
        # wasserstein_distance = min(wasserstein_distance, current_wasserstein_distance)

    if len(pl1) < len(pl2):
        for i in range(end, len(pl2)):
            current_wasserstein_distance += abs(pl2[i])
    elif len(pl1) > len(pl2):
        for i in range(end, len(pl1)):
            current_wasserstein_distance += abs(pl1[i])

    # Return the Wasserstein distance
    return current_wasserstein_distance

# # Example usage
# pl1 = [40., 10., 50.]
# pl2 = [30., 20., 60.]

# wasserstein_distance = persistence_landscape_wasserstein_distance(pl1, pl2)
# print("Wasserstein distance:", wasserstein_distance)



import numpy as np

def persistence_landscape_lp_dise(pl1, pl2, p = 2):
    # Calculate the Lp norm between the two persistence landscapes
    lp_norm = 0

    # Iterate over the two persistence landscapes
    end= min(len(pl1), len(pl2))
    for i in range(end):
        # Calculate the difference in persistence
        persistence_difference = abs(pl1[i] - pl2[i])

        # Update the Lp norm
        lp_norm += persistence_difference ** p

    if len(pl1) < len(pl2):
        for i in range(end, len(pl2)):
            lp_norm += abs(pl2[i]) ** p
    elif len(pl1) > len(pl2):
        for i in range(end, len(pl1)):
            lp_norm += abs(pl1[i]) ** p

    lp_norm = np.power(lp_norm, 1 / p)

    # if len(pl1) < len(pl2):
    #     for i in range(end, len(pl2)):
    #         lp_norm += abs(pl2[i])
    # elif len(pl1) > len(pl2):
    #     for i in range(end, len(pl1)):
    #         lp_norm += abs(pl1[i])

    return lp_norm
    # Return the Lp norm

def wagner_fischer(s1, s2):
    len_s1, len_s2 = len(s1), len(s2)
    if len_s1 > len_s2:
        s1, s2 = s2, s1
        len_s1, len_s2 = len_s2, len_s1

    current_row = range(len_s1 + 1)
    for i in range(1, len_s2 + 1):
        previous_row, current_row = current_row, [i] + [0] * len_s1
        for j in range(1, len_s1 + 1):
            add, delete, change = previous_row[j] + 1, current_row[j-1] + 1, previous_row[j-1]
            if s1[j-1] != s2[i-1]:
                change += 1
            current_row[j] = min(add, delete, change)

    return current_row[len_s1]


def persistence_landscape_lp_distance(pl1, pl2, p = 2):
    a = persistence_landscape_lp_dise(pl1, pl2, p)
    plc1 = pl1.copy()
    plc2 = pl2.copy()
    plc1 = plc1[::-1]
    plc2 = plc2[::-1]
    b = persistence_landscape_lp_dise(plc1, plc2, p)
    # return (a + b) / 2
    # return np.sqrt(a * a + b * b)
    return min(a, b)
    # return max(a, b)

# # Example usage
# pl1 = [5., 10., 15.]
# # pl1 = [5., 10.,]
# # pl1 = [5., 10.,]
# print(persistence_landscape_lp_distance(pl1, [5., 10.,]))
# print(persistence_landscape_lp_distance(pl1, [10.,15.]))
# print(persistence_landscape_lp_distance(pl1, [5., 5.,15.]))
# print(persistence_landscape_lp_distance(pl1, [5.,15.]))
# print(persistence_landscape_lp_distance(pl1, [4.,9.]))
# print(persistence_landscape_lp_distance(pl1, [9.,16.]))
# print(persistence_landscape_lp_distance(pl1, [4.,11.,14]))
# print(persistence_landscape_lp_distance(pl1, [4.,11.,14, 20]))
# lp_norm = persistence_landscape_lp_distance(pl1, pl2, 2)
# print("Lp norm:", lp_norm)

from sklearn.cluster import KMeans
import sklearn

def persistence_landscape_kmeans_clustering(pl_list, k):
    # Calculate the Wasserstein distance between all persistence landscapes
    # wasserstein_distance_matrix = np.zeros((len(pl_list), len(pl_list)))
    # for i in range(len(pl_list)):
    #     for j in range(i + 1, len(pl_list)):
    #         wasserstein_distance_matrix[i, j] = persistence_landscape_lp_distance(pl_list[i], pl_list[j])
    #         wasserstein_distance_matrix[j, i] = wasserstein_distance_matrix[i, j]

    # # Perform k-means clustering on the Wasserstein distance matrix
    # kmeans = KMeans(n_clusters=k, random_state=0)
    # kmeans.fit(wasserstein_distance_matrix)
    reduced_dataset = sklearn.cluster.DBSCAN(metric=persistence_landscape_lp_distance).fit(pl_list)

    # Return the labels of the clusters
    return reduced_dataset.labels_

if __name__ == "__main__":
    # Вызов основной функции
    # print("ERR: " + params)
    # print(params)
    method, X, params = cpio.parseArgs(sys.argv)
    labels = persistence_landscape_kmeans_clustering(X, params["n_clusters"])
    cpio.writeResult(labels)
