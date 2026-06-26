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


class PreParser:
    """解析 .pre 文件，构造 DataProject（dim/场/单元骨架/材料参数）。"""

    def __init__(self, projDir, projName):
        self.projDir = projDir
        self.projName = projName
        self._path = os.path.join(projDir, projName, projName + ".pre")
        self._lines = []
        self._mode = "field"
        self._cur_field = None
        self._fields = {}
        self._field_order = []

    def parse(self):
        """执行解析，返回 DataProject。"""
        if not os.path.exists(self._path):
            raise FileNotFoundError(f"pre 文件不存在：{self._path}")
        with open(self._path, "r", encoding="utf-8") as f:
            self._lines = [ln.rstrip("\n") for ln in f.readlines()]
        project = self._parseHead()
        self._parseBody()
        self._attachFields(project)
        return project

    # ---- 各段解析 ----

    def _parseHead(self):
        """第1行提取 dim，构造 DataProject。"""
        dim = 2
        if self._lines:
            head = self._lines[0].split()
            if head and head[0].startswith("2"):
                dim = 2
            elif head and head[0].startswith("3"):
                dim = 3
        return DataProject(self.projName, dim)

    def _parseBody(self):
        """从第2行开始状态机解析 field/element/matedata。"""
        i = 1 if self._lines else 0
        while i < len(self._lines):
            ln = self._lines[i].strip()
            i += 1
            if not ln:
                continue
            toks = ln.split()
            if ln == "#":
                self._cur_field = None
                continue
            if toks[0] == "element":
                self._mode = "element"
                continue
            if toks[0] == "matedata":
                self._mode = "matedata"
                continue
            if self._mode == "field":
                i = self._parseFieldLine(toks, i)
            elif self._mode == "element":
                i = self._parseElementLine(toks, i)
            elif self._mode == "matedata":
                i = self._parseMatedataLine(toks, i)

    def _parseFieldLine(self, toks, i):
        """<场名> 0 <dof> <?> <dispNames...>"""
        fname = toks[0]
        dispNames = toks[4:]
        fld = DataField(fname)
        self._fields[fname] = fld
        self._field_order.append(fname)
        fld._preDispNames = dispNames
        self._cur_field = fld
        return i

    def _parseElementLine(self, toks, i):
        """<场名> 切换当前场；<单元名> <nNode> <paramNames...> 添加单元。"""
        if self._cur_field is None or toks[0] in self._fields:
            if toks[0] in self._fields:
                self._cur_field = self._fields[toks[0]]
            return i
        eleName = toks[0]
        nNode = int(toks[1])
        paramNames = toks[2:]
        ele = DataEleSubG(eleName, nNode)
        ele.paramNames = paramNames
        ele.dispNames = list(self._cur_field._preDispNames)
        self._cur_field.addEleSub(ele)
        return i

    def _parseMatedataLine(self, toks, i):
        """<场名> <单元名>，下一非空行为数值。"""
        if len(toks) >= 2 and toks[0] in self._fields and toks[1]:
            fname, eleName = toks[0], toks[1]
            vals = []
            while i < len(self._lines):
                nl = self._lines[i].strip()
                i += 1
                if nl == "" or nl == "#":
                    break
                try:
                    vals = [float(x) for x in nl.split()]
                    break
                except ValueError:
                    break
            fld = self._fields[fname]
            for e in fld.eleSubs:
                if e.name == eleName:
                    e.paramValues = vals
                    break
        return i

    def _attachFields(self, project):
        """将解析到的场按顺序挂到 project。"""
        for fname in self._field_order:
            fld = self._fields[fname]
            fld.project = project
            project.addField(fld)


def parsePre(projDir, projName):
    """兼容旧调用方式的便捷函数。"""
    return PreParser(projDir, projName).parse()
