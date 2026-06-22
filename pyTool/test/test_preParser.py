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
print("parsePre:", "PASS" if ok else "FAIL")
sys.exit(0 if ok else 1)
