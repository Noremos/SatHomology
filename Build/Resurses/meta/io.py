
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

def writeResult(labels):
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

def parseArgs(argv):
    data_file = argv[1]
    method = argv[2]
    jsonAsString = argv[3]

    # Загрузка данных из файла
    X = read_array_from_file(data_file)
    try:
        jsonAsString = jsonAsString[1:-1]
        params = json.loads(jsonAsString)
    except Exception as e:
        print(f"ERROR: Str: {jsonAsString}\n {e}")
        return

    return method, X, params
