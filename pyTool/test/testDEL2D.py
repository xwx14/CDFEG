# SPDX-License-Identifier: GPL-3.0
# This file is part of CDFEG.
#
# CDFEG is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CDFEG is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.
# 2D弹性力学场（平面应力）
import sys
import json
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from MakerCpp import MakerCpp
from MakerGidFile import MakerGidFile

project = DataProject("del2d", 2)
field = DataField("DelDisp")
field.pdeType = 3  # 3双曲型
field.bDynamic=True
s = 1 / (3**0.5)

# Q4高斯积分四边形单元
ele1 = DataEleSubG("DelQ4g", 4)
ele1.type = 2
ele1.dispNames = ["u", "v"]
ele1.eleResNames = ["sigmaXX", "sigmaYY", "sigmaXY", "volume"]
ele1.paramNames = ["pe", "pv", "fu", "fv", "rou", "alpha"]
ele1.paramValues=[1.0e10, 0.3, 0.0, 0.0, 3000.0, 0.6]
ele1.gaussPoints = [
    [s, s],
    [s, -s],
    [-s, s],
    [-s, -s]
]
ele1.gaussWeights = [1.0, 1.0, 1.0, 1.0]
ele1.shapeFuns = [
    "(1. - x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. + x[2]) / 2.",
    "(1. - x[1]) / 2. * (1. + x[2]) / 2."
]
field.addEleSub(ele1)

project.addField(field)

outPath = "sample/DEl2D"
maker = MakerCpp(project, outPath, mode='add', sln_cmake_path="CMakeLists.txt")
maker.mainMode=1
maker.makeAll()
gidMaker = MakerGidFile(project, outPath)
gidMaker.makeAll()
d = project.toDict()
with open(outPath + "/data.json", "w") as f:
    json.dump(d, f, indent=4)
