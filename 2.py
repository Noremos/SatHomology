# Путь к либе
import sys
import numpy as np

def getValue(baseLayer, coorxX, coordY):
    rectangle = baseLayer.extent()

    pixelSizeX = baseLayer.rasterUnitsPerPixelX()
    pixelSizeY = baseLayer.rasterUnitsPerPixelY()

    return QgsPointXY(rectangle.xMinimum() +coorxX * pixelSizeX, rectangle.yMaximum() - coordY * pixelSizeY)


def createPoly(baseLayer, points, polyname = 'poly'):
    pass


from typing import Dict, List, Tuple
from enum import Enum
from collections import deque

class StartPos(Enum):
    LeftMid = 0
    LeftTop = 1
    TopMid = 2
    RightTop = 3
    RightMid = 4
    RightBottom = 5
    BottomMid = 6
    LeftBottom = 7


class XY:
    def __init__(self, x, y) -> None:
        self.x = x
        self.y = y

class MapCountur:
    def __init__(self) -> None:
        self.x = 0
        self.y = 0
        self.stIndex = 0
        self.contur = []
        self.points: Dict[int, bool] = {}
        self.dirct = StartPos.RightMid
        self.dirs: deque[StartPos] = deque()
        self.pointsStack: deque[int] = deque()

    def run(self, aproxim: bool = False) -> None:
        poss = [(-1, 0), (-1, -1), (0, -1), (1, -1), (1, 0), (1, 1), (0, 1), (-1, 1),
                (-1, 0), (-1, -1), (0, -1), (1, -1), (1, 0), (1, 1), (0, 1), (-1, 1)]
        self.dirct = StartPos.RightMid
        while True:
            start = (self.dirct.value + 6) % 8
            end = start + 5
            prevS = (self.x, self.y)
            for i in range(start, end):
                off = poss[i]
                if self.try_val(off[0], off[1]):
                    break
                start += 1

            if start != end:
                s = self.get_index()
                old = self.dirct
                self.dirs.append(old)
                self.dirct = StartPos(i % 8)
                if self.dirct != old or not aproxim:
                    self.contur.append(prevS)
                if s == self.stIndex:
                    break
            else:
                self.unset()
                if len(self.pointsStack) < 1:
                    break
                old = self.dirct
                self.dirct = self.dirs.pop()
                if self.dirct != old or not aproxim:
                    self.contur.pop()
                p = self.stGetStatPoint(self.pointsStack.pop())
                self.x = p[0]
                self.y = p[1]

    def set(self, p) -> None:
        self.points[self.stGetStatInd(p.x, p.y)] = True

    def set_start(self, x: int, y: int) -> None:
        self.x = x
        self.y = y
        self.stIndex = self.stGetStatInd(x, y)

    def unset(self) -> None:
        self.points[self.stGetStatInd(self.x, self.y)] = False

    def exists(self, xl: int, yl: int) -> bool:
        return self.stGetStatInd(xl, yl) in self.points

    def try_val(self, oX: int, oY: int) -> bool:
        if self.exists(self.x + oX, self.y + oY):
            self.pointsStack.append(self.get_index())
            self.x += oX
            self.y += oY
            return True
        return False

    def get_index(self) -> int:
        return self.stGetStatInd(self.x, self.y)

    def stGetStatInd(self, x, y) -> int:
        return int(y * 65535 + x)

    def stGetStatPoint(self, index):
        return (int(index % 65535), int(index / 65535))


def getCountour(points, aproximate: bool):
    rect = [99999999, 99999999, 0, 0]
    stY = None
    dictPoints = MapCountur()

    for p in points:
        dictPoints.set(p)
        x, y = p.x, p.y
        if x < rect[0]:
            rect[0] = x
            stY = y
        if x > rect[2]:
            rect[2] = x
        if y < rect[1]:
            rect[1] = y
        if y > rect[3]:
            rect[3] = y

    wid = rect[2] - rect[0]
    hei = rect[3] - rect[1]

    if wid < 3 or hei < 3:
        return dictPoints.contur

    dictPoints.set_start(rect[0], stY)
    dictPoints.run(aproximate)
    return dictPoints.contur
