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

# FEPG（pre/ges/gcn）文件解析统一编排：组织三种文件的读流程
from .preParser import parsePre
from .gcnParser import parseGcn
from .gesParser import parseGes


def parseProject(projDir, projName):
    """解析 macs 项目的 pre/gcn/ges，按 pre→gcn→ges 顺序编排，返回完整 DataProject。

    1. parsePre：读 .pre 构造项目骨架（dim/场/单元/材料参数），不触发 ges；
    2. parseGcn：读 .gcn 渲染 caculateCode（求解命令流）；
    3. parseGes：逐单元读 .ges，填充积分点/形函数/结果名/VTK 类型。
    """
    project = parsePre(projDir, projName)
    parseGcn(projDir, projName, project)
    for fld in project.fields:
        for e in fld.eleSubs:
            e.project = fld              # 给单元挂 field 引用（parseGes 依赖）
            parseGes(projDir, e.name, e)
            e.inferVTKCellType()
    return project
