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

# generate 参数捕获测试（monkeypatch makeAll，不真生成文件）
from DataProject import DataProject
from services import generate


def _stub_maker(self_cls_name):
    """返回一个假 Maker 类，记录构造参数与 makeAll 调用。"""
    instances = []

    class _Stub:
        mainMode = 0

        def __init__(self, *args, **kwargs):
            self.args = args
            self.kwargs = kwargs
            self.madeAll = False
            instances.append(self)

        def makeAll(self):
            self.madeAll = True
            print(f"[stub {self_cls_name}] makeAll called")

    return _Stub, instances


def test_run_new_mode_calls_maker_cpp_with_correct_args(monkeypatch):
    cpp_stub, cpp_inst = _stub_maker("MakerCpp")
    gid_stub, gid_inst = _stub_maker("MakerGidFile")
    monkeypatch.setattr(generate, "MakerCpp", cpp_stub)
    monkeypatch.setattr(generate, "MakerGidFile", gid_stub)

    proj = DataProject("Truss1D", 1)
    logs = []
    ok, _ = generate.run(proj, mode="new", mainMode=0,
                         outPath="out/truss", log=logs.append)

    assert ok is True
    assert len(cpp_inst) == 1
    assert cpp_inst[0].kwargs["mode"] == "new"
    assert cpp_inst[0].args[1] == "out/truss"
    assert cpp_inst[0].mainMode == 0
    assert cpp_inst[0].madeAll is True
    # mainMode=0 不应调 MakerGidFile
    assert gid_inst == []


def test_run_mainmode1_calls_gid_maker(monkeypatch):
    cpp_stub, _ = _stub_maker("MakerCpp")
    gid_stub, gid_inst = _stub_maker("MakerGidFile")
    monkeypatch.setattr(generate, "MakerCpp", cpp_stub)
    monkeypatch.setattr(generate, "MakerGidFile", gid_stub)

    proj = DataProject("El2D", 2)
    ok, _ = generate.run(proj, mode="add", mainMode=1,
                         outPath="sample/El2D",
                         sln_cmake_path="FEMproject/CMakeLists.txt",
                         log=lambda *_: None)
    assert ok is True
    assert len(gid_inst) == 1
    assert gid_inst[0].madeAll is True


def test_run_swallows_exception_and_returns_false(monkeypatch):
    class _Boom:
        mainMode = 0

        def __init__(self, *a, **k):
            pass

        def makeAll(self):
            raise RuntimeError("故意失败")

    monkeypatch.setattr(generate, "MakerCpp", _Boom)
    monkeypatch.setattr(generate, "MakerGidFile",
                        type("G", (), {"__init__": lambda self, *a, **k: None,
                                       "makeAll": lambda self: None}))
    logs = []
    ok, txt = generate.run(DataProject("X", 2), "new", 0, "out/x", log=logs.append)
    assert ok is False
    assert "故意失败" in txt
