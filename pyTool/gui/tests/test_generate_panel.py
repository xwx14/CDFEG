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

# GeneratePanel 测试
from DataProject import DataProject
from models.project_model import ProjectModel
from views import generate_panel as gp_mod
from views.generate_panel import GeneratePanel


def test_add_mode_enables_sln_path(qapp):
    m = ProjectModel.fromProject(DataProject("X", 2))
    p = GeneratePanel(m)
    p._mode.setCurrentIndex(1)  # add
    assert p._slnPath.isEnabled() is True
    p._mode.setCurrentIndex(0)  # new
    assert p._slnPath.isEnabled() is False


def test_generate_invokes_service_and_logs(monkeypatch, qapp):
    m = ProjectModel.fromProject(DataProject("X", 2))
    p = GeneratePanel(m)
    captured = {}

    def fake_run(project, mode, mainMode, outPath, sln_cmake_path=None, log=print):
        captured.update(project=project, mode=mode, mainMode=mainMode,
                        outPath=outPath, sln_cmake_path=sln_cmake_path)
        log("模拟生成成功")
        return True, "模拟生成成功"

    monkeypatch.setattr(gp_mod, "generate", type("G", (), {"run": staticmethod(fake_run)}))
    p._outPath.setText("out/x")
    p._doGenerate()
    assert captured["mode"] == "new"
    assert captured["outPath"] == "out/x"
    assert "模拟生成成功" in p._log.toPlainText()
