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

# 单元编辑面板（含 DataEleSubG 高斯区分支）
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QSpinBox, QComboBox, QCheckBox,
    QGroupBox, QVBoxLayout, QHBoxLayout, QLabel,
)

from DataEleSubG import DataEleSubG
from widgets.list_editor import ListEditor
from widgets.csv_line_edit import CsvLineEdit
from widgets.table_editor import TableEditor


class ElementPanel(QWidget):
    """单元字段编辑；DataEleSubG 额外显示高斯积分区。"""

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        self._field = None
        self._ele = None
        root = QVBoxLayout(self)

        # ---- 公共字段 ----
        common = QGroupBox("单元公共属性")
        form = QFormLayout()
        self._name = QLineEdit()
        self._nNodes = QSpinBox()
        self._nNodes.setRange(1, 999)
        self._type = QComboBox()
        self._type.addItems(["0 点单元", "1 线单元", "2 面单元", "3 体单元"])
        self._dim = QSpinBox()
        self._dim.setRange(1, 3)
        self._bBC = QCheckBox("边界单元")
        self._baseClass = QComboBox()
        self._baseClass.addItems(["EleSubBase", "IsoEleBase"])
        # 类型由实例决定（DataEleSub vs DataEleSubG），只读反映当前类型
        self._baseClass.setEnabled(False)
        self._gidName = QLineEdit()
        form.addRow("单元名称", self._name)
        form.addRow("节点数", self._nNodes)
        form.addRow("几何类型", self._type)
        form.addRow("单元维度", self._dim)
        form.addRow("边界单元", self._bBC)
        form.addRow("基类", self._baseClass)
        form.addRow("GiD 名称", self._gidName)
        common.setLayout(form)
        root.addWidget(common)

        # ---- 列表：dispNames / eleResNames ----
        lists = QGroupBox("位移与变量名")
        lh = QHBoxLayout()
        self._dispNames = CsvLineEdit("广义位移", placeholder="逗号分隔，如：u, v, w")
        self._eleResNames = CsvLineEdit("单元变量", placeholder="逗号分隔，如：sx, sy, sxy")
        lh.addWidget(self._dispNames)
        lh.addWidget(self._eleResNames)
        lists.setLayout(lh)
        root.addWidget(lists)

        # ---- 参数表（名 / 默认值）----
        params = QGroupBox("材料参数（名 / 默认值）")
        pv = QVBoxLayout()
        self._params = TableEditor(["名称", "默认值"])
        pv.addWidget(self._params)
        params.setLayout(pv)
        root.addWidget(params)

        # ---- 高斯区（仅 DataEleSubG）----
        self._gaussGroup = QGroupBox("高斯积分（仅 IsoEleBase 单元）")
        gv = QVBoxLayout()
        gform = QFormLayout()
        self._gaussOrder = QSpinBox()
        self._gaussOrder.setRange(1, 10)
        gform.addRow("积分阶数", self._gaussOrder)
        gv.addLayout(gform)
        gv.addWidget(QLabel("积分点坐标（每行一个点，列=单元维度）"))
        self._gaussPoints = TableEditor(["x1", "x2", "x3"])
        gv.addWidget(self._gaussPoints)
        gv.addWidget(QLabel("积分权重"))
        self._gaussWeights = ListEditor("权重")
        gv.addWidget(self._gaussWeights)
        gv.addWidget(QLabel("形函数表达式"))
        self._shapeFuns = ListEditor("形函数")
        gv.addWidget(self._shapeFuns)
        self._gaussGroup.setLayout(gv)
        self._gaussGroup.setVisible(False)
        root.addWidget(self._gaussGroup)

        root.addStretch(1)

        # 编辑即提交
        for w in (self._name,):
            w.editingFinished.connect(self._commit)
        for s in (self._nNodes, self._dim, self._gaussOrder):
            s.valueChanged.connect(self._commit)
        for c in (self._type, self._baseClass):
            c.currentIndexChanged.connect(self._commit)
        self._bBC.toggled.connect(self._commit)
        self._gidName.editingFinished.connect(self._commit)
        self._dispNames.itemsChanged.connect(self._commit)
        self._eleResNames.itemsChanged.connect(self._commit)
        self._params.rowsChanged.connect(self._commit)
        self._gaussPoints.rowsChanged.connect(self._commit)
        self._gaussWeights.itemsChanged.connect(self._commit)
        self._shapeFuns.itemsChanged.connect(self._commit)

    def loadEleSub(self, field, ele):
        self._field = field
        self._ele = ele
        # 临时断开信号避免回填触发 commit
        self._blockAll(True)
        try:
            self._name.setText(ele.name)
            self._nNodes.setValue(ele.nNodes)
            self._type.setCurrentIndex(ele.type)
            self._dim.setValue(ele.dim)
            self._bBC.setChecked(ele.bBC)
            self._baseClass.setCurrentIndex(0 if ele.baseClass != "IsoEleBase" else 1)
            self._gidName.setText(ele.gidName or ele.name)
            self._dispNames.setItems(ele.dispNames)
            self._eleResNames.setItems(ele.eleResNames)
            self._params.setRows(list(map(list, zip(
                ele.paramNames,
                [str(v) for v in ele.paramValues] if ele.paramValues else [""] * len(ele.paramNames)
            ))) if ele.paramNames else [])
            is_g = isinstance(ele, DataEleSubG)
            self._gaussGroup.setVisible(is_g)
            if is_g:
                self._gaussOrder.setValue(ele.gaussOrder)
                self._gaussPoints.setRows([list(map(str, pt)) for pt in ele.gaussPoints])
                self._gaussWeights.setItems([str(w) for w in ele.gaussWeights])
                self._shapeFuns.setItems(ele.shapeFuns)
        finally:
            self._blockAll(False)

    def _blockAll(self, on):
        for w in (self._name, self._nNodes, self._dim, self._gaussOrder,
                  self._type, self._baseClass, self._bBC, self._gidName):
            w.blockSignals(on)

    def _commit(self, *_):
        if self._ele is None:
            return
        ele = self._ele
        ele.name = self._name.text()
        ele.nNodes = self._nNodes.value()
        ele.type = self._type.currentIndex()
        ele.dim = self._dim.value()
        ele.bBC = self._bBC.isChecked()
        ele.gidName = self._gidName.text()
        ele.dispNames = self._dispNames.items()
        ele.eleResNames = self._eleResNames.items()
        rows = self._params.rows()
        ele.paramNames = [r[0] for r in rows]
        ele.paramValues = [r[1] if len(r) > 1 else "" for r in rows]
        if isinstance(ele, DataEleSubG):
            ele.gaussOrder = self._gaussOrder.value()
            ele.gaussPoints = [[float(x) for x in r if x.strip()] for r in self._gaussPoints.rows()]
            ele.gaussWeights = [float(w) for w in self._gaussWeights.items() if w.strip()]
            ele.shapeFuns = self._shapeFuns.items()
        if self._field is not None:
            self._field.makeData()
        self._model.markDirty()
