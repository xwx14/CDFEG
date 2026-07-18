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

# generate：封装 MakerCpp + MakerGidFile，重定向 stdout 到日志回调，异常不上抛。
import contextlib
import io
import traceback

from MakerCpp import MakerCpp          # 模块级 import，便于测试 monkeypatch
from MakerGidFile import MakerGidFile


def run(project, mode, mainMode, outPath, sln_cmake_path=None, log=print):
    """
    调用代码生成器。

    Args:
        project: DataProject
        mode: 'new' | 'add'
        mainMode: 0=makeData | 1=GiD（1 时额外生成 GiD 文件）
        outPath: 输出目录
        sln_cmake_path: mode='add' 时的解决方案 CMake 路径
        log: 日志回调（默认 print）

    Returns:
        (ok: bool, logText: str)
    """
    buf = io.StringIO()
    try:
        with contextlib.redirect_stdout(buf):
            maker = MakerCpp(project, outPath, mode=mode,
                             sln_cmake_path=sln_cmake_path)
            maker.mainMode = mainMode
            maker.makeAll()
            if mainMode == 1:
                MakerGidFile(project, outPath).makeAll()
        text = buf.getvalue()
        log(text)
        return True, text
    except Exception:
        text = buf.getvalue() + "\n[生成失败]\n" + traceback.format_exc()
        log(text)
        return False, text
