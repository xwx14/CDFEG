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

# 项目结构树：构建 / 刷新 / 选中恢复 / 节点查找
from PySide6.QtWidgets import QTreeWidget, QTreeWidgetItem
from PySide6.QtCore import Qt

from DataEleSubG import DataEleSubG


class ProjectTreeWidget(QTreeWidget):
    """项目结构树。

    仅负责树的构建、内容刷新、选中恢复与节点查找；选中变化后的面板切换、
    按钮启停等业务协调由 MainWindow 经原生信号处理。
    """

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setHeaderLabels(["项目结构"])
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self._pendingSelect = None   # eleReplaced 记录的新对象，下次 refresh 恢复选中

    def markPendingSelect(self, obj):
        """记录单元类型切换产生的新对象，供下次 refresh 恢复选中。"""
        self._pendingSelect = obj

    def refresh(self, project):
        """按 project 重建树并恢复选中（pendingSelect 优先，否则按切换前选中）。"""
        prev = None
        cur = self.currentItem()
        if cur is not None:
            data = cur.data(0, Qt.UserRole)
            if data:
                prev = data[1]
        self.clear()
        root = QTreeWidgetItem([f"{project.name or '(未命名)'} (dim={project.dim})"])
        root.setData(0, Qt.UserRole, ("project", project))
        for field in project.fields:
            fnode = QTreeWidgetItem([field.name or "(未命名场)"])
            fnode.setData(0, Qt.UserRole, ("field", field))
            for ele in field.eleSubs:
                tag = " [G]" if isinstance(ele, DataEleSubG) else ""
                enode = QTreeWidgetItem([f"{ele.name}{tag}"])
                enode.setData(0, Qt.UserRole, ("ele", ele))
                fnode.addChild(enode)
            root.addChild(fnode)
        self.addTopLevelItem(root)
        self.expandAll()
        # 恢复选中：类型切换优先用新对象，否则按切换前选中对象（按身份 is 匹配）
        target = None
        if self._pendingSelect is not None:
            target = self._findItem(root, self._pendingSelect)
            self._pendingSelect = None
        elif prev is not None:
            target = self._findItem(root, prev)
        if target is not None:
            self.setCurrentItem(target)

    def selectByObject(self, obj):
        """按身份查找 obj 节点并设为当前选中（触发 currentItemChanged）；找不到返回 False。"""
        root = self.topLevelItem(0)
        if root is None:
            return False
        node = self._findItem(root, obj)
        if node is not None:
            self.setCurrentItem(node)
            return True
        return False

    def _findItem(self, root, obj):
        """在树中按对象身份(is)查找 UserRole data[1] is obj 的节点。"""
        nodes = [root]
        while nodes:
            node = nodes.pop()
            data = node.data(0, Qt.UserRole)
            if data and data[1] is obj:
                return node
            for i in range(node.childCount()):
                nodes.append(node.child(i))
        return None
