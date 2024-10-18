import matplotlib.pyplot as plt

import matplotlib
matplotlib.use('Qt5Agg')


nameMap = {
	"spectral": "S",
	"dbscan": "D",
	"affinity": "A",
	"hierarchical": "H",
	"mds_kmeans": "K",
	"kmedoids": "M"
}

def getMethodName(name):
	if name in nameMap:
		return nameMap[name]
	else:
		return name

def getMethodNameShortcutList(names):
	newNames = []
	for name in names:
		newNames.append(getMethodName(name))

	return newNames

def getType(name):
	if name == "f0t255":
		return "W"
	elif name == "f255t0":
		return "B"
	else:
		return name

table1 = {
    "Функция расстояния": [
        "0", "1", "1", "*", "1|2", "*", "0", "1", "2|3", "1|2|3", "0", "1|3", "2"
    ],
    "Шаг": [
        "*", "1.0", "1.0", "*", "0.5|0.1", "*", "*", "*", "*", "*", "*", "*", "*"
    ],
    "Метод": [
        "spectral", "spectral", "spectral", "spectral", "spectral", "dbscan", "affinity", "affinity", "affinity", "spectral", "affinity", "affinity", "affinity"
    ],
    "Тип обработки": [
        "f0t255", "*", "f0t255", "f0t255", "f0t255", "*", "f0t255", "f0t255", "f0t255", "f255t0", "f255t0", "f255t0", "f255t0"
    ],
    "Кол-во правильно кластеризованных": [
        "20/40", "22/40", "22/40", "21/40", "23/40", "20/40", "17/40", "29/40", "26/40", "20/40", "14/40", "33/40", "34/40"
    ],
    "Точность": [
        50, 55, 57.5, 52.5, 52.5, 50, 42.5, 72.5, 65, 60, 35, 82.5, 85
    ]
}


table2 = {
    "Функция расстояния": [
        "0", "1|2", "3", "0", "1", "2", "3", "0", "0", "1|3", "2", "1|2|3", "0", "1|2|3", "0"
    ],
    "Шаг": [
        "*", "*", "*", "*", "*", "*", "*", "1.0", "0.5|0.1", "*", "*", "*", "*", "*", "*"
    ],
    "Метод": [
        "hierarchical", "hierarchical", "hierarchical", "mds_kmeans", "mds_kmeans", "mds_kmeans", "mds_kmeans", "kmedoids", "kmedoids", "kmedoids", "kmedoids", "hierarchical", "mds_kmeans", "mds_kmeans", "kmedoids"
    ],
    "Тип обработки": [
        "*", "*", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "*", "*", "f255t0", "f255t0", "f255t0", "f255t0"
    ],
    "Кол-во правильно кластеризованных": [
        "39/40", "36/40", "37/40", "38/40", "35/40", "37/40", "31/40", "21/40", "22/40", "37/40", "38/40", "37/40", "39/40", "37/40", "37/40"
    ],
    "Точность": [
        97.5, 90, 92.7, 95, 87.5, 92.5, 77.5, 52.5, 55, 92.5, 95, 92.5, 97.5, 92.5, 92.5
    ]
}

table3 = {
    "Функция расстояния":
        ["0", "1", "2", "3"] * 6,
    "Метод":
       ["hierarchical"] * 4 + ["mds_kmeans"] * 4 + ["kmedoids"] * 4 +  ["hierarchical"] * 4 + ["mds_kmeans"] * 4 + ["kmedoids"] * 4
    ,
    "Тип обработки":
        ["f0t255"] * 12 + ["f255t0"] * 12,
    "Результат исходный": [
        "20/40", "21/40, 52.5%", "23/40, 57.5%", "21/40, 52.5%", "23/40, 57.5%", "28/40, 70%", "25/40, 62.5%", "23/40, 57.5%", "22/40, 55%", "23/40, 57.5%", "21/40, 52.5%", "22/40, 55%", "29/40, 72.5%", "20/40, 50%", "22/40, 55%", "21/40, 52.5%", "28/40, 70%", "21/40, 52.5%", "21/40, 52.5%", "22/40, 55%", "28/40, 70%", "23/40, 57.5%", "23/40, 57.5%", "22/40, 55%", "28/40, 70%", "23/40, 57.5%", "23/40, 57.5%", "22/40, 55%", "22/40, 55%", "23/40, 57.5%", "21/40, 52.5%", "22/40, 55%"
    ],
    "Результат с нормализацией": [
        "22/40, 55%", "26/40, 65%", "26/40, 65%", "28/40, 70%", "25/40, 62.5%", "29/40, 72.5%", "24/40, 60%", "25/40, 62.5%", "22/40, 55%", "22/40, 55%", "23/40, 57.5%", "23/40, 57.5%", "25/40, 62.5%", "25/40, 62.5%", "27/40, 67.5%", "20/40, 50%", "31/40, 77.5%", "22/40, 55%", "26/40, 65%", "23/40, 57.5%", "21/40, 52.5%", "22/40, 55%", "21/40, 52.5%", "23/40, 57.5%", "21/40, 52.5%", "22/40, 55%", "22/40, 55%", "23/40, 57.5%", "21/40, 52.5%", "21/40, 52.5%", "22/40, 55%", "23/40, 57.5%"
    ],
    "Процент улучшения": [
       5,12.5,7.5,17.5,5,2.5,2.5,5,0,2.5,5,2.5,10,12.5,12.5,2.5,7.5,2.5,12.5,2.5,-17.5,-2.5,-5,2.5,
    ]
}


