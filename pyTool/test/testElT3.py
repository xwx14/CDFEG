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
# 2D弹性力学场.三角形单元（平面应力）
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
project = DataProject("ElasticT3",2)
field=DataField("Elastic2DDisp")
ele1=DataEleSub("ElT3",3)
ele1.type=2
ele1.dispNames=["u","v"]
ele1.eleResNames=["Sxx","Syy","Sxy"]
ele1.paramNames=["E","nu","t","fx","fy"]
field.addEleSub(ele1)
project.addField(field)
# 生成器
outPath="sample/ElT3"
maker = MakerCpp(project, outPath, mode='add', sln_cmake_path="CMakeLists.txt")
maker.mainMode=1
maker.makeAll()
gidMaker=MakerGidFile(project,outPath+"/gid")
gidMaker.makeAll()
d=project.toDict()
with open(outPath+"/data.json","w") as f:
    json.dump(d,f,indent=4)