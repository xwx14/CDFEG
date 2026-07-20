# 树视窗按钮栏 + 广义位移/单元变量单行逗号输入 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 pyTool GUI 左侧项目树下方新增横向按钮栏（添加场/添加单元/删除，按选中节点动态启用），并将单元面板的「广义位移」「单元变量」由 ListEditor 改为单行逗号输入框（支持中英文逗号、带占位提示）。

**Architecture:** 新建可复用组件 `CsvLineEdit`（接口与现有 `ListEditor` 对齐：`setItems/items/itemsChanged`），ElementPanel 仅替换两个实例化、其余不动；MainWindow 将左栏由「裸树」改为「容器(树 + 横向按钮栏)」，按钮复用现有 `_addField/_addEleSub/_deleteSelected` slot，新增 `_updateActionButtons` 依选中节点类型动态启用。不触动 model 层、services、生成器与模板。

**Tech Stack:** Python 3、PySide6（Qt6）、pytest（`qapp` fixture + `QT_QPA_PLATFORM=offscreen`）。

**Spec:** `docs/superpowers/specs/2026-07-20-gui-tree-buttons-csv-input-design.md`

## Global Constraints

- **框架**：PySide6；`from PySide6.QtWidgets/QCore import ...`；信号用 `Signal`。
- **界面语言**：简体中文（控件文本）；代码注释中文（与现有 `pyTool/gui/` 一致）。
- **命名**：类名/方法名驼峰（`CsvLineEdit`/`setItems`/`_updateActionButtons`），文件名/测试函数 snake_case，Qt 信号驼峰（`itemsChanged`）。
- **模块导入**：`pyTool/gui/` 下代码用 `from widgets.xxx import` / `from views.xxx import`（`conftest.py` 与 `main.py` 已注入 `pyTool/gui` 与 `pyTool` 根到 `sys.path`）。
- **测试运行**：`cd pyTool/gui && python -m pytest tests/<file>.py -v`；所有 QWidget 测试由 `conftest.py` 设 offscreen。
- **GPL 头**：新增 `.py` 文件须带 SPDX-License-Identifier: GPL-3.0 头（照搬 `widgets/list_editor.py` 头部）。
- **逗号解析**：仅英文 `,` 与中文全角 `，`；**不含分号**（YAGNI）；`re.split(r"[，,]", text)` → 各项 `strip()` → 过滤空串；保序。
- **提交**：每个任务结束 commit；中文 conventional 信息（如 `feat(pyTool-gui): ...`）；只提交本任务相关文件。

---

## File Structure

| 文件 | 职责 | 任务 |
|---|---|---|
| `pyTool/gui/widgets/csv_line_edit.py` | `CsvLineEdit(QWidget)`：单行逗号输入，中英文逗号解析，接口对齐 ListEditor | Task 1（新建） |
| `pyTool/gui/views/element_panel.py` | 「广义位移」「单元变量」两个 ListEditor 实例换 CsvLineEdit（含 placeholder） | Task 2（修改） |
| `pyTool/gui/main_window.py` | 左栏容器化（树 + 横向按钮栏）、按钮信号、`_updateActionButtons` 动态启用 | Task 3（修改） |
| `pyTool/gui/tests/test_widgets.py` | 追加 CsvLineEdit 用例 | Task 1 |
| `pyTool/gui/tests/test_panels.py` | 追加 ElementPanel 中文逗号写回用例 | Task 2 |
| `pyTool/gui/tests/test_skeleton.py` | 追加按钮栏存在性与动态启用状态用例 | Task 3 |

`widgets/list_editor.py` **保留不动**（高斯区「权重」「形函数」仍用）。

---

## Task 1: CsvLineEdit 可复用单行逗号输入组件

**目标**：新建 `CsvLineEdit`，接口与 `ListEditor` 完全对齐（`setItems/items/itemsChanged`），内部 `QLineEdit` + 中英文逗号解析；`setItems` 不触发 `itemsChanged`（与 ListEditor 一致，避免回填误触发 commit）。

**Files:**
- Create: `pyTool/gui/widgets/csv_line_edit.py`
- Modify: `pyTool/gui/tests/test_widgets.py`（末尾追加 CsvLineEdit 用例）

