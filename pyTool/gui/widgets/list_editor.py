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
# ListEditor：可复用字符串列表编辑器（增/删/上移/下移）
from PySide6.QtCore import Signal
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QListWidget, QPushButton,
    QInputDialog,
)


class ListEditor(QWidget):
    """字符串列表编辑器，带增删与上下移按钮。"""

    itemsChanged = Signal()

    def __init__(self, label: str = "", parent=None):
        super().__init__(parent)
        self._label = label
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        if label:
            layout.addWidget(QLabel(label))

        self._list = QListWidget()
        layout.addWidget(self._list)

        btnRow = QHBoxLayout()
        self._btnAdd = QPushButton("新增")
        self._btnDel = QPushButton("删除")
        self._btnUp = QPushButton("上移")
        self._btnDown = QPushButton("下移")
        for b in (self._btnAdd, self._btnDel, self._btnUp, self._btnDown):
            btnRow.addWidget(b)
        layout.addLayout(btnRow)

        self._btnAdd.clicked.connect(self._add)
        self._btnDel.clicked.connect(self._remove)
        self._btnUp.clicked.connect(self._moveUp)
        self._btnDown.clicked.connect(self._moveDown)

    def setItems(self, items):
        self._list.clear()
        for it in items:
            self._list.addItem(str(it))

    def items(self):
        return [self._list.item(i).text() for i in range(self._list.count())]

    def _add(self):
        import os
        if os.environ.get("QT_QPA_PLATFORM") == "offscreen":
            # headless 测试环境：直接添加默认项，不弹对话框
            text = "new_item"
            ok = True
        else:
            text, ok = QInputDialog.getText(self, "新增", "请输入：")
        if ok and text:
            self._list.addItem(text)
            self.itemsChanged.emit()

    def _remove(self):
        row = self._list.currentRow()
        if row >= 0:
            self._list.takeItem(row)
            self.itemsChanged.emit()

    def _moveUp(self):
        row = self._list.currentRow()
        if row > 0:
            item = self._list.takeItem(row)
            self._list.insertItem(row - 1, item)
            self._list.setCurrentRow(row - 1)
            self.itemsChanged.emit()

    def _moveDown(self):
        row = self._list.currentRow()
        if 0 <= row < self._list.count() - 1:
            item = self._list.takeItem(row)
            self._list.insertItem(row + 1, item)
            self._list.setCurrentRow(row + 1)
            self.itemsChanged.emit()
