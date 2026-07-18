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

# 面板读写 model 测试
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from models.project_model import ProjectModel
from views.project_panel import ProjectPanel
from views.field_panel import FieldPanel


def test_project_panel_edits_name_and_dim(qapp):
    proj = DataProject("Truss1D", 1)
    m = ProjectModel.fromProject(proj)
    p = ProjectPanel(m)
    p.loadProject()
    p._name.setText("Renamed")
    p._dim.setCurrentIndex(2)  # 0->1D,1->2D,2->3D
    p._commit()
    assert proj.name == "Renamed"
    assert proj.dim == 3
    assert m.isDirty is True


def test_field_panel_edits_pdtype(qapp):
    m = ProjectModel()
    f = m.addField("ElDisp")
    m.markClean()
    fp = FieldPanel(m)
    fp.loadField(f)
    fp._pdeType.setCurrentIndex(1)  # 0->1椭圆,1->2抛物,2->3双曲
    fp._commit()
    assert f.pdeType == 2
    assert m.isDirty is True


# ---- ElementPanel (Task 8) ----
from DataEleSubG import DataEleSubG
from views.element_panel import ElementPanel


def test_element_panel_plain_writes_back(qapp):
    m = ProjectModel()
    f = m.addField("F")
    ele = m.addEleSub(f, "Truss", gauss=False)
    m.markClean()
    ep = ElementPanel(m)
    ep.loadEleSub(f, ele)
    assert ep._gaussGroup.isHidden()  # 普通单元不显示高斯区
    ep._name.setText("Truss2")
    ep._nNodes.setValue(3)
    ep._commit()
    assert ele.name == "Truss2"
    assert ele.nNodes == 3
    assert m.isDirty is True


def test_element_panel_gauss_shows_gauss_group(qapp):
    m = ProjectModel()
    f = m.addField("F")
    g = m.addEleSub(f, "ElQ4g", gauss=True)
    g.gaussPoints = [[0.5, 0.5]]
    g.gaussWeights = [1.0]
    g.shapeFuns = ["N1"]
    ep = ElementPanel(m)
    ep.loadEleSub(f, g)
    assert not ep._gaussGroup.isHidden()  # G 单元显示高斯区
    assert ep._gaussPoints.rows() == [["0.5", "0.5", ""]]
    assert ep._gaussWeights.items() == ["1.0"]
    assert ep._shapeFuns.items() == ["N1"]


def test_element_panel_param_table_roundtrip(qapp):
    m = ProjectModel()
    f = m.addField("F")
    ele = m.addEleSub(f, "Truss", gauss=False)
    ele.paramNames = ["E", "A"]
    ele.paramValues = ["", ""]
    ep = ElementPanel(m)
    ep.loadEleSub(f, ele)
    assert ep._params.rows() == [["E", ""], ["A", ""]]
