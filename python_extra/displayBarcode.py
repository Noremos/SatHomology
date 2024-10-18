import struct
from typing import BinaryIO, Optional, Callable, List

from BinState import *


def loadBarcode(name):
	reader = BinStateReader()
	reader.open(name)

	lines = []
	size = reader.p_int()
	for _ in range(size):
		lines.append((reader.p_int(),  reader.p_int()))

	return lines


import matplotlib.pyplot as plt
import numpy as np
import matplotlib
# matplotlib.use('Qt5Agg'


def drawSingle(lines, name):
	# Количество объектов
	np.random.seed(0)  # Для повторяемости результатов
	# Уникальные цвета для каждого объекта
	colors = []
	for i in range(30):
		r = np.round(np.random.rand(),1)
		g = np.round(np.random.rand(),1)
		b = np.round(np.random.rand(),1)

	# y = i + np.random.rand(100)*0.25
		colors.append([r,g,b])

	separateFig, ax = plt.subplots(1, 1)
	# ax.ylim((0,255))

	# ax.axis('equal')
	# ax.legend()

	ax.grid(True)
	# ax.set_label('Brightness')
	# ax.set_ylabel('Number of component')

	ax.set_xlabel('Яркость')
	ax.set_ylabel('Номер компоненты')

	# Start y from 0
	# ax.set_yticks([0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100])

	# ax2.set_yticklabels([])
	# set x legend

	k2 = 1

	for line in lines:
		x_coords = [line[0], line[1]]
		heights = [k2, k2]
		ax.plot(x_coords, heights, linestyle='-', color=colors[0], linewidth= 1)
		k2 += 1


	plt.savefig(f"{name}.png", dpi=300)
	plt.savefig(f"{name}.svg")
	plt.show()



def main():
	lines = loadBarcode("f0t255.bin")
	drawSingle(lines, "2_f0t255_ru")
	lines = loadBarcode("f255t0.bin")
	drawSingle(lines, "2_f255t0_ru")

main()
