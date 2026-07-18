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

import filecmp
import os

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from MakerCpp import MakerCpp


def _buildTruss1DProject() -> DataProject:
    """复刻 test/test1DTruss.py 的数据结构。"""
    project = DataProject("Truss1D", 1)
    field = DataField("Truss1DDisp")
    ele = DataEleSub("Truss1D")
    ele.dispNames = ["u"]
    ele.eleResNames = ["T"]
    ele.paramNames = ["E", "A"]
    field.addEleSub(ele)
    project.addField(field)
    project.cmds.append(("imp", 0))
    return project


def _projectFiles(root):
    """收集项目代码文件（排除复制的 CDFEG/ 与 third/ 库）。"""
    out = {}
    for dirpath, _, files in os.walk(root):
        rel = os.path.relpath(dirpath, root)
        if rel.startswith("CDFEG") or rel.startswith("third"):
            continue
        for fn in files:
            p = os.path.join(dirpath, fn)
            out[os.path.relpath(p, root).replace("\\", "/")] = p
    return out


def test_gui_path_matches_script_path(tmp_path):
    proj_for_gui = _buildTruss1DProject()      # GUI 路径用相同数据
    proj_for_script = _buildTruss1DProject()   # 脚本路径用相同数据

    gui_dir = str(tmp_path / "gui_out")
    script_dir = str(tmp_path / "script_out")

    # GUI 路径：直接经 MakerCpp（generate.run 内部即此调用）
    mk = MakerCpp(proj_for_gui, gui_dir, mode="new")
    mk.mainMode = 0
    mk.makeAll()

    # 脚本路径：完全照搬 test1DTruss.py 的调用
    mk2 = MakerCpp(proj_for_script, script_dir, mode="new")
    mk2.mainMode = 0
    mk2.makeAll()

    gui_files = _projectFiles(gui_dir)
    script_files = _projectFiles(script_dir)
    assert set(gui_files.keys()) == set(script_files.keys()), \
        f"文件列表不一致:\n仅GUI:{set(gui_files)-set(script_files)}\n仅脚本:{set(script_files)-set(gui_files)}"

    diffs = []
    for rel, gp in gui_files.items():
        if not filecmp.cmp(gp, script_files[rel], shallow=False):
            diffs.append(rel)
    assert diffs == [], f"以下文件内容不一致: {diffs}"
