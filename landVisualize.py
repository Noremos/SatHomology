import struct
from typing import BinaryIO, Optional, Callable, List

from BinState import *

def loadLandscape(name):
	reader = BinStateReader()
	reader.open(name)
	resolution = reader.p_int()
	clustersCount = reader.p_short()
	a = reader.p_bool()

	landsPerClass = {}
	landsAll = []
	while a:
		land = Landscape()
		# land.name = reader.p_string()
		land.correctId = reader.p_int()
		land.readPoints(reader)
		# land.readHeis(reader)

		landId = len(landsAll)
		landsAll.append(land)

		if land.correctId in landsPerClass:
			landsPerClass[land.correctId].append(landId)
		else:
			landsPerClass[land.correctId] = [landId]

		a = reader.p_bool()
		# break

	return landsPerClass, landsAll


import matplotlib.pyplot as plt
import numpy as np
import matplotlib
matplotlib.use('Qt5Agg')

# num_objects = len(landsPerClass)

# enum with Combiend ,vector, hei
UseCombined = 1
UseSignature = 2
UseHei = 3
UsePoints = 4

def drawForOne(land : Landscape, source):
	# Количество объектов

	# Генерация случайных точек для каждого объекта
	np.random.seed(0)  # Для повторяемости результатов
	points_per_object = 10

	# Уникальные цвета для каждого объекта
	colors = []
	for i in range(11):
		r = np.round(np.random.rand(),1)
		g = np.round(np.random.rand(),1)
		b = np.round(np.random.rand(),1)

		colors.append([r,g,b])

	pointsColor = len(colors) - 1

	# Создание графика
	# for point in land.hist:
	# 	plt.scatter(point.x, point.y, color=colors(pointsColor), label=name)



	total = None
	if source == UseSignature:
		land.get_signature_as_vector()
	elif source == UseHei:
		total = land.hist
	else:
		total = land.getCombined()

	x_coords = list(range(len(total)))
	heights = total

	plt.plot(x_coords, heights, marker='o', linestyle='-', color=colors[pointsColor], label="Inter")

	combined = {}

	for line in land.lines:
		for point in line:
			if point.x in combined:
				combined[point.x] += point.y
			else:
				combined[point.x] = point.y

	# sort combined by keys
	combined = sorted(combined.items())
	x_coords = [p[0] for p in combined]
	heights = [p[1] for p in combined]
	plt.plot(x_coords, heights, marker='o', linestyle='-', color=colors[0], label="Combined")


	# for i, line in enumerate(land.lines):
	# 	# x_coords = [p.x for p in line]
	# 	# heights =  [p.y for p in line]
	# 	# plt.plot(x_coords, heights, marker='o', linestyle='-', color=colors(i))
	# 	for p in line:
	# 		if p.x < 150:
	# 			print(f"{p.x} {p.y}")
	# 			plt.scatter(p.x, p.y, color=colors[2])


	# Настройка графика
	plt.title('Multiple Points of Several Objects')
	plt.xlabel('X-axis')
	plt.ylabel('Y-axis')
	plt.legend()
	plt.grid(True)
	plt.show()



