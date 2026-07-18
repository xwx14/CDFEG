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