table4 = {
    "Функция расстояния": [
        "0", "1", "2", "3", "0", "1", "2", "3", "0", "1", "2", "3", "0", "1", "2", "3"
    ],
    "Метод": [
        "hierarchical", "hierarchical", "hierarchical", "hierarchical", "mds_kmeans", "mds_kmeans", "mds_kmeans", "mds_kmeans", "hierarchical", "hierarchical", "hierarchical", "hierarchical", "mds_kmeans", "mds_kmeans", "mds_kmeans", "mds_kmeans"
    ],
    "Тип обработки": [
        "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0"
    ],
    "Результат без relen": [
        "53/100, 53%", "68/100, 68%", "67/100, 67%", "69/100, 69%", "52/100, 52%", "70/100, 70%", "66/100, 66%", "65/100, 65%", "76/100, 76%", "63/100, 63%", "66/100, 66%", "64/100, 64%", "74/100, 74%", "56/100, 56%", "65/100, 65%", "59/100, 59%"
    ],
    "Результат с relen": [
        "50/100, 50%", "69/100, 69%", "67/100, 67%", "63/100, 63%", "53/100, 53%", "67/100, 67%", "63/100, 63%", "66/100, 66%", "73/100, 73%", "51/100, 51%", "54/100, 54%", "52/100, 52%", "75/100, 75%", "50/100, 50%", "51/100, 51%", "52/100, 52%"
    ],
    "Результат без relen %": [
        53, 68, 67, 69, 52, 70, 66, 65, 76, 63, 66, 64, 74, 56, 65, 59
    ],
    "Результат с relen %": [
        50, 69, 67, 63, 53, 67, 63, 66, 73, 51, 54, 52, 75, 50, 51, 52
    ]
}


table5 = {
    "Функция расстояния": [
        "0", "1", "2", "3", "0", "1", "2", "3", "0", "1", "2", "3", "0", "1", "2", "3"
    ],
    "Метод": [
        "hierarchical", "hierarchical", "hierarchical", "hierarchical", "mds_kmeans", "mds_kmeans", "mds_kmeans", "mds_kmeans", "hierarchical", "hierarchical", "hierarchical", "hierarchical", "mds_kmeans", "mds_kmeans", "mds_kmeans", "mds_kmeans"
    ],
    "Тип обработки": [
        "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f0t255", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0", "f255t0"
    ],
    "Результат для 3х классов": [
        "176/300, 58.6667%", "157/300, 52.3333%", "195/300, 65%", "150/300, 50%", "195/300, 65%", "162/300, 54%", "165/300, 55%", "164/300, 54.6667%", "149/300, 49.6667%", "196/300, 65.3333%", "201/300, 67%", "203/300, 67.6667%", "207/300, 69%", "190/300, 63.3333%", "205/300, 68.3333%", "187/300, 62.3333%"
    ],
    "Результат для 4х классов": [
        "176/400, 44%", "190/400, 47.5%", "204/400, 51%", "208/400, 52%", "204/400, 51%", "174/400, 43.5%", "196/400, 49%", "197/400, 49.25%", "167/400, 41.75%", "203/400, 50.75%", "220/400, 55%", "201/400, 50.25%", "180/400, 45%", "193/400, 48.25%", "232/400, 58%", "210/400, 52.5%"
    ],
    "Результат для 3х классов %": [
        58.6667, 52.3333, 65, 50, 65, 54, 55, 54.6667, 49.6667, 65.3333, 67, 67.6667, 69, 63.3333, 68.3333, 62.3333
    ],
    "Результат для 4х классов %": [
        44, 47.5, 51, 52, 51, 43.5, 49, 49.25, 41.75, 50.75, 55, 50.25, 45, 48.25, 58, 52.5
    ]
}


def getName(table, includeStep = True):
	size = len(table["Метод"])
	names = []
	for i in range(size):
		name = getMethodName(table['Метод'][i]) + ' \n' + table["Функция расстояния"][i] +'\n' + str(str(table["Шаг"][i]  + '\n') if includeStep else "") + getType(table["Тип обработки"][i])
		names.append(name)

	return names

