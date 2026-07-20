# GUI 改善：树视窗按钮栏 + 广义位移/单元变量单行逗号输入

- 日期：2026-07-20
- 状态：设计待审阅
- 关联：`docs/superpowers/specs/2026-07-18-pytool-gui-design.md`（GUI 初始设计）

## 1. 背景与目标

pyTool GUI（`pyTool/gui`）目前存在两处可用性短板，本次改善：

1. 「添加场」「添加单元」入口仅存在于菜单栏「编辑」菜单，用户在左侧项目树操作时需反复回到顶部菜单，路径长。
2. 「广义位移」「单元变量」采用 `ListEditor`（垂直列表 + 增删/上下移按钮），对短小的名称列表（如 `u, v, w`）操作过重。

目标：

1. 在左侧项目树视窗下方新增按钮栏（横向），承载「添加场」「添加单元」「删除」三个操作。
2. 将「广义位移」「单元变量」改为单行逗号输入框，带占位提示，内部解析同时支持英文 `,` 与中文 `，`。

## 2. 现状（基线）

- `main_window.py`
  - 「添加场」「添加单元」「删除选中」为「编辑」菜单项（`main_window.py:96-98`），slot 为 `_addField / _addEleSub / _deleteSelected`。
  - 左侧 `_tree` 直接作为 `QSplitter` 第一 widget（`main_window.py:60`）。
  - `_addEleSub` 依赖 `_selectedField()`：未选中场节点时弹 `QMessageBox` 提示（`main_window.py:168-179`）。
- `element_panel.py`
  - 「广义位移」「单元变量」用 `ListEditor`（`element_panel.py:67-68`），信号 `itemsChanged → _commit`。
  - `ListEditor` 同时被高斯区「权重」「形函数」复用（`element_panel.py:94,97`），**该类本次保留不动**。
- `widgets/list_editor.py`：`ListEditor` 接口为 `setItems(list) / items() -> list / itemsChanged` 信号。

## 3. 设计

### 3.1 左栏容器化与按钮栏（`main_window.py`）

将原「splitter 直接装 `_tree`」改为「左栏容器装 树 + 按钮栏」：

```
leftPanel = QWidget(QVBoxLayout)
  ├─ _tree                              (stretch=1)
  └─ btnBar  = QWidget(QHBoxLayout)
       ├─ btnAddField   [添加场]
       ├─ btnAddEle     [添加单元]
       ├─ btnDelete     [删除]
       └─ addStretch(1)                 # 按钮靠左
splitter.addWidget(leftPanel)           # 替换原 splitter.addWidget(_tree)
```

- 三按钮的 `clicked` 分别连接现有 `_addField / _addEleSub / _deleteSelected`，逻辑复用，不改。
- 「编辑」菜单项保留，与按钮共用同一 slot（多入口，不删菜单）。

### 3.2 按钮动态启用/禁用策略

新增 `_updateActionButtons()`，在以下时机调用：
- `_onTreeCurrentChanged`（选中变化时）
- `refreshTree` 末尾（结构重建后，含初始 `newProject`）

启用规则（依选中节点 `kind`）：

| 按钮 | project 根 | field 节点 | ele 节点 | 无选中 |
| --- | --- | --- | --- | --- |
| 添加场 | 启用 | 启用 | 启用 | 启用 |
| 添加单元 | 禁用 | 启用 | 启用 | 禁用 |
| 删除 | 禁用 | 启用 | 启用 | 禁用 |

> 「添加单元」在选中 `ele` 节点时也启用，因其通过 `_selectedField()` 可回溯到所属场（现有逻辑已支持，`main_window.py:202-205`）。

### 3.3 新组件 `CsvLineEdit`（`widgets/csv_line_edit.py`）

与 `ListEditor` / `TableEditor` 同级的可复用单行逗号输入组件，**接口对齐 `ListEditor`** 以最小化 ElementPanel 改动：

```python
class CsvLineEdit(QWidget):
    itemsChanged = Signal()
    def __init__(self, label="", placeholder="", parent=None): ...
    def setItems(self, items: list[str]) -> None: ...
    def items(self) -> list[str]: ...
```

实现要点：
- 内部：`QLabel`（标题，可选）+ `QLineEdit`，`QVBoxLayout`，无边距（与 `ListEditor` 一致 `setContentsMargins(0,0,0,0)`）。
- `setItems`：`", ".join(items)` 回填（统一以「英文逗号 + 空格」展示）。
- `items`：解析算法见 3.5。
- `QLineEdit.editingFinished` → 发 `itemsChanged`（与 `ListEditor` 同信号语义）。
  - 选用 `editingFinished`（失焦/回车触发）而非 `textChanged`（每次按键触发），避免输入中途频繁 `_commit`，与 `ElementPanel` 现有 `editingFinished` 风格一致（`element_panel.py:106-107`）。

