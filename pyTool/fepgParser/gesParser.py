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


class GesParser:
    """解析 ges 文件并填充 DataEleSubG 对象。"""

    def __init__(self, projDir, eleName, ele):
        self.projDir = projDir
        self.eleName = eleName
        self.ele = ele
        self._path = os.path.join(projDir, ele.project.project.name, eleName + ".ges")
        self._lines = []
        self._gaus_n = 0
        self._gaus_lines = []
        self._shap_exprs = []
        self._in_shap = False
        self._ges_coef_vars = []
        self._nrefc = 0

    def parse(self):
        """执行解析，填充 ele 的各字段。"""
        if not os.path.exists(self._path):
            return
        with open(self._path, "r", encoding="utf-8") as f:
            self._lines = [ln.rstrip("\n") for ln in f.readlines()]
        self._parseLines()
        self._fillGauss()
        self._fillShapeFuns()
        self._fillType()
        self._fillCoordVars()
        if self._ges_coef_vars:
            self.ele._gesCoefVars = self._ges_coef_vars

    # ---- 行级解析 ----

    def _parseLines(self):
        for idx, raw in enumerate(self._lines):
            s = raw.strip()
            if s == "":
                continue
            if s.startswith("refc"):
                self._parseRefc(s)
            elif s.startswith("gaus"):
                self._parseGaus(idx)
            elif s.startswith("shap"):
                self._in_shap = True
            elif self._in_shap:
                self._parseShap(s)
            elif s.startswith("func") and "=" in s:
                self._parseFunc(s)
            elif s.startswith("coef") and not s.startswith("coef="):
                self._parseCoef(s)

    def _parseRefc(self, s):
        """refc rx,ry, -> 推断参考坐标维度。"""
        toks_refc = s.replace(",", " ").split()
        self._nrefc = len([t for t in toks_refc[1:] if t])

    def _parseGaus(self, idx):
        """gaus 段 -> 收集积分点坐标及权重。"""
        s = self._lines[idx].strip()
        toks = s.replace("=", " ").split()
        toks = [t for t in toks if t]
        if len(toks) >= 2:
            self._gaus_n = int(toks[1])
        cnt = 0
        j = idx + 1
        while j < len(self._lines) and cnt < self._gaus_n:
            lj = self._lines[j].strip()
            j += 1
            if lj == "":
                continue
            try:
                vals = [float(x) for x in lj.split()]
                self._gaus_lines.append(vals)
                cnt += 1
            except ValueError:
                break

    def _parseShap(self, s):
        """shap 段内行：分量标题跳过，表达式行提取，关键字结束收集。"""
        if s.endswith("=") and "=" not in s[:-1]:
            return
        if "=" in s and not s.endswith("="):
            expr = s.split("=", 1)[1].strip()
            if expr:
                self._shap_exprs.append(expr)
            return
        if any(s.startswith(k) for k in
               ("tran", "func", "dist", "mass", "load", "end",
                "coef", "refc", "coor", "node", "mate", "gaus")):
            self._in_shap = False

    def _parseFunc(self, s):
        """func = exx,eyy,exy, -> eleResNames。"""
        rhs = s.split("=", 1)[1]
        self.ele.eleResNames = [x.strip() for x in rhs.split(",") if x.strip()]

    def _parseCoef(self, s):
        """coef u,v, -> 记录依赖场位移变量。"""
        rhs = s[len("coef"):].strip()
        if rhs:
            self._ges_coef_vars = [x.strip() for x in rhs.replace(",", " ").split()
                                   if x.strip()]

    # ---- 字段填充 ----

    def _fillGauss(self):
        """填充积分点坐标及权重。"""
        self.ele.gaussPoints = []
        self.ele.gaussWeights = []
        if self._gaus_lines:
            actual_nrefc = len(self._gaus_lines[0]) - 1 if len(self._gaus_lines[0]) > 1 else 1
            if self._nrefc == 0:
                self._nrefc = actual_nrefc
            for gl in self._gaus_lines:
                self.ele.gaussPoints.append(gl[:self._nrefc])
                self.ele.gaussWeights.append(gl[-1])

    def _fillShapeFuns(self):
        """填充形函数（rx->x[1], ry->x[2] 占位符，pyTool _replaceCoordVars 会替换）。"""
        def _conv(expr):
            return expr.replace("rx", "x[1]").replace("ry", "x[2]")
        self.ele.shapeFuns = [_conv(e) for e in self._shap_exprs[:self.ele.nNodes]]

    def _fillType(self):
        """根据高斯点在几何空间的实际分布推断单元类型。"""
        gdim = self._gaussDim(self.ele.gaussPoints)
        if gdim == 1:
            self.ele.type = 1
        elif gdim == 2:
            self.ele.type = 2
        elif gdim >= 3:
            self.ele.type = 3
        else:
            self.ele.type = self._inferType(
                self.ele.nNodes, self.ele.project.project.dim, self.ele.coordVars)
        if self.ele.type == 1:
            self.ele.bBC = True

    def _fillCoordVars(self):
        """填充参考坐标变量名。"""
        if self._nrefc == 1:
            self.ele.coordVars = ["x"]
        elif self._nrefc >= 2:
            self.ele.coordVars = ["x", "y"]

    # ---- 静态工具 ----

    @staticmethod
    def _inferType(nNode, dim, coordVars):
        """由节点数+维度推断单元几何类型：0点/1线/2面/3体。"""
        if dim == 1:
            return 1
        if dim == 2:
            return 2 if nNode >= 3 else 1
        if dim == 3:
            return 3 if nNode >= 5 else 2
        return 1

    @staticmethod
    def _gaussDim(gaussPoints):
        """由高斯点实际坐标分布判断单元几何类型：1线/2面/3体（0=无积分点）。"""
        if not gaussPoints:
            return 0
        ndim = len(gaussPoints[0])
        rank = 0
        for i in range(ndim):
            vals = [gp[i] for gp in gaussPoints]
            if max(vals) - min(vals) > 1e-12:
                rank += 1
        return rank


def parseGes(projDir, eleName, ele):
    """兼容旧调用方式的便捷函数。"""
    GesParser(projDir, eleName, ele).parse()
