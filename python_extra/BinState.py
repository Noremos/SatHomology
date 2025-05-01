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



class BinStateWriter:
	def __init__(self, stream: Optional[BinaryIO] = None):
		self.stream = open(stream, 'wb') if isinstance(stream, str) else stream
		self.inside = stream is None

	def write_raw(self, fmt: str, value):
		data = struct.pack(fmt, value)
		self.stream.write(data)

	def open(self, path: str) -> bool:
		if not self.inside:
			raise Exception("Stream already initialized externally")

		self.stream = open(path, 'wb')
		return True

	def p_bool(self, value: bool) -> bool:
		self.write_raw('B', 1 if value else 0)
		return value

	def p_short(self, value: int) -> int:
		self.write_raw('h', value)
		return value

	def p_int(self, value: int) -> int:
		self.write_raw('i', value)
		return value

	def p_float(self, value: float) -> float:
		self.write_raw('f', value)
		return value

	def p_int64(self, value: int) -> int:
		self.write_raw('Q', value)
		return value

	def p_string(self, value: str) -> str:
		assert len(value) < 65536  # ushort max value
		self.p_short(len(value))
		self.stream.write(value.encode('utf-8'))
		return value

	def p_array(self, size: int) -> int:
		return self.p_int(size)

	def end_item(self):
		pass

	def close(self):
		if self.stream:
			cur_pos = self.stream.tell()
			self.write_raw('I', len(self.memoffs))  # Write the size of memoffs
			for offset in self.memoffs:
				self.write_raw('Q', offset)

			self.stream.seek(0)
			self.write_raw('Q', cur_pos)  # Update items_end_pos
			self.stream.close()

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
				point.x = out.p_float()
				point.y = out.p_float()
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
