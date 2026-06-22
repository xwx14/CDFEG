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

import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/..")
from preParser import parsePre

MACS = r"E:/mfelProject/RegTest/testData/macs"
proj = parsePre(MACS, "el")
ok = True
ok &= (proj.dim == 2)
names = [f.name for f in proj.fields]
ok &= (set(names) == {"ela", "elb"})
ela = next(f for f in proj.fields if f.name == "ela")
ok &= (ela.dispNames == ["u", "v"])
ele_names = [e.name for e in ela.eleSubs]
ok &= ("a1eq4g2" in ele_names and "a2ll2" in ele_names)
a1 = next(e for e in ela.eleSubs if e.name == "a1eq4g2")
ok &= (a1.nNodes == 4)
ok &= (a1.paramNames == ["pe", "pv", "fu", "fv", "rou", "alpha"])
ok &= (a1.paramValues == [1.0e10, 0.3, 0.0, 0.0, 3000.0, 0.6])
elb = next(f for f in proj.fields if f.name == "elb")
ok &= (elb.dispNames == ["dxx", "dyy", "dxy"])
# --- ges 解析断言 ---
# a1eq4g2: 2D 4节点面单元，4个积分点，双线性形函数，func=exx,eyy,exy
a1 = next(e for e in ela.eleSubs if e.name == "a1eq4g2")
ok &= (len(a1.gaussPoints) == 4)
ok &= (abs(a1.gaussPoints[0][0] - 0.5773502692) < 1e-9)
ok &= (a1.gaussWeights == [1.0, 1.0, 1.0, 1.0])
ok &= (len(a1.shapeFuns) == 4)
ok &= ("exx" in a1.eleResNames and "eyy" in a1.eleResNames and "exy" in a1.eleResNames)
ok &= (a1.type == 2)  # 4节点2D → 面单元
ok &= (a1.bBC == False)
ok &= (a1.coordVars == ["x", "y"])
# 线单元 a2ll2: 2节点，type=1, bBC=True
a2 = next(e for e in ela.eleSubs if e.name == "a2ll2")
ok &= (a2.type == 1 and a2.bBC == True)
ok &= (len(a2.gaussPoints) == 2)
ok &= (len(a2.shapeFuns) == 2)
ok &= (a2.coordVars == ["x"])
# beq4g2: 4节点面单元(含coef)，type=2，nNodes>=3修正
beq = next(e for e in elb.eleSubs if e.name == "beq4g2")
ok &= (beq.type == 2)
ok &= (len(beq.gaussPoints) == 4)
ok &= (len(beq.shapeFuns) == 4)
ok &= (hasattr(beq, "_gesCoefVars") and beq._gesCoefVars == ["u", "v"])
print("parsePre:", "PASS" if ok else "FAIL")
sys.exit(0 if ok else 1)
