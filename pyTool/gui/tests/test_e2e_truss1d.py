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
from services import json_io, generate


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
    """GUI 全链路（json_io round-trip → generate.run）与脚本路径产物逐字节一致。"""
    proj_for_gui = _buildTruss1DProject()
    proj_for_script = _buildTruss1DProject()

    gui_dir = str(tmp_path / "gui_out")
    script_dir = str(tmp_path / "script_out")

    # GUI 路径：模拟 GUI 配置结果 → 存档 → 加载 → 生成（GUI 真实链路）
    tmp_json = str(tmp_path / "truss.cdfeg.json")
    json_io.save(proj_for_gui, tmp_json)
    proj_reloaded = json_io.load(tmp_json)
    generate.run(proj_reloaded, mode="new", mainMode=0, outPath=gui_dir, log=lambda *_: None)

    # 脚本路径：照搬 test1DTruss.py 的调用
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