**Interfaces:**
- Produces（供 Task 2 使用）：
  - `CsvLineEdit(label: str = "", placeholder: str = "", parent=None)`
  - `csvLineEdit.setItems(items: list) -> None`（展示为 `", ".join`，**不发 itemsChanged**）
  - `csvLineEdit.items() -> list[str]`（按 `[，,]` 切分，strip，过滤空串，保序）
  - `csvLineEdit.itemsChanged = Signal()`（仅 `QLineEdit.editingFinished` 即失焦/回车时发射）
  - 内部属性 `csvLineEdit._edit`（`QLineEdit`，供测试 setText/触发 editingFinished）

- [ ] **Step 1: 写失败测试**

在 `pyTool/gui/tests/test_widgets.py` 末尾追加：
```python
from widgets.csv_line_edit import CsvLineEdit


def test_csv_line_edit_roundtrip(qapp):
    le = CsvLineEdit("广义位移")
    le.setItems(["u", "v", "w"])
    assert le.items() == ["u", "v", "w"]


def test_csv_line_edit_chinese_comma(qapp):
    le = CsvLineEdit()
    le._edit.setText("u，v，w")  # 中文全角逗号
    assert le.items() == ["u", "v", "w"]


def test_csv_line_edit_filters_empty(qapp):
    le = CsvLineEdit()
    le._edit.setText("u,, v ,")  # 连续逗号 / 首尾逗号 / 空白
    assert le.items() == ["u", "v"]
    le._edit.setText("")
    assert le.items() == []


def test_csv_line_edit_setitems_does_not_emit(qapp):
    le = CsvLineEdit()
    hits = []
    le.itemsChanged.connect(lambda: hits.append(1))
    le.setItems(["a", "b"])
    assert hits == []  # setItems 不触发（与 ListEditor 一致）


def test_csv_line_edit_editingfinished_emits(qapp):
    le = CsvLineEdit()
    hits = []
    le.itemsChanged.connect(lambda: hits.append(1))
    le._edit.setText("a, b")
    le._edit.editingFinished.emit()  # 模拟失焦/回车
    assert len(hits) >= 1
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_widgets.py -v -k csv`
Expected: FAIL（`No module named 'widgets.csv_line_edit'`），原 ListEditor/TableEditor 用例仍 PASS。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/widgets/csv_line_edit.py`：
```python
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
# CsvLineEdit：单行逗号输入编辑器（中英文逗号均可），接口对齐 ListEditor
import re

from PySide6.QtCore import Signal
from PySide6.QtWidgets import QWidget, QVBoxLayout, QLabel, QLineEdit


class CsvLineEdit(QWidget):
    """单行逗号分隔输入；setItems/items/itemsChanged 与 ListEditor 同义。"""

    itemsChanged = Signal()

    # 仅英文逗号与中文全角逗号；不含分号（需求仅逗号）
    _SPLIT_RE = re.compile(r"[，,]")

    def __init__(self, label: str = "", placeholder: str = "", parent=None):
        super().__init__(parent)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        if label:
            layout.addWidget(QLabel(label))

        self._edit = QLineEdit()
        self._edit.setPlaceholderText(placeholder or "逗号分隔，如：u, v, w")
        layout.addWidget(self._edit)

        # editingFinished（失焦/回车）才发信号，与 ElementPanel 现有 QLineEdit 风格一致；
        # programmatic setText 不触发 editingFinished，故 setItems 不会误触发。
        self._edit.editingFinished.connect(self.itemsChanged.emit)

    def setItems(self, items) -> None:
        # 统一以「英文逗号 + 空格」展示；不主动 emit，与 ListEditor 行为一致。
        self._edit.setText(", ".join(str(x) for x in items))

    def items(self) -> list:
        parts = self._SPLIT_RE.split(self._edit.text())
        return [p.strip() for p in parts if p.strip()]
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_widgets.py -v`
Expected: PASS（11 passed：原 3 ListEditor + 3 TableEditor + 5 CsvLineEdit）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/widgets/csv_line_edit.py pyTool/gui/tests/test_widgets.py
git commit -m "feat(pyTool-gui): 新增 CsvLineEdit 单行逗号输入组件（中英文逗号）"
```

