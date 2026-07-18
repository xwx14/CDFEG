# SPDX-License-Identifier: GPL-3.0
# pyTool GUI 骨架冒烟测试
from main_window import MainWindow


def test_main_window_can_construct(qapp):
    """空 MainWindow 必须能被构造，且窗口标题含中文标识。"""
    win = MainWindow()
    assert win is not None
    assert "pyTool" in win.windowTitle()
    win.deleteLater()
