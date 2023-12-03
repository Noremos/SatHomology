import matplotlib.pyplot as plt
import numpy as np


def read_array_from_file(file_path):
    collection = []
    try:
        with open(file_path, 'r') as file:
            # Чтение строки из файла и разделение чисел по пробелам
            for line in file:
                if (len(line) > 100):
                    continue
                if (len(line) == 2):
                    continue

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
    # return np.array(collection)
    return collection


# Assuming 'collection' is a list of NumPy arrays
# collection = read_array_from_file(r'd:\Education\BAR\input_with_middle.txt')
collection = read_array_from_file(r'd:\Education\BAR\input_without_middle.txt')

# Assuming 'collection' is a list of NumPy arrays
def displayArrays():
    plt.figure()
    # Plot each array in 'collection'
    for arr in collection:
        plt.plot(arr)

    # Add labels and title
    plt.xlabel('X-axis label')
    plt.ylabel('Y-axis label')
    plt.title('Multiple Arrays in Collection')

    # Show the plot
    plt.show()

def displayMeta():
    # plt.figure()
    # Plot each array in 'collection' and display statistics in separate figures
    # for i, arr in enumerate(collection):
    #     # Create a new figure for each array
    #     plt.plot(arr)
    #     size = len(arr)
        # mean = np.mean(arr)
        # std_dev = np.std(arr)
        # min_val = np.min(arr)
        # max_val = np.max(arr)

        # Create a histogram of sizes
    fig, ax = plt.subplots()
    sizes = [len(arr) for arr in collection]
    ax.hist(sizes, bins=range(min(sizes), max(sizes) + 1), align='left', alpha=0.7, edgecolor='black')

    # Add labels and title for the entire plot
    ax.set_xlabel('Size of Arrays')
    ax.set_ylabel('Count')
    ax.set_title('Size Distribution of Arrays in Collection')
    # Add labels and title
    # plt.xlabel('X-axis label')
    # plt.ylabel('Y-axis label')
    # plt.title(f'Array {i+1} with Statistics')

    # Show all the plots
    plt.show()

# displayMeta()
displayArrays()
