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
