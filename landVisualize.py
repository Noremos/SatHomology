import struct
from typing import BinaryIO, Optional, Callable, List

class BinStateReader:
	def __init__(self, stream: Optional[BinaryIO] = None):
		self.main_stream = open(stream, 'rb') if isinstance(stream, str) else stream
		self.stream = self.main_stream
		self.inside = stream is None
		self.parse_barscalar = None  # Callable[[BinaryIO], Barscalar]
		self.memoffs = []
		self.items_end_pos = 0

		if self.stream:
			self.inti_on_open()

	def read_raw(self, fmt: str):
		size = struct.calcsize(fmt)
		data = self.stream.read(size)
		return struct.unpack(fmt, data)[0]

	def inti_on_open(self):
		self.items_end_pos = self.read_raw('Q')  # Assuming size_t is 8 bytes
		cur_pos = self.stream.tell()

		self.stream.seek(self.items_end_pos)

		size = self.read_raw('I')  # Assuming uint is 4 bytes
		self.memoffs = [self.read_raw('Q') for _ in range(size)]

		self.stream.seek(cur_pos)

	def open(self, path: str) -> bool:
		if not self.inside:
			raise Exception("Stream already initialized externally")

		self.main_stream = open(path, 'rb')
		self.stream = self.main_stream

		if self.main_stream:
			self.inti_on_open()
			return True
		else:
			return False

	def move_index(self, index: int):
		assert index < len(self.memoffs)
		self.stream.seek(self.memoffs[index])

	def get_index_size(self):
		return len(self.memoffs)

	def ended(self) -> bool:
		return self.stream.tell() >= self.items_end_pos or self.stream.peek() == b''

	def close(self) -> None:
		if not self.inside and self.main_stream is not None:
			self.main_stream.close()

	def p_bool(self):
		return self.read_raw('B') > 0

	def p_short(self):
		return self.read_raw('h')

	def p_int(self):
		return self.read_raw('i')

	def p_float(self):
		return self.read_raw('f')

	def p_int64(self):
		return self.read_raw('Q')

	def p_barscalar(self):
		return self.parse_barscalar(self.stream)

	def p_string(self):
		length = self.read_raw('H')  # Assuming ushort is 2 bytes
		return str(self.stream.read(length))

	def p_array(self):
		return self.p_int()



class Point:
	def __init__(self):
		self.x = 0
		self.y = 0

class Landscape:
	def __init__(self):
		self.hist = []
		self.lines = []
		self.correctId = -1
		self.name = ""

	def readPoints(self, out : BinStateReader):
		lineCount = out.p_array()
		for _ in range(lineCount):
			line = []
			size = out.p_array()
			for _ in range(size):
				point = Point()
				point.x = out.p_int()
				point.y = out.p_int()
				line.append(point)

			self.lines.append(line)

	def readHeis(self, out : BinStateReader):
		setSize = out.p_array()
		for _ in range(setSize):
			self.hist.append(out.p_float())

	def get_signature_as_vector(self):
		AddE = 1

		total = [0] * int(256 * AddE)  # Приведение типа к int для размера списка
		N = len(self.hist)

		for landset in self.lines:
			for j in range(1, len(landset)):
				prev = landset[j - 1]
				cur = landset[j]
				startY = prev.y

				startX = round(prev.x * AddE)
				endX = round(cur.x * AddE)
				assert startX <= endX

				width = endX - startX
				height = abs(cur.y - prev.y)
				if height == 0 or width == 0:
					total[startX] += round(startY)
					continue
				iter = height / width

				y = startY
				assert iter > 0
				assert endX < len(total)
				for i in range(startX, endX):
					total[i] += round(y)
					y += iter

		x_coords = list(range(len(total)))
		heights = total
		return x_coords, heights

	def getCombined(self):
		combined = {}

		for line in self.lines:
			for point in line:

				if point.x in combined:
					combined[point.x] += point.y
				else:
					combined[point.x] = point.y


		combined = sorted(combined.items())
		x_coords = [p[0] for p in combined]
		heights = [p[1] for p in combined]

		return x_coords, heights

	def getHei(self):
		x_coords = list(range(len(self.hist)))
		heights = self.hist
		return x_coords, heights

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
	plt.figure(figsize=(10, 6))

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



def drawAll(lands : List[Landscape], source, name = None):
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

		y = i + np.random.rand(100)*0.25
		colors.append([r,g,b])

	# Создание графика
	plt.figure(figsize=(10, 6))

	# for point in land.hist:
	# 	plt.scatter(point.x, point.y, color=colors(pointsColor), label=name)

	# x_coords = list(range(len(land.hist)))
	# heights = land.hist

	figugres = {}
	ignored = []
	hasName = {}
	mainFig = plt.figure()
	combined = mainFig.add_subplot(111)
	combined.set_xlabel('X-axis')
	combined.set_ylabel('Y-axis')
	# combined.legend()
	combined.grid(True)


	# separateFig = plt.figure()
	# separateFig, axs = plt.subplots(3, 3, figsize=(15, 10))
	# for j in range(7, 9):
	# 	separateFig.delaxes(axs[j // 3, j % 3])

	separateFig, axs = plt.subplots(1, 2, figsize=(15, 10))


	for land in lands:
		if land.correctId in ignored:
			continue

		if source == UseSignature:
			x_coords, heights = land.get_signature_as_vector()
		elif source == UseHei: #same as signature
			x_coords, heights = land.getHei()
		# elif source == UsePoints:
		# 	x_coords, heights = land.getPoints()
		else:
			x_coords, heights = land.getCombined()
		# print(heights[3])

		#draw each correctId on separete graph

		if not land.correctId in figugres:
			# f = plt.figure()
			# ax = f.add_subplot(111)
			row = land.correctId // 3
			col = land.correctId % 3
			ax = axs[col]
			figugres[land.correctId] = (separateFig, ax)

			ax.set_title(str(land.correctId))
			ax.set_xlabel('X-axis')
			ax.set_ylabel('Y-axis')
			# ax.legend()
			ax.grid(True)

		if not land.correctId in hasName:
			hasName[land.correctId] = land.name
			combined.plot(x_coords, heights, linestyle='-', color=colors[land.correctId], label=str(land.correctId))
		else:
			combined.plot(x_coords, heights, linestyle='-', color=colors[land.correctId])

		figugres[land.correctId][1].plot(x_coords, heights, linestyle='-', color=colors[land.correctId])



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
		mainFig.savefig('combined_' + name)
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

landsPerClass, landsAll = loadLandscape("f0t255.bin")
# drawAll(landsAll, UseSignature)
drawAll(landsAll, UseSignature)

# drawForOne(landsAll[0])
