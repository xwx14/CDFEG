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

# 解析 macs 项目的 gcn（求解命令流），填充 DataProject.caculateCode
import os


class GcnParser:
    """解析 .gcn 文件，填充 DataProject.caculateCode。"""

    def __init__(self, projDir, projName, project):
        self.projDir = projDir
        self.projName = projName
        self.project = project
        self._path = os.path.join(projDir, projName, projName + ".gcn")
        self._keysOrder = []

    def parse(self):
        """执行解析，填充 project.caculateCode。"""
        if not os.path.exists(self._path):
            return
        with open(self._path, "r", encoding="utf-8") as f:
            self._lines = [ln.rstrip("\n").strip() for ln in f.readlines()]
        self._parseDefi()
        self._renderCode()

    def _parseDefi(self):
        """defi 段：场标识+求解器+依赖顺序。"""
        inDefi = False
        for ln in self._lines:
            if ln == "defi":
                inDefi = True
                continue
            if inDefi:
                if ln == "" or ln.startswith("START") or ln.startswith("SOLV") or ln.startswith("gidres"):
                    break
                toks = ln.split()
                if len(toks) >= 2:
                    self._keysOrder.append(toks[0])

    def _renderCode(self):
        """gcn key → project.fields 映射，渲染 initMatrix/eProgram/solve/uPhy 调用。"""
        fieldList = self.project.fields
        keyToField = {}
        for idx, k in enumerate(self._keysOrder):
            if idx < len(fieldList):
                keyToField[k] = fieldList[idx]
        linesOut = []
        for k in self._keysOrder:
            fld = keyToField.get(k)
            if not fld:
                continue
            var = fld.fieldDataClassName[0].lower() + fld.fieldDataClassName[1:]
            linesOut.append(f"        {var}->initMatrix();")
            linesOut.append(f"        {var}->eProgram();")
            linesOut.append(f"        {var}->solve();")
            linesOut.append(f"        {var}->uPhy();")
        self.project.caculateCode = "\n".join(linesOut) + "\n"


def parseGcn(projDir, projName, project):
    """兼容旧调用方式的便捷函数。"""
    GcnParser(projDir, projName, project).parse()
