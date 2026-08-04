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

# ProjectTreeWidget 测试
from PySide6.QtCore import Qt

from DataEleSubG import DataEleSubG
from models.project_model import ProjectModel
from views.project_tree import ProjectTreeWidget


def _sampleProject():
    m = ProjectModel()
    m.project.name = "P"
    f = m.addField("F")
    m.addEleSub(f, "Bar", gauss=False)
    m.addEleSub(f, "Q4", gauss=True)
    return m.project, f


def test_refresh_builds_tree_with_g_tag(qapp):
    tree = ProjectTreeWidget()
    proj, _ = _sampleProject()
    tree.refresh(proj)
    root = tree.topLevelItem(0)
    assert root.text(0) == "P (dim=2)"
    field_node = root.child(0)
    assert field_node.text(0) == "F"
    assert field_node.child(0).text(0) == "Bar"        # 普通单元无标记
    assert field_node.child(1).text(0) == "Q4 [G]"     # 高斯等参元带 [G]


def test_refresh_restores_prev_selection(qapp):
    tree = ProjectTreeWidget()
    proj, f = _sampleProject()
    tree.refresh(proj)
    root = tree.topLevelItem(0)
    tree.setCurrentItem(root.child(0).child(0))   # 选中 Bar
    # 对象未变，再次 refresh 后选中应恢复到同一对象
    tree.refresh(proj)
    assert tree.currentItem().data(0, Qt.UserRole)[1] is f.eleSubs[0]


def test_refresh_pending_select_overrides_prev(qapp):
    tree = ProjectTreeWidget()
    proj, f = _sampleProject()
    tree.refresh(proj)
    root = tree.topLevelItem(0)
    tree.setCurrentItem(root.child(0).child(0))   # 选中 Bar
    # 模拟类型切换：Bar 被替换为新对象
    newEle = DataEleSubG("Bar")
    f.eleSubs[0] = newEle
    tree.markPendingSelect(newEle)
    tree.refresh(proj)
    # pendingSelect 优先：选中指向新对象而非丢失
    assert tree.currentItem() is not None
    assert tree.currentItem().data(0, Qt.UserRole)[1] is newEle


def test_find_item_by_identity(qapp):
    tree = ProjectTreeWidget()
    proj, f = _sampleProject()
    tree.refresh(proj)
    root = tree.topLevelItem(0)
    target = f.eleSubs[1]
    node = tree._findItem(root, target)
    assert node is not None
    assert node.data(0, Qt.UserRole)[1] is target
    # 不存在的对象返回 None
    assert tree._findItem(root, DataEleSubG("nope")) is None
