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

# pyTool 配置式生成器主窗口
from PySide6.QtWidgets import (
    QMainWindow, QWidget, QSplitter, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QVBoxLayout, QFileDialog, QMessageBox, QInputDialog,
    QMenu, QLabel,
)
from PySide6.QtCore import Qt

from DataProject import DataProject
from DataEleSubG import DataEleSubG
from models.project_model import ProjectModel
from services import json_io
from views.project_panel import ProjectPanel
from views.field_panel import FieldPanel
from views.element_panel import ElementPanel
from views.generate_panel import GeneratePanel


class MainWindow(QMainWindow):
    _STACK_PROJ, _STACK_FIELD, _STACK_ELE = 0, 1, 2

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("pyTool 配置式生成器")
        self.resize(1100, 760)

        self._model = ProjectModel()
        self._currentFile = None

        # ---- 左：树 ----
        self._tree = QTreeWidget()
        self._tree.setHeaderLabels(["项目结构"])
        self._tree.setContextMenuPolicy(Qt.CustomContextMenu)

        # ---- 右：堆叠面板 ----
        self._projPanel = ProjectPanel(self._model)
        self._fieldPanel = FieldPanel(self._model)
        self._elePanel = ElementPanel(self._model)
        placeholder = QLabel("（请在左侧选择节点）")
        placeholder.setAlignment(Qt.AlignCenter)
        self._stack = QStackedWidget()
        self._stack.addWidget(self._projPanel)    # 0
        self._stack.addWidget(self._fieldPanel)   # 1
        self._stack.addWidget(self._elePanel)     # 2

        splitter = QSplitter(Qt.Horizontal)
        splitter.addWidget(self._tree)
        splitter.addWidget(self._stack)
        splitter.setStretchFactor(0, 1)
        splitter.setStretchFactor(1, 3)

        # ---- 下：生成面板 ----
        self._genPanel = GeneratePanel(self._model)
        self._genPanel.setDefaults("sample/NewProj", "FEMproject/CMakeLists.txt")

        central = QWidget()
        outer = QVBoxLayout(central)
        outer.addWidget(splitter, 3)
        outer.addWidget(self._genPanel, 1)
        self.setCentralWidget(central)

        self._tree.currentItemChanged.connect(self._onTreeCurrentChanged)
        self._model.dirtyChanged.connect(self._updateTitle)
        self._model.structureChanged.connect(self.refreshTree)

        self._buildMenus()
        self.newProject()
        self.refreshTree()
        self._tree.setCurrentItem(self._tree.topLevelItem(0))

    # ---- 菜单 ----
    def _buildMenus(self):
        mb = self.menuBar()
        fileM = mb.addMenu("文件")
        for text, slot in (("新建", self.newProject),
                           ("打开…", self._open),
                           ("保存", self._save),
                           ("另存为…", self._saveAs),
                           ("导出 data.json…", self._exportDataJson)):
            fileM.addAction(text, slot)

        editM = mb.addMenu("编辑")
        editM.addAction("添加场", self._addField)
        editM.addAction("添加单元", self._addEleSub)
        editM.addAction("删除选中", self._deleteSelected)

        mb.addMenu("帮助").addAction("关于", self._about)

    # ---- 文件操作 ----
    def newProject(self):
        self._model.setProject(DataProject("", 2))
        self._currentFile = None
        self.refreshTree()

    def openProject(self, path) -> bool:
        try:
            proj = json_io.load(path)
        except Exception as e:
            QMessageBox.critical(self, "打开失败", f"无法解析工程文件：\n{e}")
            return False
        self._model.setProject(proj)
        self._currentFile = path
        self.refreshTree()
        return True

    def _open(self):
        if not self._confirmDiscard():
            return
        path, _ = QFileDialog.getOpenFileName(
            self, "打开工程文件", "", "pyTool 工程 (*.cdfeg.json);;JSON (*.json)")
        if path:
            self.openProject(path)

    def saveAs(self, path=None) -> bool:
        if path is None:
            path, _ = QFileDialog.getSaveFileName(
                self, "另存为", f"{self._model.project.name or 'project'}.cdfeg.json",
                "pyTool 工程 (*.cdfeg.json)")
            if not path:
                return False
        json_io.save(self._model.project, path)
        self._currentFile = path
        self._model.markClean()
        return True

    def _save(self):
        if self._currentFile:
            self.saveAs(self._currentFile)
        else:
            self._saveAs()

    def _saveAs(self):
        self.saveAs()

    def _exportDataJson(self):
        path, _ = QFileDialog.getSaveFileName(
            self, "导出 data.json", "data.json", "JSON (*.json)")
        if path:
            json_io.save(self._model.project, path)

    def _confirmDiscard(self) -> bool:
        if not self._model.isDirty:
            return True
        btn = QMessageBox.question(
            self, "未保存", "当前项目有未保存修改，是否丢弃？",
            QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        return btn == QMessageBox.Yes

    # ---- 编辑操作 ----
    def _addField(self):
        name, ok = QInputDialog.getText(self, "添加场", "场名称：")
        if ok and name:
            self._model.addField(name)

    def _addEleSub(self):
        field = self._selectedField()
        if field is None:
            QMessageBox.information(self, "添加单元", "请先在左侧选择一个场节点。")
            return
        name, ok = QInputDialog.getText(self, "添加单元", "单元名称：")
        if ok and name:
            gauss_btn = QMessageBox.question(
                self, "单元类型", "是高斯积分单元（IsoEleBase）吗？",
                QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            gauss = (gauss_btn == QMessageBox.Yes)
            self._model.addEleSub(field, name, gauss=gauss)

    def _deleteSelected(self):
        item = self._tree.currentItem()
        if item is None:
            return
        data = item.data(0, Qt.UserRole)
        if data is None:
            return
        kind, obj = data
        if kind == "field":
            self._model.removeField(obj)
        elif kind == "ele":
            field = item.parent().data(0, Qt.UserRole)
            self._model.removeEleSub(field[1], obj)

    def _selectedField(self):
        item = self._tree.currentItem()
        if item is None:
            return None
        data = item.data(0, Qt.UserRole)
        if data and data[0] == "field":
            return data[1]
        if data and data[0] == "ele":
            parent = item.parent()
            pdata = parent.data(0, Qt.UserRole)
            return pdata[1] if pdata else None
        return None

    # ---- 树 ----
    def refreshTree(self):
        self._tree.clear()
        proj = self._model.project
        root = QTreeWidgetItem([f"{proj.name or '(未命名)'} (dim={proj.dim})"])
        root.setData(0, Qt.UserRole, ("project", proj))
        for field in proj.fields:
            fnode = QTreeWidgetItem([field.name or "(未命名场)"])
            fnode.setData(0, Qt.UserRole, ("field", field))
            for ele in field.eleSubs:
                tag = " [G]" if isinstance(ele, DataEleSubG) else ""
                enode = QTreeWidgetItem([f"{ele.name}{tag}"])
                enode.setData(0, Qt.UserRole, ("ele", ele))
                fnode.addChild(enode)
            root.addChild(fnode)
        self._tree.addTopLevelItem(root)
        self._tree.expandAll()
        self._updateTitle()

    def _onTreeCurrentChanged(self, cur, _prev):
        if cur is None:
            return
        data = cur.data(0, Qt.UserRole)
        if not data:
            return
        kind, obj = data
        if kind == "project":
            self._stack.setCurrentIndex(self._STACK_PROJ)
            self._projPanel.loadProject()
        elif kind == "field":
            self._stack.setCurrentIndex(self._STACK_FIELD)
            self._fieldPanel.loadField(obj)
        elif kind == "ele":
            field = cur.parent().data(0, Qt.UserRole)[1]
            self._stack.setCurrentIndex(self._STACK_ELE)
            self._elePanel.loadEleSub(field, obj)

    def _updateTitle(self):
        dirty = "*" if self._model.isDirty else ""
        name = self._model.project.name or "(未命名)"
        f = f" — {self._currentFile}" if self._currentFile else ""
        self.setWindowTitle(f"pyTool 配置式生成器 — {name}{dirty}{f}")

    def _about(self):
        QMessageBox.about(self, "关于", "pyTool 配置式生成器\nPySide6 GUI\nCDFEG 项目")

    def closeEvent(self, e):
        if self._confirmDiscard():
            e.accept()
        else:
            e.ignore()
