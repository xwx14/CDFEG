# SPDX-License-Identifier: GPL-3.0
# pyTool GUI 主窗口（Task 1 骨架，Task 10 填充内容）
from PySide6.QtWidgets import QMainWindow


class MainWindow(QMainWindow):
    """pyTool 配置式生成器主窗口。"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("pyTool 配置式生成器")
        self.resize(1100, 760)