---

## Task 2: ElementPanel 集成 CsvLineEdit

**目标**：把 ElementPanel 的「广义位移」「单元变量」两个 `ListEditor` 实例换为 `CsvLineEdit`（带 placeholder）。信号连接、`setItems/items` 调用、`_commit` 全部兼容，不改。`ListEditor` import 保留（高斯区仍用）。

**Files:**
- Modify: `pyTool/gui/views/element_panel.py`（import 行 + 第 67-68 行两个实例化）
- Modify: `pyTool/gui/tests/test_panels.py`（末尾追加中文逗号写回用例）

**Interfaces:**
- Consumes: `CsvLineEdit`（Task 1）。
- Produces：ElementPanel 的 `self._dispNames`/`self._eleResNames` 类型由 `ListEditor` 变为 `CsvLineEdit`；对外方法签名（`loadEleSub`/`_commit`）不变。

- [ ] **Step 1: 写失败测试**

在 `pyTool/gui/tests/test_panels.py` 末尾追加：
```python
from widgets.csv_line_edit import CsvLineEdit


def test_element_panel_csv_input_writes_back(qapp):
    """广义位移/单元变量为 CsvLineEdit，中文逗号输入写回为 list。"""
    m = ProjectModel()
    f = m.addField("F")
    ele = m.addEleSub(f, "Truss", gauss=False)
    m.markClean()
    ep = ElementPanel(m)
    ep.loadEleSub(f, ele)
    # 已换成 CsvLineEdit
    assert isinstance(ep._dispNames, CsvLineEdit)
    assert isinstance(ep._eleResNames, CsvLineEdit)
    # 中文逗号输入广义位移 → 写回 ele.dispNames
    ep._dispNames._edit.setText("u，v")
    ep._dispNames._edit.editingFinished.emit()  # 触发 _commit
    assert ele.dispNames == ["u", "v"]
    # 英文逗号输入单元变量
    ep._eleResNames._edit.setText("sx, sy, sxy")
    ep._eleResNames._edit.editingFinished.emit()
    assert ele.eleResNames == ["sx", "sy", "sxy"]
    assert m.isDirty is True


def test_element_panel_csv_roundtrip_on_reload(qapp):
    """已存的 dispNames 在 loadEleSub 时正确回填到单行框。"""
    m = ProjectModel()
    f = m.addField("F")
    ele = m.addEleSub(f, "T", gauss=False)
    ele.dispNames = ["u", "v", "w"]
    ep = ElementPanel(m)
    ep.loadEleSub(f, ele)
    assert ep._dispNames._edit.text() == "u, v, w"
    assert ep._dispNames.items() == ["u", "v", "w"]
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_panels.py -v -k csv`
Expected: FAIL（`AssertionError`：`isinstance(ep._dispNames, CsvLineEdit)` 为 False，因当前仍是 ListEditor）。

- [ ] **Step 3: 改 import**

修改 `pyTool/gui/views/element_panel.py`，在现有 `from widgets.list_editor import ListEditor` 之后追加一行（`ListEditor` 保留，高斯区仍用）：

原（约第 23-25 行）：
```python
from DataEleSubG import DataEleSubG
from widgets.list_editor import ListEditor
from widgets.table_editor import TableEditor
```
改为：
```python
from DataEleSubG import DataEleSubG
from widgets.list_editor import ListEditor
from widgets.csv_line_edit import CsvLineEdit
from widgets.table_editor import TableEditor
```

- [ ] **Step 4: 换两个实例化**

修改 `pyTool/gui/views/element_panel.py` 第 67-68 行：

原：
```python
        self._dispNames = ListEditor("广义位移")
        self._eleResNames = ListEditor("单元变量")
```
改为：
```python
        self._dispNames = CsvLineEdit("广义位移", placeholder="逗号分隔，如：u, v, w")
        self._eleResNames = CsvLineEdit("单元变量", placeholder="逗号分隔，如：sx, sy, sxy")
```

> 其余不动：`itemsChanged` 信号连接（第 114-115 行）、`loadEleSub` 中 `setItems`（第 134-135 行）、`_commit` 中 `items()`（第 165-166 行）对 CsvLineEdit 同样适用。

