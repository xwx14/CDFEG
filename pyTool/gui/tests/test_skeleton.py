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

# pyTool GUI 骨架冒烟测试
from main_window import MainWindow


def test_main_window_can_construct(qapp):
    """空 MainWindow 必须能被构造，且窗口标题含中文标识。"""
    win = MainWindow()
    assert win is not None
    assert "pyTool" in win.windowTitle()
    win.deleteLater()


import os
from models.project_model import ProjectModel


def test_main_window_add_field_and_save(tmp_path, qapp):
    win = MainWindow()
    win.newProject()                       # 新建空项目
    field = win._model.addField("ElDisp")
    win._model.addEleSub(field, "ElQ4g", gauss=True)
    win.refreshTree()
    assert win._tree.topLevelItemCount() == 1            # 项目根

    path = str(tmp_path / "p.cdfeg.json")
    assert win.saveAs(path)
    assert os.path.exists(path)

    # 重新打开
    assert win.openProject(path)
    assert win._model.project.fields[0].name == "ElDisp"
    win.deleteLater()


def test_main_window_switch_panel_on_select(qapp):
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    win._model.addEleSub(f, "T", gauss=False)
    win.refreshTree()
    # 选中单元叶节点（项目根→场→单元）
    root = win._tree.topLevelItem(0)
    field_node = root.child(0)
    ele_node = field_node.child(0)
    win._tree.setCurrentItem(ele_node)
    assert win._stack.currentIndex() == win._STACK_ELE
    win.deleteLater()


def test_main_window_left_button_bar_exists(qapp):
    """左栏有横向按钮栏：添加场/添加单元/删除。"""
    win = MainWindow()
    assert win._btnAddField.text() == "添加场"
    assert win._btnAddEle.text() == "添加单元"
    assert win._btnDelete.text() == "删除"
    win.deleteLater()


def test_main_window_buttons_initial_state(qapp):
    """初始选中项目根：仅添加场启用，添加单元/删除禁用。"""
    win = MainWindow()
    win.newProject()
    win.refreshTree()
    win._tree.setCurrentItem(win._tree.topLevelItem(0))  # 项目根
    assert win._btnAddField.isEnabled() is True
    assert win._btnAddEle.isEnabled() is False
    assert win._btnDelete.isEnabled() is False
    win.deleteLater()


def test_main_window_buttons_enable_on_field_select(qapp):
    """选中场节点后，添加单元/删除启用。"""
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    win._model.addEleSub(f, "T", gauss=False)
    win.refreshTree()
    root = win._tree.topLevelItem(0)
    field_node = root.child(0)
    win._tree.setCurrentItem(field_node)
    assert win._btnAddField.isEnabled() is True
    assert win._btnAddEle.isEnabled() is True
    assert win._btnDelete.isEnabled() is True
    win.deleteLater()


def test_main_window_buttons_enable_on_ele_select(qapp):
    """选中单元叶节点后，添加单元/删除同样启用（回溯到所属场）。"""
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    win._model.addEleSub(f, "T", gauss=False)
    win.refreshTree()
    root = win._tree.topLevelItem(0)
    field_node = root.child(0)
    ele_node = field_node.child(0)
    win._tree.setCurrentItem(ele_node)
    assert win._btnAddField.isEnabled() is True
    assert win._btnAddEle.isEnabled() is True
    assert win._btnDelete.isEnabled() is True
    win.deleteLater()


def test_main_window_type_switch_keeps_selection(qapp):
    """ElementPanel 切单元类型后，树重建选中保持到新对象，左栏按钮仍可用。"""
    from PySide6.QtCore import Qt
    from DataEleSubG import DataEleSubG
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    win._model.addEleSub(f, "Bar", gauss=False)
    win.refreshTree()
    root = win._tree.topLevelItem(0)
    ele_node = root.child(0).child(0)
    win._tree.setCurrentItem(ele_node)
    # 在面板把普通单元切到高斯等参元
    win._elePanel._eleType.setCurrentIndex(1)
    # 树已重建：选中应指向新对象（DataEleSubG），按钮仍启用
    cur = win._tree.currentItem()
    assert cur is not None
    assert cur.data(0, Qt.UserRole)[1] is win._elePanel._ele
    assert isinstance(win._elePanel._ele, DataEleSubG)
    assert win._btnAddEle.isEnabled() is True
    assert win._btnDelete.isEnabled() is True
    win.deleteLater()


def test_main_window_add_field_selects_new_field(qapp):
    """添加场后选中该场，右侧切到 FieldPanel（等效于 _addField 完成后的跳转）。"""
    win = MainWindow()
    win.newProject()
    field = win._model.addField("F")
    win._tree.selectByObject(field)
    assert win._stack.currentIndex() == win._STACK_FIELD
    win.deleteLater()


def test_main_window_add_ele_selects_new_ele(qapp):
    """添加单元后选中该单元，右侧切到 ElementPanel。"""
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    ele = win._model.addEleSub(f, "Bar", gauss=False)
    win._tree.selectByObject(ele)
    assert win._stack.currentIndex() == win._STACK_ELE
    win.deleteLater()
