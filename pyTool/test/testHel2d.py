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
# 2D热弹性力学场
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

project = DataProject("hel2d", 2)
# 第一个场为温度场
field1 = DataField("Heat")
hele1 = DataEleSubG("HeatQ4g", 4)
hele1.type = 2
hele1.matName = "HelQ4g"
hele1.dispNames = ["T"]
hele1.paramNames = ["ek", "ec", "q"]
hele1.paramValues = [1.0, 1.0, 0.0]
s = 1 / (3**0.5)
hele1.gaussPoints = [
    [s, s], 
    [s, -s],
    [-s, s],
    [-s, -s]]
hele1.gaussWeights = [1.0, 1.0, 1.0, 1.0]
hele1.shapeFuns = [
    "(1. - x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. + x[2]) / 2.",
    "(1. - x[1]) / 2. * (1. + x[2]) / 2."
]
field1.addEleSub(hele1)
project.addField(field1)
# 第二个场为位移场
field2 = DataField("DelDisp")
dele1=DataEleSubG("DelQ4g", 4)
dele1.matName = "HelQ4g"
dele1.type = 2
dele1.dispNames = ["u", "v"]
dele1.eleResNames = ["sigmaXX", "sigmaYY", "sigmaXY", "volume"]
dele1.paramNames = ["pe", "pv", "fu", "fv", "rou", "alpha", "alfa"]
dele1.paramValues = [1.0e10, 0.3, 0.0, 0.0, 3000.0, 0.6, 0.6]
dele1.gaussPoints = [
    [s, s],
    [s, -s],
    [-s, s],
    [-s, -s]
]
dele1.gaussWeights = [1.0, 1.0, 1.0, 1.0]
dele1.shapeFuns = [
    "(1. - x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. - x[2]) / 2.",
    "(1. + x[1]) / 2. * (1. + x[2]) / 2.",
    "(1. - x[1]) / 2. * (1. + x[2]) / 2."
]
field2.addEleSub(dele1)
field2.bNodeExtrap=True
field2.nodeExtrapNames=["sigmaXX","sigmaYY","sigmaXY"]
project.addField(field2)
project.cmds.append(("imp",0))
project.cmds.append(("imp",1))
# GiD 后处理输出项（与 sample/Hel2D/main.cpp 一致：temperature@场0，disp/stress@场1）
project.addOutputItem("temperature", "Scalar", "OnNodes", [(0, "T")])
project.addOutputItem("disp", "Vector", "OnNodes", [(1, "u"), (1, "v")])
project.addOutputItem("stress", "Matrix", "OnNodes", [(1, "sigmaXX"), (1, "sigmaYY"), (1, "sigmaXY")])
project.addOutputItem("eleStress", "Matrix", "OnElements", [(1, "sigmaXX"), (1, "sigmaYY"), (1, "sigmaXY")])

outPath="sample\\Hel2D"
maker = MakerCpp(project, outPath)
maker.mainMode=1
maker.makeAll()
gidMaker = MakerGidFile(project, outPath)
gidMaker.makeAll()
d = project.toDict()
with open(outPath + "/data.json", "w") as f:
    json.dump(d, f, indent=4)