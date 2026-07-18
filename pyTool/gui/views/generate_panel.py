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

# 生成面板：mode/mainMode/outPath/sln_cmake_path + 生成按钮 + 日志
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QFormLayout, QComboBox, QLineEdit,
    QPushButton, QPlainTextEdit, QFileDialog, QLabel,
)

from services import generate


class GeneratePanel(QWidget):
    """底部生成参数面板与日志显示。"""

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        layout = QVBoxLayout(self)

        formRow = QFormLayout()
        self._mode = QComboBox()
        self._mode.addItems(["new（新解决方案）", "add（追加到现有 CMake）"])
        self._mainMode = QComboBox()
        self._mainMode.addItems(["0 makeData", "1 GiD 文件"])
        self._outPath = QLineEdit()
        self._outPathBtn = QPushButton("浏览…")
        self._slnPath = QLineEdit()

        pathRow = QHBoxLayout()
        pathRow.addWidget(self._outPath)
        pathRow.addWidget(self._outPathBtn)

        formRow.addRow("生成模式", self._mode)
        formRow.addRow("主函数模式", self._mainMode)
        formRow.addRow("输出路径", pathRow)
        formRow.addRow("解决方案 CMake", self._slnPath)
        layout.addLayout(formRow)

        btnRow = QHBoxLayout()
        self._btnGen = QPushButton("▶ 生成")
        btnRow.addWidget(self._btnGen)
        btnRow.addStretch(1)
        layout.addLayout(btnRow)

        layout.addWidget(QLabel("日志"))
        self._log = QPlainTextEdit()
        self._log.setReadOnly(True)
        layout.addWidget(self._log)

        # 初始：new 模式不启用 slnPath
        self._slnPath.setEnabled(False)
        self._mode.currentIndexChanged.connect(self._onModeChanged)
        self._outPathBtn.clicked.connect(self._pickOutPath)
        self._btnGen.clicked.connect(self._doGenerate)

    def setDefaults(self, outPath: str, sln_cmake_path: str):
        if not self._outPath.text():
            self._outPath.setText(outPath)
        if not self._slnPath.text():
            self._slnPath.setText(sln_cmake_path)

    def _onModeChanged(self, _):
        self._slnPath.setEnabled(self._mode.currentIndex() == 1)

    def _pickOutPath(self):
        d = QFileDialog.getExistingDirectory(self, "选择输出目录")
        if d:
            self._outPath.setText(d)

    def _doGenerate(self):
        proj = self._model.project
        mode = "new" if self._mode.currentIndex() == 0 else "add"
        mainMode = self._mainMode.currentIndex()
        outPath = self._outPath.text().strip()
        if not outPath:
            self._log.appendPlainText("[错误] 输出路径不能为空")
            return
        sln = self._slnPath.text().strip() if mode == "add" else None
        self._log.appendPlainText("——— 开始生成 ———")
        ok, _ = generate.run(proj, mode, mainMode, outPath, sln_cmake_path=sln,
                             log=lambda text: self._log.appendPlainText(text.rstrip()))
        self._log.appendPlainText("✅ 生成完成" if ok else "❌ 生成失败（详见上方日志）")
