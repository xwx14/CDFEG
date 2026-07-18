# SPDX-License-Identifier: GPL-3.0
# pytest 公共配置：注入 pyTool 根到 sys.path、offscreen Qt、QApplication fixture
import os
import sys

# 让 tests/ 能 import 到 pyTool/gui/ 与 pyTool/ 根的模块
_GUI_DIR = os.path.dirname(os.path.abspath(__file__))          # .../pyTool/gui
_PYTOOL_ROOT = os.path.dirname(_GUI_DIR)                       # .../pyTool
for _p in (_GUI_DIR, _PYTOOL_ROOT):
    if _p not in sys.path:
        sys.path.insert(0, _p)

# GUI 测试必须 headless
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PySide6.QtWidgets import QApplication


@pytest.fixture(scope="session")
def qapp():
    """会话级单例 QApplication（offscreen）。"""
    app = QApplication.instance() or QApplication(sys.argv)
    yield app
