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

# AddEleSubDialog 测试
from dialogs.add_ele_dialog import AddEleSubDialog


def test_add_ele_dialog_empty_name_returns_none(qapp):
    dlg = AddEleSubDialog()
    assert dlg.values() is None          # 默认名称为空 → None


def test_add_ele_dialog_default_plain(qapp):
    dlg = AddEleSubDialog()
    dlg._nameEdit.setText("Bar")
    assert dlg.values() == ("Bar", False)   # 默认普通单元


def test_add_ele_dialog_gauss_when_selected(qapp):
    dlg = AddEleSubDialog()
    dlg._nameEdit.setText("Q4")
    dlg._typeCombo.setCurrentIndex(1)
    assert dlg.values() == ("Q4", True)
