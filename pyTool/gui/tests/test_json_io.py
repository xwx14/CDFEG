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

# json_io round-trip 测试（重点：DataEleSubG 高斯字段不丢失）
import os

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from services import json_io


def _buildMixedProject() -> DataProject:
    """含 1 个普通单元 + 1 个高斯积分单元的项目。"""
    proj = DataProject("Mixed", 2)
    field = DataField("ElDisp")
    plain = DataEleSub("Truss", 2)
    plain.dispNames = ["u"]
    plain.paramNames = ["E", "A"]
    field.addEleSub(plain)

    g = DataEleSubG("ElQ4g", 4)
    g.type = 2
    g.dispNames = ["u", "v"]
    g.paramNames = ["pe", "pv"]
    g.gaussPoints = [[0.5, 0.5], [-0.5, 0.5]]
    g.gaussWeights = [1.0, 1.0]
    g.shapeFuns = ["0.25*(1-x)*(1-y)", "0.25*(1+x)*(1-y)"]
    field.addEleSub(g)

    proj.addField(field)
    return proj


def test_roundtrip_preserves_plain_and_gauss(tmp_path):
    proj = _buildMixedProject()
    path = str(tmp_path / "mixed.cdfeg.json")
    json_io.save(proj, path)
    assert os.path.exists(path)

    loaded = json_io.load(path)
    assert loaded.name == "Mixed"
    assert len(loaded.fields) == 1
    fld = loaded.fields[0]
    assert len(fld.eleSubs) == 2

    # 普通单元
    plain = next(e for e in fld.eleSubs if e.name == "Truss")
    assert not isinstance(plain, DataEleSubG)
    assert plain.paramNames == ["E", "A"]

    # 高斯单元：必须是 DataEleSubG 且高斯字段完整
    g = next(e for e in fld.eleSubs if e.name == "ElQ4g")
    assert isinstance(g, DataEleSubG)
    assert g.baseClass == "IsoEleBase"
    assert g.gaussPoints == [[0.5, 0.5], [-0.5, 0.5]]
    assert g.gaussWeights == [1.0, 1.0]
    assert g.shapeFuns == ["0.25*(1-x)*(1-y)", "0.25*(1+x)*(1-y)"]


def test_load_bad_json_raises(tmp_path):
    path = str(tmp_path / "bad.cdfeg.json")
    with open(path, "w", encoding="utf-8") as f:
        f.write("{ 不是合法 json")
    try:
        json_io.load(path)
        assert False, "应抛异常"
    except Exception:
        assert True
