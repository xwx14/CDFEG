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

# ProjectModel 测试
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from models.project_model import ProjectModel


def test_new_model_is_not_dirty():
    m = ProjectModel()
    assert m.isDirty is False
    assert m.project.name == ""
    assert m.project.dim == 2


def test_add_field_marks_dirty_and_emits():
    m = ProjectModel()
    dirty_hits = []
    struct_hits = []
    m.dirtyChanged.connect(lambda d: dirty_hits.append(d))
    m.structureChanged.connect(lambda: struct_hits.append(1))
    f = m.addField("ElDisp")
    assert isinstance(f, DataField)
    assert f in m.project.fields
    assert m.isDirty is True
    assert dirty_hits == [True]
    assert struct_hits == [1]


def test_add_elesub_gauss_creates_gauss_type():
    m = ProjectModel()
    f = m.addField("ElDisp")
    m.markClean()
    ele = m.addEleSub(f, "ElQ4g", gauss=True)
    assert isinstance(ele, DataEleSubG)
    assert ele.baseClass == "IsoEleBase"
    assert m.isDirty is True


def test_add_elessub_plain_creates_base_type():
    m = ProjectModel()
    f = m.addField("ElDisp")
    m.addEleSub(f, "Truss", gauss=False)
    assert any(isinstance(e, DataEleSub) and not isinstance(e, DataEleSubG)
               for e in f.eleSubs)


def test_remove_field_and_elesub():
    m = ProjectModel()
    f = m.addField("ElDisp")
    ele = m.addEleSub(f, "ElQ4g", gauss=True)
    m.removeEleSub(f, ele)
    assert ele not in f.eleSubs
    m.removeField(f)
    assert f not in m.project.fields


def test_set_project_resets_dirty():
    m = ProjectModel()
    m.addField("X")
    assert m.isDirty is True
    proj = DataProject("New", 3)
    m.setProject(proj)
    assert m.project is proj
    assert m.isDirty is False