- [ ] **Step 5: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_panels.py -v`
Expected: PASS（7 passed：原 5 + 新增 2）。原 ElementPanel 用例（普通/高斯/参数表/高斯 commit 过滤）不受影响。

- [ ] **Step 6: 提交**

```bash
git add pyTool/gui/views/element_panel.py pyTool/gui/tests/test_panels.py
git commit -m "feat(pyTool-gui): 广义位移/单元变量改用 CsvLineEdit 单行逗号输入"
```

---

## Task 3: MainWindow 左栏按钮栏 + 动态启用

**目标**：将左栏由「裸树」改为「容器(树 + 横向按钮栏)」，按钮栏含「添加场/添加单元/删除」，复用现有 slot；新增 `_updateActionButtons` 依选中节点类型动态启用/禁用；在选中变化与树刷新时调用。

**Files:**
- Modify: `pyTool/gui/main_window.py`（import、`__init__` 左栏与信号、新增 `_updateActionButtons`、`_onTreeCurrentChanged` 与 `refreshTree` 调用点）
- Modify: `pyTool/gui/tests/test_skeleton.py`（末尾追加按钮栏用例）

**Interfaces:**
- Consumes：现有 `_addField/_addEleSub/_deleteSelected` slot（不改）。
- Produces：MainWindow 新增属性 `self._btnAddField / self._btnAddEle / self._btnDelete`（`QPushButton`）；新方法 `_updateActionButtons()`。

**动态启用规则**（依选中节点 `kind`）：
| 按钮 | project 根 | field / ele | 无选中 |
| --- | --- | --- | --- |
| 添加场 | 启用 | 启用 | 启用 |
| 添加单元 | 禁用 | 启用 | 禁用 |
| 删除 | 禁用 | 启用 | 禁用 |

- [ ] **Step 1: 写失败测试**

在 `pyTool/gui/tests/test_skeleton.py` 末尾追加：
```python
def test_main_window_left_button_bar_exists(qapp):
    """左栏有横向按钮栏：添加场/添加单元/删除。"""
    win = MainWindow()
    assert win._btnAddField.text() == "添加场"
    assert win._btnAddEle.text() == "添加单元"
    assert win._btnDelete.text() == "删除"
    win.deleteLater()


def test_main_window_buttons_initial_state(qapp):
    """初始选中项目根：仅添加场启用，添加单元/删除禁用。"""
    win = MainWindow()
    win.newProject()
    win.refreshTree()
    win._tree.setCurrentItem(win._tree.topLevelItem(0))  # 项目根
    assert win._btnAddField.isEnabled() is True
    assert win._btnAddEle.isEnabled() is False
    assert win._btnDelete.isEnabled() is False
    win.deleteLater()


def test_main_window_buttons_enable_on_field_select(qapp):
    """选中场节点后，添加单元/删除启用。"""
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    win._model.addEleSub(f, "T", gauss=False)
    win.refreshTree()
    root = win._tree.topLevelItem(0)
    field_node = root.child(0)
    win._tree.setCurrentItem(field_node)
    assert win._btnAddField.isEnabled() is True
    assert win._btnAddEle.isEnabled() is True
    assert win._btnDelete.isEnabled() is True
    win.deleteLater()


def test_main_window_buttons_enable_on_ele_select(qapp):
    """选中单元叶节点后，添加单元/删除同样启用（回溯到所属场）。"""
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    win._model.addEleSub(f, "T", gauss=False)
    win.refreshTree()
    root = win._tree.topLevelItem(0)
    field_node = root.child(0)
    ele_node = field_node.child(0)
    win._tree.setCurrentItem(ele_node)
    assert win._btnAddEle.isEnabled() is True
    assert win._btnDelete.isEnabled() is True
    win.deleteLater()
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_skeleton.py -v -k button`
Expected: FAIL（`AttributeError: 'MainWindow' object has no attribute '_btnAddField'`）。

- [ ] **Step 3: 改 import（补 QHBoxLayout、QPushButton）**

修改 `pyTool/gui/main_window.py` 第 18-21 行：

原：
```python
from PySide6.QtWidgets import (
    QMainWindow, QWidget, QSplitter, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QVBoxLayout, QFileDialog, QMessageBox, QInputDialog,
)
```
改为：
```python
from PySide6.QtWidgets import (
    QMainWindow, QWidget, QSplitter, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QFileDialog, QMessageBox, QInputDialog,
)
```

- [ ] **Step 4: 改 `__init__` 左栏为「树 + 按钮栏」容器**

修改 `pyTool/gui/main_window.py`。原第 45-48 行：

```python
        # ---- 左：树 ----
        self._tree = QTreeWidget()
        self._tree.setHeaderLabels(["项目结构"])
        self._tree.setContextMenuPolicy(Qt.CustomContextMenu)