def drawAll(lands : List[Landscape], name = None):
	# Количество объектов

	# Генерация случайных точек для каждого объекта
	np.random.seed(0)  # Для повторяемости результатов
	points_per_object = 10

	# Уникальные цвета для каждого объекта
	colors = []
	for i in range(30):
		r = np.round(np.random.rand(),1)
		g = np.round(np.random.rand(),1)
		b = np.round(np.random.rand(),1)

		# y = i + np.random.rand(100)*0.25
		colors.append([r,g,b])

	# # Создание графика
	# plt.figure(figsize=(10, 6))

	# for point in land.hist:
	# 	plt.scatter(point.x, point.y, color=colors(pointsColor), label=name)

	# x_coords = list(range(len(land.hist)))
	# heights = land.hist

	figugres = {}
	ignored = []
	# hasName = {}
	# mainFig = plt.figure()
	# combined = mainFig.add_subplot(111)
	# combined.set_xlabel('X-axis')
	# combined.set_ylabel('Y-axis')
	# # combined.legend()
	# combined.grid(True)


	# separateFig = plt.figure()
	# separateFig, axs = plt.subplots(3, 3, figsize=(15, 10))
	# for j in range(7, 9):
	# 	separateFig.delaxes(axs[j // 3, j % 3])

	# separateFig, axs = plt.subplots(1, 2, figsize=(15, 10))
	separateFig, axs = plt.subplots(2, 5, figsize=(20, 10))


	usedCOls = [0,0]
	for land in lands:
		if land.correctId in ignored:
			continue



		# if not land.correctId in figugres:
			# f = plt.figure()
			# ax = f.add_subplot(111)
		row = land.correctId
		col = usedCOls[row]
		usedCOls[row] += 1

		ax = axs[row][col]
		# figugres[land.correctId] = (separateFig, ax)

		ax.set_title(str(land.correctId))
		ax.set_xlabel('X-axis')
		ax.set_ylabel('Y-axis')
		plt.axis('equal')
		# ax.legend()
		ax.grid(True)

		k = 0

		for line in land.lines:
			x_coords = [p.x for p in line]
			heights = [p.y for p in line]


			wid = 5 - k
			if wid < 0:
				break
			ax.plot(x_coords, heights, linestyle='-', color=colors[k % len(colors)], linewidth= wid)
			k += 1

		# print(heights[3])

		#draw each correctId on separete graph


				# if not land.correctId in hasName:
				# 	hasName[land.correctId] = land.name
				# 	combined.plot(x_coords, heights, linestyle='-', color=colors[land.correctId], label=str(land.correctId))
				# else:
				# 	combined.plot(x_coords, heights, linestyle='-', color=colors[land.correctId])

				# figugres[land.correctId][1].plot(x_coords, heights, linestyle='-', color=colors[land.correctId])



	# for land in lands:
	# 	x_coords, heights = land.getCombined()
	# 	plt.plot(x_coords, heights, marker='o', linestyle='-', color=colors[land.correctId], label="Inter")

	# for land in lands:
	# 	x_coords = list(range(len(land.hist)))
	# 	heights = land.hist
	# 	plt.plot(x_coords, heights, marker='o', linestyle='-', color=colors[land.correctId], label="Inter")

	# Настройка графика
	# plt.title('Multiple Points of Several Objects')
	# plt.xlabel('X-axis')
	# plt.ylabel('Y-axis')
	# plt.legend()
	# plt.grid(True)
	if name:
		# mainFig.savefig('combined_' + name)
		separateFig.savefig('separated_' + name)
		plt.close()
	else:
		plt.show()

# landsPerClass, landsAll = loadLandscape("f0t255.bin")
# drawAll(landsAll)

# landsPerClass, landsAll = loadLandscape("f255t0.bin")
# drawAll(landsAll, UseCombined, "f255t0_combined.png")
# drawAll(landsAll, UseSignature, "f255t0_signature.png")
# drawAll(landsAll, UseHei, "f255t0_hei.png")

# landsPerClass, landsAll = loadLandscape("f0t255.bin")
# drawAll(landsAll, UseCombined, "f0t255_combined.png")
# drawAll(landsAll, UseSignature, "f0t255_signature.png")
# drawAll(landsAll, UseHei, "f0t255_hei.png")


linesd= [
(0, 95),
(0, 95),
(9, 104),
(101, 110),
(0, 118),
(0, 132),
(95, 132),
(0, 132),
(126, 132),
(126, 139),
(0, 141),
(100, 141),
(125, 141),
]

def drawSingle(land : Landscape, name = None):
	# Количество объектов
	np.random.seed(0)  # Для повторяемости результатов0
	# Уникальные цвета для каждого объекта
	colors = []
	for i in range(30):
		r = np.round(np.random.rand(),1)
		g = np.round(np.random.rand(),1)
		b = np.round(np.random.rand(),1)

	# y = i + np.random.rand(100)*0.25
		colors.append([r,g,b])

	separateFig, (ax, ax2) = plt.subplots(1, 2, figsize=(10, 5))


	plt.axis('equal')
	# ax.legend()
	ax.grid(True)
	ax2.grid(True)

	ax.set_xlabel('Яркость')
	ax.set_ylabel('Высота')

	ax2.set_xlabel('Яркость')
	ax2.set_ylabel('Номер компоненты')
	# Start y from 0
	# ax.set_yticks([0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100])

	ax2.set_yticklabels([])
	# set x legend


	k = 0

	k2 = 1

	for line in linesd:
		x_coords = [line[0], line[1]]
		heights = [k2, k2]
		ax2.plot(x_coords, heights, linestyle='-', color=colors[k2 % len(colors)], linewidth= 2)
		k2 += 5

	for line in land.lines:
		if k == 0:
			k += 1
			continue
		x_coords = [p.x for p in line]
		heights = [p.y for p in line]

		if k == 7:
			x_coords[0] -= 1
			x_coords[1] -= 0.5
			heights[1] += 0.5

		wid = 5 - k
		if wid < 0:
			wid = 1
		wid = 1
		ax.plot(x_coords, heights, linestyle='-', color=colors[k % len(colors)], linewidth= wid)
		k += 1

	# save figure with 300 dpi

	plt.savefig("out.jpg", dpi=300)
	plt.show()

landsPerClass, landsAll = loadLandscape("out.bin")
drawSingle(landsAll[0])
# drawForOne(landsAll[0], UseHei)

# drawForOne(landsAll[0])
