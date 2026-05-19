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
# 总体数据
project = DataProject("Elastic2D",2)
field=DataField("Elastic2DDisp")
ele1=DataEleSubG("ElT3",3)
ele1.dispNames=["u","v"]
ele1.eleResNames=["Sxx","Syy","Sxy"]
ele1.paramNames=["E","nu","fx","fy"]
ele1.gaussPoints=[
    [1/6, 1/6],
    [2/3, 1/6],
    [1/6, 2/3]
]
ele1.gaussWeights=[1/6, 1/6, 1/6]
ele1.shapeFuns=[
    "1. - x[1] - x[2]",
    "x[1]",
    "x[2]"
]
field.addEleSub(ele1)
ele2=DataEleSubG("ElQ4",4)
ele2.dispNames=["u","v"]
ele2.eleResNames=["Sxx","Syy","Sxy"]
ele2.paramNames=["E","nu","fx","fy"]
s=1/(3**0.5)
ele2.gaussPoints=[
    [-s, -s],
    [ s, -s],
    [ s,  s],
    [-s,  s]
]
ele2.gaussWeights=[1.0, 1.0, 1.0, 1.0]
ele2.shapeFuns=[
    "(1. - x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. + x[2]) / 2.",
    "(1. - x[1]) / 2. * (1. + x[2]) / 2."
]
field.addEleSub(ele2)
project.addField(field)
# 生成器
outPath="sample/El2D"
maker = MakerCpp(project, outPath, mode='add', sln_cmake_path="CMakeLists.txt")
maker.makeAll()
gidMaker=MakerGidFile(project,outPath+"/gid")
gidMaker.makeAll()
d=project.toDict()
with open(outPath+"/data.json","w") as f:
    json.dump(d,f,indent=4)