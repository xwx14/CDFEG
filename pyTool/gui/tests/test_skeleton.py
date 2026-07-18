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
