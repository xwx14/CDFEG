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

# 项目节点编辑面板
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QComboBox, QLabel,
)


class ProjectPanel(QWidget):
    """编辑 DataProject 的 name / dim（coordVars 按 dim 派生展示）。"""

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        form = QFormLayout(self)

        self._name = QLineEdit()
        self._dim = QComboBox()
        self._dim.addItems(["1 (一维)", "2 (二维)", "3 (三维)"])
        self._coordVars = QLabel("(由维度派生)")

        form.addRow("项目名称", self._name)
        form.addRow("总体维度", self._dim)
        form.addRow("坐标变量", self._coordVars)

        self._name.editingFinished.connect(self._commit)
        self._dim.currentIndexChanged.connect(self._commit)

    def loadProject(self):
        proj = self._model.project
        self._name.setText(proj.name)
        self._dim.setCurrentIndex(proj.dim - 1)
        self._refreshCoordVars(proj.dim)

    def _refreshCoordVars(self, dim):
        proj = self._model.project
        proj.coordVars = ['x', 'y', 'z'][:dim]
        self._coordVars.setText(str(proj.coordVars))

    def _commit(self):
        proj = self._model.project
        proj.name = self._name.text()
        proj.dim = self._dim.currentIndex() + 1
        self._refreshCoordVars(proj.dim)
        self._model.markDirty()
