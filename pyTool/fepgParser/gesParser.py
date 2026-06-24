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

# 解析 macs 项目的 ges（单元几何/积分/形函数/结果名），填充 DataEleSubG
import os


def _inferType(nNode, dim, coordVars):
    """由节点数+维度推断单元几何类型：0点/1线/2面/3体。"""
    if dim == 1:
        return 1
    if dim == 2:
        return 2 if nNode >= 3 else 1     # >=3节点为面，否则线
    if dim == 3:
        return 3 if nNode >= 5 else 2
    return 1


def parseGes(projDir, eleName, ele):
    """解析 <eleName>.ges -> 填充 gaussPoints/Weights/shapeFuns/eleResNames/type。
    ges 关键段：
      node N          -> 校验 nNodes
      refc rx,ry,     -> coordVars（参考坐标名）
      coor x,y,       -> 维度
      gaus = N        + 下 N 行: rx ry weight（2D）/ rx weight（1D）
      shap 段         -> 形函数表达式（取 disp 的第一个分量的 N 个表达式）
      func = a,b,c    -> eleResNames
      coef u,v        -> 记录到 ele._gesCoefVars（依赖其他场位移）
    """
    path = os.path.join(projDir, ele.project.project.name, eleName + ".ges")
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f.readlines()]
    gaus_n = 0
    gaus_lines = []
    shap_exprs = []
    in_shap = False
    ges_coef_vars = []
    # 收集 nrefc（参考坐标维度）
    nrefc = 0
    for idx, raw in enumerate(lines):
        s = raw.strip()
        if s == "":
            continue
        # refc rx,ry, -> 推断参考坐标维度
        if s.startswith("refc"):
            toks_refc = s.replace(",", " ").split()
            # refc 后面是坐标名列表，过滤 "refc" 本身
            nrefc = len([t for t in toks_refc[1:] if t])
            continue
        # gaus = N 或 gaus  N
        if s.startswith("gaus"):
            toks = s.replace("=", " ").split()
            toks = [t for t in toks if t]  # 过滤空串（"gaus = 4"产生空串）
            if len(toks) >= 2:
                gaus_n = int(toks[1])
            # 收集后续 gaus_n 个数值行
            cnt = 0
            j = idx + 1
            while j < len(lines) and cnt < gaus_n:
                lj = lines[j].strip()
                j += 1
                if lj == "":
                    continue
                try:
                    vals = [float(x) for x in lj.split()]
                    gaus_lines.append(vals)
                    cnt += 1
                except ValueError:
                    break
            continue
        # shap 段开始
        if s.startswith("shap"):
            in_shap = True
            continue
        if in_shap:
            # 分量标题行（u= / v= / dxx= / dxy= 等）：以字母开头、以=结尾、无空格
            if s.endswith("=") and "=" not in s[:-1]:
                continue
            # 表达式行（u1 = (...)）：含=但不以=结尾
            if "=" in s and not s.endswith("="):
                expr = s.split("=", 1)[1].strip()
                if expr:
                    shap_exprs.append(expr)
                continue
            # 遇到其他 ges 段关键字结束 shap 收集
            if any(s.startswith(k) for k in
                   ("tran", "func", "dist", "mass", "load", "end",
                    "coef", "refc", "coor", "node", "mate", "gaus")):
                in_shap = False
            continue
        # func = exx,eyy,exy, （defi 头部声明）
        if s.startswith("func") and "=" in s:
            rhs = s.split("=", 1)[1]
            ele.eleResNames = [x.strip() for x in rhs.split(",") if x.strip()]
            continue
        # coef u,v, （defi 头部声明，无=号）
        if s.startswith("coef") and not s.startswith("coef="):
            rhs = s[len("coef"):].strip()
            if rhs:
                # "u,v," -> 过滤空串
                ges_coef_vars = [x.strip() for x in rhs.replace(",", " ").split()
                                 if x.strip()]
            continue
    # 填充积分点（每行前 nrefc 个为坐标，末个为权重）
    ele.gaussPoints = []
    ele.gaussWeights = []
    if gaus_lines:
        actual_nrefc = len(gaus_lines[0]) - 1 if len(gaus_lines[0]) > 1 else 1
        if nrefc == 0:
            nrefc = actual_nrefc  # 若 refc 未解析到，用积分点行推断
        for gl in gaus_lines:
            ele.gaussPoints.append(gl[:nrefc])
            ele.gaussWeights.append(gl[-1])
    # 形函数：rx->x[1], ry->x[2] 占位符（pyTool _replaceCoordVars 会替换）
    # 只取与节点数相等的前 nNode 个表达式
    def _conv(expr):
        return expr.replace("rx", "x[1]").replace("ry", "x[2]")
    ele.shapeFuns = [_conv(e) for e in shap_exprs[:ele.nNodes]]
    # type 推断：含 coef 先假设线单元，再按 nNode>=3 修正回面单元
    if ges_coef_vars:
        ele._gesCoefVars = ges_coef_vars
    if ges_coef_vars:
        ele.type = 1  # 含 coef 的依赖单元多为线单元（beq4g2 修正回 2）
    inferred = _inferType(ele.nNodes, ele.project.project.dim, ele.coordVars)
    if ele.type == 1 and ele.nNodes >= 3 and ele.project.project.dim == 2:
        ele.type = 2  # beq4g2（4节点面单元含 coef）修正回面
    elif not ges_coef_vars:
        ele.type = inferred
    if ele.type == 1:
        ele.bBC = True
    # coordVars
    if nrefc == 1:
        ele.coordVars = ["x"]
    elif nrefc >= 2:
        ele.coordVars = ["x", "y"]