### 3.4 ElementPanel 改动（`element_panel.py:67-68`）

仅替换两个实例化，信号连接与 `_commit` 调用不变：

```python
self._dispNames = CsvLineEdit("广义位移", placeholder="逗号分隔，如：u, v, w")
self._eleResNames = CsvLineEdit("单元变量", placeholder="逗号分隔，如：sx, sy, sxy")
```

- `_commit` 中 `ele.dispNames = self._dispNames.items()`、`ele.eleResNames = self._eleResNames.items()` 保持不变。
- `loadEleSub` 中 `self._dispNames.setItems(ele.dispNames)` 保持不变。
- 「位移与变量名」`QGroupBox` 内仍用 `QHBoxLayout` 横向放两者（布局不变）。

### 3.5 中英文逗号解析算法

```python
import re
def items(self) -> list[str]:
    raw = self._edit.text()
    parts = re.split(r"[，,]", raw)   # 仅英文 , 与中文 ，，不含分号
    return [p.strip() for p in parts if p.strip()]
```

- 分隔符：英文 `,` 与中文 `，`（全角）。**不含分号**（用户需求仅逗号，遵循 YAGNI）。
- 容错：连续逗号 `u,,v`、首尾逗号 `,u,v,`、纯空白项均被 `if p.strip()` 过滤。
- 顺序：`re.split` 保序，`dispNames` 顺序对应自由度编号，必须保序（满足）。

### 3.6 占位提示（placeholder）文案

| 字段 | placeholder |
| --- | --- |
| 广义位移 | `逗号分隔，如：u, v, w` |
| 单元变量 | `逗号分隔，如：sx, sy, sxy` |

## 4. 测试计划

- `widgets/csv_line_edit.py` 新增单测（`tests/test_widgets.py` 扩展或新建）：
  - `setItems(["u","v"]) → items() == ["u","v"]` 往返一致。
  - 中文逗号：`setItems` 后手动改文本为 `"u，v，w"` → `items() == ["u","v","w"]`。
  - 容错：`"u,, v ,"` → `["u","v"]`；空串 `""` → `[]`。
  - `itemsChanged` 在 `editingFinished` 时发射。
- `main_window` 测试（`tests/` 扩展）：
  - 按钮栏存在且含三个按钮。
  - 初始状态：仅「添加场」启用，「添加单元」「删除」禁用。
  - 选中 field 节点后「添加单元」「删除」启用；选回 project 根后禁用。
- 回归：`tests/test_panels.py`（ElementPanel 加载/提交）、`tests/test_e2e_truss1d.py` 不受影响。

## 5. 影响面

| 文件 | 改动 |
| --- | --- |
| `pyTool/gui/main_window.py` | 左栏容器化 + 按钮栏 + `_updateActionButtons` + 调用点 |
| `pyTool/gui/widgets/csv_line_edit.py` | 新增 |
| `pyTool/gui/views/element_panel.py` | 两个 `ListEditor` 实例换 `CsvLineEdit`（含 placeholder） |
| `pyTool/gui/tests/test_widgets.py` | 扩展 CsvLineEdit 用例 |
| `pyTool/gui/tests/`（main_window 相关） | 扩展按钮栏与动态启用用例 |

不触动：`models/`（ProjectModel）、`services/`（json_io/generate）、生成器与模板、`ListEditor`/`TableEditor`、菜单结构。

## 6. 非目标（YAGNI）

- 不支持分号 `;` `；` 作为分隔符（需求仅逗号）。
- 不做单元变量名的合法性校验（如标识符规则）——现有 `ListEditor` 也不校验，保持一致。
- 不改造高斯区「权重」「形函数」的 `ListEditor`（它们项数多、单项可能较长，单行不适合）。
- 不删除「编辑」菜单入口（与按钮并存）。

## 7. 验收标准

1. 左侧项目树下方出现横向按钮栏，含「添加场」「添加单元」「删除」。
2. 选中 project 根：仅「添加场」可用；选中场/单元节点：三按钮均可用。
3. 「添加场」「添加单元」「删除」按钮与菜单项行为一致（共用 slot）。
4. 单元面板中「广义位移」「单元变量」为单行输入框，显示 placeholder。
5. 输入 `u，v，w`（中文逗号）保存后重新载入，得到三项 `["u","v","w"]`。
6. 现有测试（test_panels、test_e2e_truss1d）通过；新增 CsvLineEdit 与按钮栏测试通过。
