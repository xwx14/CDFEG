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
# 可复用 widget 测试
from widgets.list_editor import ListEditor


def test_list_editor_set_get(qapp):
    le = ListEditor("广义位移")
    le.setItems(["u", "v"])
    assert le.items() == ["u", "v"]


def test_list_editor_add_remove_move(qapp):
    le = ListEditor()
    le.setItems(["a", "b", "c"])
    # 选中第 1 行（"b"）下移
    le._list.setCurrentRow(1)
    le._moveDown()
    assert le.items() == ["a", "c", "b"]
    # 删除第 0 行
    le._list.setCurrentRow(0)
    le._remove()
    assert le.items() == ["c", "b"]


def test_list_editor_emits_changed(qapp):
    le = ListEditor()
    hits = []
    le.itemsChanged.connect(lambda: hits.append(1))
    le.setItems(["a", "b"])
    # setItems 不触发
    assert hits == []
    # 删除触发 itemsChanged
    le._list.setCurrentRow(0)
    le._remove()
    assert len(hits) >= 1
    assert le.items() == ["b"]


from widgets.table_editor import TableEditor


def test_table_editor_set_get(qapp):
    te = TableEditor(["名称", "默认值"])
    te.setRows([["E", ""], ["A", ""]])
    assert te.rows() == [["E", ""], ["A", ""]]


def test_table_editor_gauss_points(qapp):
    te = TableEditor(["xi", "eta"])
    te.setRows([["0.5", "0.5"], ["-0.5", "0.5"]])
    rows = te.rows()
    assert rows == [["0.5", "0.5"], ["-0.5", "0.5"]]


def test_table_editor_empty_rows_dropped(qapp):
    te = TableEditor(["a"])
    te.setRows([["x"], [""], ["y"]])
    assert te.rows() == [["x"], ["y"]]
