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

# 场节点编辑面板
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QComboBox, QCheckBox, QLabel,
)


class FieldPanel(QWidget):
    """编辑 DataField；dispNames/eleResNames 由单元聚合，只读展示。"""

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        self._field = None
        form = QFormLayout(self)

        self._name = QLineEdit()
        self._pdeType = QComboBox()
        self._pdeType.addItems(["1 椭圆型 (K)", "2 抛物型 (K/M)", "3 双曲型 (K/M/D)"])
        self._bDynamic = QCheckBox("动力学场")
        self._dispNames = QLabel("(由单元聚合)")
        self._eleResNames = QLabel("(由单元聚合)")

        form.addRow("场名称", self._name)
        form.addRow("PDE 类型", self._pdeType)
        form.addRow("动力学", self._bDynamic)
        form.addRow("广义位移", self._dispNames)
        form.addRow("单元变量", self._eleResNames)

        self._name.editingFinished.connect(self._commit)
        self._pdeType.currentIndexChanged.connect(self._commit)
        self._bDynamic.toggled.connect(self._commit)

    def loadField(self, field):
        self._field = field
        self._name.setText(field.name)
        self._pdeType.setCurrentIndex(field.pdeType - 1)
        self._bDynamic.setChecked(field.bDynamic)
        self._refreshAgg()

    def _refreshAgg(self):
        if self._field is None:
            return
        self._field.makeData()
        self._dispNames.setText(str(self._field.dispNames))
        self._eleResNames.setText(str(self._field.eleResNames))

    def _commit(self):
        if self._field is None:
            return
        self._field.name = self._name.text()
        self._field.pdeType = self._pdeType.currentIndex() + 1
        self._field.bDynamic = self._bDynamic.isChecked()
        self._refreshAgg()
        self._model.markDirty()