```

改为：

```python
        # ---- 左：树 + 按钮栏 ----
        self._tree = QTreeWidget()
        self._tree.setHeaderLabels(["项目结构"])
        self._tree.setContextMenuPolicy(Qt.CustomContextMenu)

        self._btnAddField = QPushButton("添加场")
        self._btnAddEle = QPushButton("添加单元")
        self._btnDelete = QPushButton("删除")
        btnBar = QHBoxLayout()
        for b in (self._btnAddField, self._btnAddEle, self._btnDelete):
            btnBar.addWidget(b)
        btnBar.addStretch(1)

        leftPanel = QWidget()
        leftLayout = QVBoxLayout(leftPanel)
        leftLayout.setContentsMargins(0, 0, 0, 0)
        leftLayout.addWidget(self._tree)
        leftLayout.addLayout(btnBar)
```

- [ ] **Step 5: splitter 改挂 leftPanel**

修改 `pyTool/gui/main_window.py`。原第 59-63 行：

```python
        splitter = QSplitter(Qt.Horizontal)
        splitter.addWidget(self._tree)
        splitter.addWidget(self._stack)
        splitter.setStretchFactor(0, 1)
        splitter.setStretchFactor(1, 3)
```

改为：

```python
        splitter = QSplitter(Qt.Horizontal)
        splitter.addWidget(leftPanel)
        splitter.addWidget(self._stack)
        splitter.setStretchFactor(0, 1)
        splitter.setStretchFactor(1, 3)
```

- [ ] **Step 6: 连接按钮信号**

修改 `pyTool/gui/main_window.py`。原第 75-77 行：

```python
        self._tree.currentItemChanged.connect(self._onTreeCurrentChanged)
        self._model.dirtyChanged.connect(self._updateTitle)
        self._model.structureChanged.connect(self.refreshTree)
```

改为：

```python
        self._tree.currentItemChanged.connect(self._onTreeCurrentChanged)
        self._model.dirtyChanged.connect(self._updateTitle)
        self._model.structureChanged.connect(self.refreshTree)

        # 按钮栏：复用「编辑」菜单的 slot
        self._btnAddField.clicked.connect(self._addField)
        self._btnAddEle.clicked.connect(self._addEleSub)
        self._btnDelete.clicked.connect(self._deleteSelected)
```

- [ ] **Step 7: 新增 `_updateActionButtons` 方法**

在 `pyTool/gui/main_window.py` 的 `_onTreeCurrentChanged` 方法**之前**插入新方法：

```python
    def _updateActionButtons(self):
        """依选中节点类型动态启用/禁用左栏按钮。"""
        item = self._tree.currentItem()
        kind = None
        if item is not None:
            data = item.data(0, Qt.UserRole)
            if data:
                kind = data[0]
        hasFieldCtx = kind in ("field", "ele")
        self._btnAddField.setEnabled(True)
        self._btnAddEle.setEnabled(hasFieldCtx)
        self._btnDelete.setEnabled(hasFieldCtx)

```

- [ ] **Step 8: 在 `_onTreeCurrentChanged` 开头调用**

修改 `pyTool/gui/main_window.py` 的 `_onTreeCurrentChanged`。原：

```python
    def _onTreeCurrentChanged(self, cur, _prev):
        if cur is None:
            return
```

改为：

```python
    def _onTreeCurrentChanged(self, cur, _prev):
        self._updateActionButtons()
        if cur is None:
            return
