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
# TableEditor：可复用行列可变表格编辑器
from PySide6.QtCore import Signal
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTableWidget, QTableWidgetItem,
    QPushButton, QHeaderView,
)


class TableEditor(QWidget):
    """行列可变表格编辑器，带增删行按钮。空行在 rows() 中被剔除。"""

    rowsChanged = Signal()

    def __init__(self, columns, parent=None):
        super().__init__(parent)
        self._ncol = len(columns)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)

        self._table = QTableWidget(0, self._ncol)
        self._table.setHorizontalHeaderLabels(columns)
        self._table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        layout.addWidget(self._table)

        row = QHBoxLayout()
        self._btnAdd = QPushButton("新增行")
        self._btnDel = QPushButton("删除行")
        row.addWidget(self._btnAdd)
        row.addWidget(self._btnDel)
        row.addStretch(1)
        layout.addLayout(row)

        self._btnAdd.clicked.connect(self._addRow)
        self._btnDel.clicked.connect(self._delRow)
        self._table.itemChanged.connect(lambda *_: self.rowsChanged.emit())

    def setRows(self, rows):
        self._table.setRowCount(0)
        for r in rows:
            self._addRowWithData([str(x) for x in r])

    def rows(self):
        out = []
        for i in range(self._table.rowCount()):
            vals = []
            for j in range(self._ncol):
                item = self._table.item(i, j)
                vals.append(item.text() if item is not None else "")
            if any(v.strip() for v in vals):   # 剔除全空行
                out.append(vals)
        return out

    def _addRowWithData(self, values):
        r = self._table.rowCount()
        self._table.insertRow(r)
        for j in range(self._ncol):
            self._table.setItem(r, j, QTableWidgetItem(values[j] if j < len(values) else ""))

    def _addRow(self):
        self._addRowWithData([""] * self._ncol)
        self.rowsChanged.emit()

    def _delRow(self):
        r = self._table.currentRow()
        if r >= 0:
            self._table.removeRow(r)
            self.rowsChanged.emit()
