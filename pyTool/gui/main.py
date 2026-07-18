# SPDX-License-Identifier: GPL-3.0
# pyTool GUI 运行入口
import os
import sys

# 注入 pyTool 根与 gui 目录到 sys.path
_HERE = os.path.dirname(os.path.abspath(__file__))
_PYTOOL_ROOT = os.path.dirname(_HERE)
for _p in (_HERE, _PYTOOL_ROOT):
    if _p not in sys.path:
        sys.path.insert(0, _p)

from PySide6.QtWidgets import QApplication
from main_window import MainWindow


def main():
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
