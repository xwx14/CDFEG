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

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from MakerCpp import MakerCpp
# 总体数据
project = DataProject("Truss1D",1)
# 物理场
field = DataField("Truss1DDisp")
# 单元子程序
ele = DataEleSub("Truss1D")
ele.dispNames=["u"]
ele.eleResNames=["T"]
ele.paramNames=["E","A"]
# 添加单元
field.addEleSub(ele)
# 添加场
project.addField(field)
# 求解步骤
project.cmds.append(("imp",0))
# 生成器
outPath="truss1D"
maker=MakerCpp(project,outPath)
maker.makeAll()