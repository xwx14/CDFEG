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

# 单元编辑面板（QTabWidget 分页：基本 / 材料参数 / 积分配置）
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QSpinBox, QComboBox, QCheckBox,
    QGroupBox, QVBoxLayout, QHBoxLayout, QLabel, QTabWidget,
)

from DataEleSubG import DataEleSubG
from widgets.list_editor import ListEditor
from widgets.csv_line_edit import CsvLineEdit
from widgets.table_editor import TableEditor


class ElementPanel(QWidget):
    """单元字段编辑；内容用 QTabWidget 分三页：基本 / 材料参数 / 积分配置。

    普通单元隐藏「积分配置」Tab（仅高斯等参元需要积分配置）。
    """

    # 单元类型索引：0=普通单元(DataEleSub)，1=高斯等参元(DataEleSubG)
    _TYPE_PLAIN, _TYPE_GAUSS = 0, 1
    # 积分点表列：x1 / x2 / x3 / 权重（坐标按维度填写，多余列留空）
    _GAUSS_COLS = ["x1", "x2", "x3", "权重"]

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        self._field = None
        self._ele = None

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
        # 单元类型：决定 DataEleSub / DataEleSubG（即 baseClass），可切换
        self._eleType = QComboBox()
        self._eleType.addItems(["普通单元", "高斯等参元"])
        self._matName = QLineEdit()
        form.addRow("单元名称", self._name)
        form.addRow("节点数", self._nNodes)
        form.addRow("几何类型", self._type)
        form.addRow("单元维度", self._dim)
        form.addRow("边界单元", self._bBC)
        form.addRow("单元类型", self._eleType)
        form.addRow("材料名", self._matName)
        common.setLayout(form)

        # ---- 位移与变量名 ----
        lists = QGroupBox("位移与变量名")
        lh = QHBoxLayout()
        self._dispNames = CsvLineEdit("广义位移", placeholder="逗号分隔，如：u, v, w")
        self._eleResNames = CsvLineEdit("单元变量", placeholder="逗号分隔，如：sx, sy, sxy")
        lh.addWidget(self._dispNames)
        lh.addWidget(self._eleResNames)
        lists.setLayout(lh)

        # ---- 材料参数 ----
        params = QGroupBox("材料参数（名 / 默认值）")
        pv = QVBoxLayout()
        self._params = TableEditor(["名称", "默认值"])
        pv.addWidget(self._params)
        params.setLayout(pv)

        # ---- 高斯积分配置（仅高斯等参元；普通单元隐藏此 Tab）----
        gaussPage = QWidget()
        gv = QVBoxLayout(gaussPage)
        gv.setContentsMargins(8, 8, 8, 8)
        gform = QFormLayout()
        self._gaussOrder = QSpinBox()
        self._gaussOrder.setRange(1, 10)
        gform.addRow("积分阶数", self._gaussOrder)
        self._nGauss = QSpinBox()
        self._nGauss.setRange(0, 999)
        gform.addRow("积分点数", self._nGauss)
        gv.addLayout(gform)
        gv.addWidget(QLabel("积分点（每行：坐标 x1/x2/x3 + 权重）"))
        self._gaussTable = TableEditor(self._GAUSS_COLS)
        gv.addWidget(self._gaussTable)
        gv.addWidget(QLabel("形函数表达式（个数 = 节点数，与积分点数无关）"))
        self._shapeFuns = ListEditor("形函数")
        gv.addWidget(self._shapeFuns)
        gv.addStretch(1)

        # ---- 用 QTabWidget 组织三页 ----
        self._tabs = QTabWidget()
        # 页「基本」：公共属性 + 位移与变量名
        basicPage = QWidget()
        bpl = QVBoxLayout(basicPage)
        bpl.setContentsMargins(8, 8, 8, 8)
        bpl.addWidget(common)
        bpl.addWidget(lists)
        bpl.addStretch(1)
        self._tabs.addTab(basicPage, "基本")
        # 页「材料参数」
        matPage = QWidget()
        mpl = QVBoxLayout(matPage)
        mpl.setContentsMargins(8, 8, 8, 8)
        mpl.addWidget(params)
        mpl.addStretch(1)
        self._tabs.addTab(matPage, "材料参数")
        # 页「积分配置」：高斯积分（普通单元隐藏）
        self._gaussTabIndex = self._tabs.addTab(gaussPage, "积分配置")

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.addWidget(self._tabs)

        # 编辑即提交（_eleType 走类型切换；_nGauss 走行数联动）
        for w in (self._name,):
            w.editingFinished.connect(self._commit)
        for s in (self._nNodes, self._dim, self._gaussOrder):
            s.valueChanged.connect(self._commit)
        self._type.currentIndexChanged.connect(self._commit)
        self._eleType.currentIndexChanged.connect(self._onTypeChanged)
        self._nGauss.valueChanged.connect(self._onNGaussChanged)
        self._bBC.toggled.connect(self._commit)
        self._matName.editingFinished.connect(self._commit)
        self._dispNames.itemsChanged.connect(self._commit)
        self._eleResNames.itemsChanged.connect(self._commit)
        self._params.rowsChanged.connect(self._commit)
        self._gaussTable.rowsChanged.connect(self._commit)
        self._shapeFuns.itemsChanged.connect(self._commit)

    def _currentGauss(self):
        """当前 ComboBox 选项是否为高斯等参元。"""
        return self._eleType.currentIndex() == self._TYPE_GAUSS

    def _onTypeChanged(self, *_):
        """单元类型切换：替换 DataEleSub ↔ DataEleSubG 并刷新面板。

        高斯→普通会丢弃高斯积分配置（由 changeEleSubType 处理）；类型未变则直接返回。
        """
        if self._ele is None or self._field is None:
            return
        gauss = self._currentGauss()
        if gauss == isinstance(self._ele, DataEleSubG):
            return  # 类型未变
        newEle = self._model.changeEleSubType(self._field, self._ele, gauss)
        self.loadEleSub(self._field, newEle)

    def _onNGaussChanged(self, n):
        """积分点数 SpinBox：尾部增删积分点表格行（保留已有数据）。"""
        if self._ele is None:
            return
        while self._gaussTable.rowCount() < n:
            self._gaussTable.addEmptyRow()      # emit rowsChanged → _commit
        while self._gaussTable.rowCount() > n:
            self._gaussTable.removeLastRow()    # emit rowsChanged → _commit

    def loadEleSub(self, field, ele):
        self._field = field
        self._ele = ele
        # 临时断开信号避免回填触发 commit / _onTypeChanged / _onNGaussChanged
        self._blockAll(True)
        try:
            self._name.setText(ele.name)
            self._nNodes.setValue(ele.nNodes)
            self._type.setCurrentIndex(ele.type)
            self._dim.setValue(ele.dim)
            self._bBC.setChecked(ele.bBC)
            is_g = isinstance(ele, DataEleSubG)
            self._eleType.setCurrentIndex(self._TYPE_GAUSS if is_g else self._TYPE_PLAIN)
            # 普通单元隐藏「积分配置」Tab，仅高斯等参元显示
            self._tabs.setTabVisible(self._gaussTabIndex, is_g)
            self._matName.setText(ele.matName or ele.name)
            self._dispNames.setItems(ele.dispNames)
            self._eleResNames.setItems(ele.eleResNames)
            self._params.setRows(list(map(list, zip(
                ele.paramNames,
                [str(v) for v in ele.paramValues] if ele.paramValues else [""] * len(ele.paramNames)
            ))) if ele.paramNames else [])
            if is_g:
                self._gaussOrder.setValue(ele.gaussOrder)
                nGauss = len(ele.gaussPoints)
                self._nGauss.setValue(nGauss)
                # 合成积分点表：[x1, x2, x3, 权重]（坐标按维度填，多余列空）
                rows = []
                for i in range(nGauss):
                    pt = ele.gaussPoints[i]
                    w = ele.gaussWeights[i] if i < len(ele.gaussWeights) else ""
                    rows.append([str(pt[j]) if j < len(pt) else "" for j in range(3)] + [str(w)])
                self._gaussTable.setRows(rows)
                self._shapeFuns.setItems(ele.shapeFuns)
        finally:
            self._blockAll(False)

    def _blockAll(self, on):
        for w in (self._name, self._nNodes, self._dim, self._gaussOrder, self._nGauss,
                  self._type, self._eleType, self._bBC, self._matName):
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
        ele.matName = self._matName.text()
        ele.dispNames = self._dispNames.items()
        ele.eleResNames = self._eleResNames.items()
        rows = self._params.rows()
        ele.paramNames = [r[0] for r in rows]
        ele.paramValues = [r[1] if len(r) > 1 else "" for r in rows]
        # baseClass 不在此写回——由 DataEleSub / DataEleSubG 的 __init__ 决定
        if isinstance(ele, DataEleSubG):
            ele.gaussOrder = self._gaussOrder.value()
            # 积分点表行 = [x1, x2, x3, 权重]：前 3 列为坐标（剔除空列），末列为权重
            grows = self._gaussTable.rows()
            ele.gaussPoints = [[float(x) for x in r[:3] if x.strip()] for r in grows]
            ele.gaussWeights = [float(r[3]) for r in grows if len(r) > 3 and r[3].strip()]
            ele.shapeFuns = self._shapeFuns.items()
        if self._field is not None:
            self._field.makeData()
        self._model.markDirty()
