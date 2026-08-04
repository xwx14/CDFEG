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

# 添加单元对话框（单元名称 + 单元类型）
from PySide6.QtWidgets import (
    QDialog, QFormLayout, QLineEdit, QComboBox, QDialogButtonBox,
)


class AddEleSubDialog(QDialog):
    """添加单元对话框：单元名称 + 单元类型（普通单元 / 高斯等参元）。"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("添加单元")
        form = QFormLayout(self)
        self._nameEdit = QLineEdit()
        self._typeCombo = QComboBox()
        self._typeCombo.addItems(["普通单元", "高斯等参元"])
        form.addRow("单元名称", self._nameEdit)
        form.addRow("单元类型", self._typeCombo)
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        form.addRow(buttons)

    def values(self):
        """返回 (name, gauss)；名称为空返回 None。"""
        name = self._nameEdit.text().strip()
        if not name:
            return None
        return name, self._typeCombo.currentIndex() == 1
