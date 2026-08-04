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

# ProjectModel：DataProject 的 ViewModel 包装（脏标记 + Qt 信号）
from PySide6.QtCore import QObject, Signal

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG


class ProjectModel(QObject):
    """当前会话 DataProject 的包装，负责增删改与脏标记广播。"""

    dirtyChanged = Signal(bool)
    structureChanged = Signal()
    # 单元类型切换：发射 (oldEle, newEle)，供视图在树重建后恢复选中到新对象
    eleReplaced = Signal(object, object)

    def __init__(self, project=None):
        super().__init__()
        self.project = project if project is not None else DataProject("", 2)
        self._dirty = False

    # ---- 工厂 ----
    @classmethod
    def fromProject(cls, project):
        return cls(project)

    # ---- 脏标记 ----
    @property
    def isDirty(self):
        return self._dirty

    def markDirty(self):
        if not self._dirty:
            self._dirty = True
            self.dirtyChanged.emit(True)

    def markClean(self):
        if self._dirty:
            self._dirty = False
            self.dirtyChanged.emit(False)

    # ---- 整体替换 ----
    def setProject(self, project):
        self.project = project
        self._dirty = False
        self.dirtyChanged.emit(False)
        self.structureChanged.emit()

    def toProject(self):
        return self.project

    # ---- 增删 ----
    def addField(self, name):
        field = self.project.addField(name)
        self._afterStructChange()
        return field

    def addEleSub(self, field, name, gauss=False):
        ele = DataEleSubG(name) if gauss else DataEleSub(name)
        field.addEleSub(ele)
        self._afterStructChange()
        return ele

    def changeEleSubType(self, field, oldEle, gauss):
        """切换 DataEleSub ↔ DataEleSubG，原地替换保持列表位置，返回新对象。

        复制公共字段（排除 baseClass 与高斯特有字段——它们由新类 __init__ 决定）。
        高斯→普通会丢弃已填的高斯积分配置；同类型调用幂等返回原对象。
        """
        isG = isinstance(oldEle, DataEleSubG)
        if (gauss and isG) or (not gauss and not isG):
            return oldEle
        newEle = DataEleSubG(oldEle.name) if gauss else DataEleSub(oldEle.name)
        skip = {'baseClass', 'gaussOrder', 'gaussPoints', 'gaussWeights', 'shapeFuns'}
        for k, v in oldEle.__dict__.items():
            if k not in skip:
                setattr(newEle, k, v)
        field.eleSubs[field.eleSubs.index(oldEle)] = newEle
        field.makeData()
        self.eleReplaced.emit(oldEle, newEle)   # 先通知视图记录新对象，再触发树重建
        self._afterStructChange()
        return newEle

    def removeField(self, field):
        if field in self.project.fields:
            self.project.fields.remove(field)
            self._afterStructChange()

    def removeEleSub(self, field, ele):
        if ele in field.eleSubs:
            field.eleSubs.remove(ele)
            field.makeData()
            self._afterStructChange()

    def _afterStructChange(self):
        self.markDirty()
        self.structureChanged.emit()