def drawExtraLegend(x = -1, yoff1 = 0, yoff2 = 0, text= ''):
	# Draw text related to axis

    plt.text(x, 1.5 + yoff1, 'W - от белого\nB - от черного', verticalalignment='bottom', horizontalalignment='left',
        bbox={'facecolor': 'white',})
    plt.text(x, 10 + yoff2, text, verticalalignment='bottom', horizontalalignment='left',
            bbox={'facecolor': 'white',})


# График 1: Сравнение методов по точности
def drawBar(table, name, text):
	fig = plt.figure(figsize=(10, 6))
	plt.subplots_adjust(bottom=0.2)
	plt.bar(getName(table), table['Точность'], color='skyblue')
	plt.xlabel('Метод')
	plt.ylabel('Точность (%)')
	plt.title('Сравнение методов по точности')


	drawExtraLegend(-0.8, 1, 3.5, text)
	fig.savefig(name)


# График 2: Улучшение точности после нормализации
def drawProcIncr():
	fig = plt.figure(figsize=(10, 6))
	plt.subplots_adjust(bottom=0.2)
	plt.bar(getName(table3, False), table3['Процент улучшения'], color='lightgreen')
	plt.xlabel('Метод')
	plt.ylabel('Процент улучшения (%)')
	plt.title('Улучшение точности после нормализации')
	drawExtraLegend(-1.2, -19.5, -23.5, 'H - hierarchical\nK - mds_kmeans\nM - kmedoids')
	fig.savefig("fig3.png")


def drawRelenAffect():
	# График 3: Влияние relen на точность
	fig = plt.figure(figsize=(10, 6))
	plt.subplots_adjust(bottom=0.2)
	bar_width = 0.35
	index = range(len(table4['Метод']))

	plt.bar(index, table4['Результат без relen %'], bar_width, label='Без relen')
	plt.bar([i + bar_width for i in index], table4['Результат с relen %'], bar_width, label='С relen', color='orange')

	plt.xlabel('Метод')
	plt.ylabel('Точность (%)')
	plt.title('Влияние параметра relen на точность')
	plt.xticks([i + bar_width / 2 for i in index], getMethodNameShortcutList(table4['Метод']))
	plt.legend()

	plt.text(-0.7, 2.5, 'H - hierarchical\nK - mds_kmeans', verticalalignment='bottom', horizontalalignment='left',
        bbox={'facecolor': 'white',})

	fig.savefig("fig4.png")


def drawDiff():
	fig = plt.figure(figsize=(10, 6))
	plt.subplots_adjust(bottom=0.2)
	plt.bar(getName(table5, False), table5['Результат для 3х классов %'], label='3 класса')
	plt.bar(getName(table5, False), table5['Результат для 4х классов %'], label='4 класса', color='red')
	plt.xlabel('Метод')
	plt.ylabel('Точность (%)')
	plt.title('Сравнение точности для 3 и 4 классов')
	drawExtraLegend(text='H - hierarchical\nK - mds_kmeans')
	plt.legend()

	fig.savefig("Fig5.png")


def drawFinale():
	import numpy as np

	# Categories and data
	labels = ['Desert', 'Intersection', 'Railway', 'Commercial Area']
	true_positive = [99, 46, 42, 45]
	false_positive = [19, 58, 37, 54]

	x = np.arange(len(labels))  # label locations
	width = 0.35  # width of the bars

	fig, ax = plt.subplots()

	# Bars for true positives
	rects1 = ax.bar(x - width/2, true_positive, width, label='True Positive', color='skyblue')

	# Bars for false positives
	rects2 = ax.bar(x + width/2, false_positive, width, label='False Positive', color='lightcoral')

	# Add some text for labels, title and custom x-axis tick labels, etc.
	ax.set_xlabel('Классы')
	ax.set_ylabel('Точность')
	# ax.set_title('True Positive vs False Positive by Category')
	ax.set_xticks(x)
	ax.set_xticklabels(labels)
	ax.legend()

	# Adding values above bars
	def add_values(rects):
		for rect in rects:
			height = rect.get_height()
			ax.annotate('{}'.format(height),
						xy=(rect.get_x() + rect.get_width() / 2, height),
						xytext=(0, 3),  # 3 points vertical offset
						textcoords="offset points",
						ha='center', va='bottom')

	add_values(rects1)
	add_values(rects2)

	# Display the plot
	plt.tight_layout()
	fig.savefig("Fig6.png")



drawBar(table1, "fig1.png", 'S - spectral\nD - dbscan\nA - affinity')
drawBar(table2, "fig2.png", 'H - hierarchical\nK - mds_kmeans\nM - kmedoids')
drawProcIncr()
drawRelenAffect()
drawDiff()
drawFinale()
# График 4: Сравнение результатов для 3 и 4 классов


# plt.show()