```

> 放在 early return 之前，确保选中清空（cur 为 None）时按钮也刷新到「无选中」状态。

- [ ] **Step 9: 在 `refreshTree` 末尾调用**

修改 `pyTool/gui/main_window.py` 的 `refreshTree`。原末尾：

```python
        self._tree.addTopLevelItem(root)
        self._tree.expandAll()
        self._updateTitle()
```

改为：

```python
        self._tree.addTopLevelItem(root)
        self._tree.expandAll()
        self._updateTitle()
        self._updateActionButtons()
```

- [ ] **Step 10: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_skeleton.py -v`
Expected: PASS（7 passed：原 3 + 新增 4）。

- [ ] **Step 11: 全量回归**

Run: `cd pyTool/gui && python -m pytest -v`
Expected: 全部 PASS（widgets 11 + panels 7 + skeleton 7 + project_model + json_io + generate + generate_panel + e2e_truss1d，无失败、无 error）。

- [ ] **Step 12: 手动走查**

Run: `cd pyTool/gui && python main.py`
Expected：
1. 左侧项目树下方出现横向按钮栏 `[添加场][添加单元][删除]`，初始「添加单元」「删除」灰色禁用。
2. 点「添加场」→ 输入名 → 树出现场节点；选中该场节点后，「添加单元」「删除」转为启用。
3. 点「添加单元」→ 输入名 → 选高斯 → 树出现单元节点。
4. 选中单元节点，「添加单元」「删除」仍启用；选回项目根，二者再次禁用。
5. 选中单元节点，右侧面板「广义位移」「单元变量」为单行输入框，未输入时显示灰色占位提示「逗号分隔，如：u, v, w」。
6. 在「广义位移」输入 `u，v`（中文逗号），失焦后场聚合正确（切回场节点查看）；重新选中该单元，输入框回显 `u, v`。
7. 「编辑」菜单的「添加场/添加单元/删除选中」仍可用，与按钮行为一致。

- [ ] **Step 13: 提交**

```bash
git add pyTool/gui/main_window.py pyTool/gui/tests/test_skeleton.py
git commit -m "feat(pyTool-gui): 项目树下新增横向按钮栏并按选中节点动态启用"
```

---

## Self-Review 记录（plan 作者执行）

**1. Spec 覆盖**：逐条对照 spec——
- 按钮栏位置（左栏底部，树下方横向）→ Task 3 Step 4-5（leftPanel = 树 + btnBar）。
- 三按钮组成（添加场/添加单元/删除）→ Task 3 Step 4。
- 复用现有 slot、菜单保留 → Task 3 Step 6（按钮连 `_addField/_addEleSub/_deleteSelected`）；菜单不动。
- 动态启用规则表（project/field/ele/无选中）→ Task 3 Step 7 `_updateActionButtons` + Step 8-9 调用点；测试 Step 1 四个用例覆盖。
- CsvLineEdit 组件（接口对齐 ListEditor）→ Task 1。
- ElementPanel 替换两实例 + placeholder → Task 2 Step 3-4。
- 中英文逗号解析（仅逗号，过滤空串，保序）→ Task 1 Step 3 `_SPLIT_RE = re.compile(r"[，,]")`。
- placeholder 文案 → Task 2 Step 4（`"逗号分隔，如：u, v, w"` / `"逗号分隔，如：sx, sy, sxy"`）。
- 测试计划（CsvLineEdit 单测 + main_window 动态启用 + panels 回归）→ Task 1/2/3。
- 影响面（不动 model/services/生成器/ListEditor）→ File Structure 注明 `list_editor.py` 保留。

**2. 占位符扫描**：无 TBD/TODO/「适当处理」；每步含实际代码或实际命令与预期输出。

**3. 类型一致性**：
- `CsvLineEdit` 方法（`setItems/items/itemsChanged`、`_edit`）在 Task 1 定义，Task 2 测试与 ElementPanel 使用一致。
- `_updateActionButtons`、`_btnAddField/_btnAddEle/_btnDelete` 在 Task 3 定义即用，测试一致引用。
- ElementPanel 改动后 `_dispNames/_eleResNames` 仍为同名属性（类型变 CsvLineEdit），`loadEleSub`/`_commit` 中的 `setItems/items/itemsChanged` 调用对 CsvLineEdit 同样成立（接口对齐）。
