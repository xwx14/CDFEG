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


def test_change_elesub_type_plain_to_gauss_keeps_common_fields():
    m = ProjectModel()
    f = m.addField("F")
    ele = m.addEleSub(f, "Bar", gauss=False)
    ele.nNodes = 3
    ele.paramNames = ["E", "A"]
    m.markClean()
    newEle = m.changeEleSubType(f, ele, gauss=True)
    assert isinstance(newEle, DataEleSubG)
    assert newEle.baseClass == "IsoEleBase"
    assert newEle.name == "Bar"
    assert newEle.nNodes == 3
    assert newEle.paramNames == ["E", "A"]
    assert f.eleSubs[0] is newEle          # 原地替换保持位置
    assert ele not in f.eleSubs
    assert m.isDirty is True


def test_change_elesub_type_gauss_to_plain_drops_gauss_fields():
    m = ProjectModel()
    f = m.addField("F")
    g = m.addEleSub(f, "Q4", gauss=True)
    g.gaussPoints = [[0.5, 0.5]]
    g.gaussWeights = [1.0]
    g.shapeFuns = ["N1"]
    newEle = m.changeEleSubType(f, g, gauss=False)
    assert isinstance(newEle, DataEleSub) and not isinstance(newEle, DataEleSubG)
    assert newEle.baseClass == "ElementBase"
    assert not hasattr(newEle, "gaussPoints")   # 普通类不含高斯字段
    assert f.eleSubs[0] is newEle


def test_change_elesub_type_idempotent_when_same():
    m = ProjectModel()
    f = m.addField("F")
    g = m.addEleSub(f, "Q4", gauss=True)
    same = m.changeEleSubType(f, g, gauss=True)
    assert same is g                          # 同类型幂等返回原对象
