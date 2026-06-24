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

# 解析 macs 项目的 pre（维度/场/单元骨架/材料参数），构造 DataProject
import os
from DataProject import DataProject
from DataField import DataField
from DataEleSubG import DataEleSubG


def parsePre(projDir, projName):
    """解析 <projName>.pre → DataProject（dim/场/单元骨架/参数）。
    pre 格式：
      第1行: 2dxy 2 3 8 10   ← 首字段 2dxy/3dxyz 给 dim
      <场名> 0 <dof> <?> <dispNames...>
      # 分隔
      element y
      <场名>
      <单元名> <nNode> <paramNames...>
      #
      matedata
      <场名> <单元名>
       <paramValues...>
      #
    注意：仅构造骨架，不解析 ges（ges 由 fepgParser.parseProject 统一编排）。
    """
    path = os.path.join(projDir, projName, projName + ".pre")
    dim = 2

    fields = {}          # name -> DataField
    field_order = []
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f.readlines()]
    # 第1行取 dim
    if lines:
        head = lines[0].split()
        if head and head[0].startswith("2"):
            dim = 2
        elif head and head[0].startswith("3"):
            dim = 3
    project = DataProject(projName, dim)
    # 状态机解析（从第2行开始，首行已处理 dim）
    mode = "field"
    cur_field = None
    cur_ele_name = None
    i = 1 if lines else 0
    while i < len(lines):
        ln = lines[i].strip()
        i += 1
        if not ln:
            continue
        toks = ln.split()
        if ln == "#":
            # # 仅重置当前场/单元上下文，不改变 mode
            cur_field = None
            cur_ele_name = None
            continue
        if toks[0] == "element":
            mode = "element"
            continue
        if toks[0] == "matedata":
            mode = "matedata"
            continue
        if mode == "field":
            # <场名> 0 <dof> <?> <dispNames...>，场名首字母非数字
            fname = toks[0]
            dispNames = toks[4:]
            fld = DataField(fname)
            fld.project = project
            fields[fname] = fld
            field_order.append(fname)
            # 暂存 dispNames（单元 addEleSub 后会从单元汇总，这里先存）
            fld._preDispNames = dispNames
            cur_field = fld
            project.addField(fld)
            continue
        if mode == "element":
            if cur_field is None or toks[0] in fields:
                # 该行是场名（切换本组场）
                if toks[0] in fields:
                    cur_field = fields[toks[0]]
                continue
            # <单元名> <nNode> <paramNames...>
            eleName = toks[0]
            nNode = int(toks[1])
            paramNames = toks[2:]
            ele = DataEleSubG(eleName, nNode)
            ele.paramNames = paramNames
            ele.dispNames = list(cur_field._preDispNames)
            cur_field.addEleSub(ele)
            continue
        if mode == "matedata":
            # <场名> <单元名>，下一非空行是数值
            if len(toks) >= 2 and toks[0] in fields and toks[1]:
                fname, eleName = toks[0], toks[1]
                # 读下一行数值
                vals = []
                while i < len(lines):
                    nl = lines[i].strip()
                    i += 1
                    if nl == "" or nl == "#":
                        break
                    try:
                        vals = [float(x) for x in nl.split()]
                        break
                    except ValueError:
                        break
                fld = fields[fname]
                for e in fld.eleSubs:
                    if e.name == eleName:
                        e.paramValues = vals
                        break
            continue
    return project
