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

# macs 项目通用生成入口：python test/testMacs.py <项目名> [macs根目录]
import sys
import os
import json

HERE = os.path.dirname(os.path.abspath(__file__))
PYTOOL = os.path.dirname(HERE)                 # pyTool/
PROJ_ROOT = os.path.dirname(PYTOOL)            # 项目根 CDFEG/
sys.path.append(PYTOOL)

from preParser import parsePre, parseGcn
from MakerCpp import MakerCpp
from MakerGidFile import MakerGidFile


def main():
    if len(sys.argv) < 2:
        print("用法: python test/testMacs.py <项目名> [macs根目录]")
        sys.exit(1)
    projName = sys.argv[1]
    macsRoot = sys.argv[2] if len(sys.argv) > 2 else r"E:/mfelProject/RegTest/testData/macs"
    # 解析 pre/ges/gcn → DataProject
    project = parsePre(macsRoot, projName)
    parseGcn(macsRoot, projName, project)
    # 落地项目根 macs/<Proj>，new 模式（自带库副本）
    outPath = os.path.join(PROJ_ROOT, "macs", projName.capitalize())
    os.makedirs(outPath, exist_ok=True)
    # MakerCpp new 模式：复制 CDFEG+third，生成解决方案级 CMake，
    # 项目代码落入 outPath/<project.name>/
    maker = MakerCpp(project, outPath, mode="new")
    maker.mainMode = 1     # GiD 入口
    maker.makeAll()
    # GiD 文件（.bas/.prb/.cnd/.bat）落入 outPath/<project.name>.gid/
    MakerGidFile(project, outPath).makeAll()
    # 写出 data.json（项目数据快照）
    with open(os.path.join(outPath, "data.json"), "w", encoding="utf-8") as f:
        json.dump(project.toDict(), f, indent=4, ensure_ascii=False)
    print("生成完成:", outPath)


if __name__ == "__main__":
    main()
