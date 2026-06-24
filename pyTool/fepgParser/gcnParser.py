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


def parseGcn(projDir, projName, project):
    """解析 <projName>.gcn → project.caculateCode。
    gcn 格式（el）：
      defi
      a ell              ← 场名 a（对应 _name），求解器 ell
      b str a            ← 场名 b，求解器 str，依赖 a
      START a
      SOLVc a            ← 标准椭圆
      SOLVstr b a        ← 最小二乘（b 依赖 a）
      gidres(coor0);
    渲染为按声明顺序的各场调用（场名取 gcn 的 a/b，映射到 project.fields 的 _name）。
    """
    path = os.path.join(projDir, projName, projName + ".gcn")
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n").strip() for ln in f.readlines()]
    # defi 段：场标识+求解器+依赖（b str a → b 依赖 a）
    keysOrder = []
    inDefi = False
    for ln in lines:
        if ln == "defi":
            inDefi = True
            continue
        if inDefi:
            if ln == "" or ln.startswith("START") or ln.startswith("SOLV") or ln.startswith("gidres"):
                break
            toks = ln.split()
            if len(toks) >= 2:
                keysOrder.append(toks[0])
    # gcn 的 a/b 映射到 project.fields：按声明顺序对应
    fieldList = project.fields
    keyToField = {}
    for idx, k in enumerate(keysOrder):
        if idx < len(fieldList):
            keyToField[k] = fieldList[idx]
    # 渲染 caculateCode：每场 initMatrix→eProgram→solve→uPhy
    linesOut = []
    for k in keysOrder:
        fld = keyToField.get(k)
        if not fld:
            continue
        var = fld.fieldDataClassName[0].lower() + fld.fieldDataClassName[1:]  # 小驼峰实例名
        linesOut.append(f"        {var}->initMatrix();")
        linesOut.append(f"        {var}->eProgram();")
        linesOut.append(f"        {var}->solve();")
        linesOut.append(f"        {var}->uPhy();")
    project.caculateCode = "\n".join(linesOut) + "\n"
