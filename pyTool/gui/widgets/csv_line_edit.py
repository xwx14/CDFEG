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
# CsvLineEdit：单行逗号输入编辑器（中英文逗号均可），接口对齐 ListEditor
import re

from PySide6.QtCore import Signal
from PySide6.QtWidgets import QWidget, QVBoxLayout, QLabel, QLineEdit


class CsvLineEdit(QWidget):
    """单行逗号分隔输入；setItems/items/itemsChanged 与 ListEditor 同义。"""

    itemsChanged = Signal()

    # 仅英文逗号与中文全角逗号；不含分号（需求仅逗号）
    _SPLIT_RE = re.compile(r"[，,]")

    def __init__(self, label: str = "", placeholder: str = "", parent=None):
        super().__init__(parent)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        if label:
            layout.addWidget(QLabel(label))

        self._edit = QLineEdit()
        self._edit.setPlaceholderText(placeholder or "逗号分隔，如：u, v, w")
        layout.addWidget(self._edit)

        # editingFinished（失焦/回车）才发信号，与 ElementPanel 现有 QLineEdit 风格一致；
        # programmatic setText 不触发 editingFinished，故 setItems 不会误触发。
        self._edit.editingFinished.connect(self.itemsChanged.emit)

    def setItems(self, items: list[str]) -> None:
        # 统一以「英文逗号 + 空格」展示；不主动 emit，与 ListEditor 行为一致。
        self._edit.setText(", ".join(str(x) for x in items))

    def items(self) -> list[str]:
        parts = self._SPLIT_RE.split(self._edit.text())
        return [p.strip() for p in parts if p.strip()]
