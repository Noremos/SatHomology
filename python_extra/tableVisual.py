import matplotlib.pyplot as plt

import matplotlib
matplotlib.use('Qt5Agg')


# Данные для таблиц 1 и 2 (объединенные)
data_1_2 = {
    'Метод': ['spectral', 'dbscan', 'affinity', 'hierarchical', 'mds_kmeans', 'kmedoids'],
    'Точность': [60, 50, 85, 97.5, 97.5, 95],
}

# Данные для таблицы 3 (нормализация)
data_3 = {
    'Метод': ['hierarchical', 'mds_kmeans', 'kmedoids'],
    'Исходная точность': [55, 57.5, 55],
    'Точность после нормализации': [65, 62.5, 55],
    'Процент улучшения': [10, 5, 0]
}

# Данные для таблицы 4 (сравнение с relen)
data_4 = {
    'Метод': ['hierarchical', 'mds_kmeans'],
    'Точность без relen': [76, 74],
    'Точность с relen': [73, 75]
}

# Данные для таблицы 5 (сравнение классов)
data_5 = {
    'Метод': ['hierarchical', 'mds_kmeans'],
    'Точность для 3 классов': [58.6667, 69],
    'Точность для 4 классов': [44, 51]
}

# Создание графиков

# График 1: Сравнение методов по точности
plt.figure(figsize=(10, 6))
plt.bar(data_1_2['Метод'], data_1_2['Точность'], color='skyblue')
plt.xlabel('Метод')
plt.ylabel('Точность (%)')
plt.title('Сравнение методов по точности')
plt.show()

# График 2: Улучшение точности после нормализации
plt.figure(figsize=(10, 6))
plt.bar(data_3['Метод'], data_3['Процент улучшения'], color='lightgreen')
plt.xlabel('Метод')
plt.ylabel('Процент улучшения (%)')
plt.title('Улучшение точности после нормализации')
plt.show()

# График 3: Влияние relen на точность
plt.figure(figsize=(10, 6))
bar_width = 0.35
index = range(len(data_4['Метод']))

plt.bar(index, data_4['Точность без relen'], bar_width, label='Без relen')
plt.bar([i + bar_width for i in index], data_4['Точность с relen'], bar_width, label='С relen', color='orange')

plt.xlabel('Метод')
plt.ylabel('Точность (%)')
plt.title('Влияние параметра relen на точность')
plt.xticks([i + bar_width / 2 for i in index], data_4['Метод'])
plt.legend()
plt.show()

# График 4: Сравнение результатов для 3 и 4 классов
plt.figure(figsize=(10, 6))
plt.plot(data_5['Метод'], data_5['Точность для 3 классов'], marker='o', label='3 класса')
plt.plot(data_5['Метод'], data_5['Точность для 4 классов'], marker='o', label='4 класса', color='red')

plt.xlabel('Метод')
plt.ylabel('Точность (%)')
plt.title('Сравнение точности для 3 и 4 классов')
plt.legend()
plt.show()
