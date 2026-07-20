# pyTool 配置式生成器 GUI 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `pyTool/` 新增一个 PySide6 桌面界面，可视化编辑有限元项目三层结构（项目/场/单元）并一键生成 C++/CMake/GiD 代码，替代手写 `test/testXxx.py` 脚本。

**Architecture:** 严格复用现有 pyTool 数据层（`DataProject/DataField/DataEleSub(G)` + `toDict/fromDict`），其上叠加三层：`ProjectModel`（QObject 包装，脏标记 + Qt 信号）、`services`（json_io 工程文件读写 + generate 封装 MakerCpp/MakerGidFile）、`views`（主窗口 + 各编辑面板 + 可复用 widget）。GUI 只管元数据/数值参数，**不编辑代码片段**（`runCode` 等留空，符合"生成框架 + 人工填充"模式）。

**Tech Stack:** Python 3、PySide6（Qt6）、pytest（GUI 测试用 `QT_QPA_PLATFORM=offscreen`）、现有 pyTool 模块（Jinja2/MakerCpp/MakerGidFile）。

## Global Constraints

- **框架**：PySide6（由 PyQt5 调整而来）；`from PySide6.QtWidgets/QCore/QtGui import ...`；信号用 `Signal`（非 `pyqtSignal`）。
- **界面语言**：简体中文（控件文本、菜单、日志均为中文）。
- **命名约定**：类名与方法名用驼峰（与现有 `DataProject.addField/toDict` 一致）；Python 文件名与测试函数用 snake_case（pytest 惯例，与 `pyTool/test/test_*.py` 一致）；Qt 信号名用驼峰（如 `dirtyChanged`）。
- **模块导入**：`pyTool/gui/` 下的代码导入 pyTool 根模块（`DataProject` 等）时，需把 `pyTool/` 根目录加入 `sys.path`；由 `pyTool/gui/conftest.py`（测试）与 `pyTool/gui/main.py`（运行）各自负责。
- **复用不重造**：直接使用现有 `DataProject/DataField/DataEleSub/DataEleSubG` 及其 `toDict()/fromDict()`，GUI 不定义重复字段。
- **G 单元重建**：`json_io` 加载时**禁止**直接用 `DataField.fromDict`（其内部写死 `DataEleSub.fromDict`，会把 `DataEleSubG` 退化为基类、丢失 `gaussPoints/gaussWeights/shapeFuns`）；必须按单元字典的 `baseClass=='IsoEleBase'`（或含 `gaussPoints` 键）分发到 `DataEleSubG.fromDict`。
- **GUI 测试**：所有涉及 QWidget 的 pytest 必须在创建 `QApplication` 前设 `QT_QPA_PLATFORM=offscreen`（由 `conftest.py` 统一处理）。
- **代码片段不编辑**：`runCode/uCode/shapeFunCode/initCode/coordTransFunCode` 在 GUI 中无编辑入口，保持默认空字符串。
- **提交**：每个任务结束 commit；提交信息中文，conventional 风格（如 `feat(pyTool-gui): ...`）；只提交本任务相关文件。

---

## File Structure

| 文件 | 职责 | 创建任务 |
|---|---|---|
| `pyTool/gui/__init__.py` | 包标识（空） | Task 1 |
| `pyTool/gui/conftest.py` | pytest 配置：设 offscreen、注入 pyTool 根到 sys.path、提供 `qapp` fixture | Task 1 |
| `pyTool/gui/main.py` | `QApplication` 入口，注入 sys.path，显示 MainWindow | Task 1 / Task 10 |
| `pyTool/gui/models/__init__.py` | 包标识（空） | Task 2 |
| `pyTool/gui/models/project_model.py` | `ProjectModel(QObject)`：持有 DataProject、脏标记、增删改、Qt 信号 | Task 2 |
| `pyTool/gui/services/__init__.py` | 包标识（空） | Task 3 |
| `pyTool/gui/services/json_io.py` | `save(project,path)` / `load(path)->DataProject`（G 单元分发重建） | Task 3 |
| `pyTool/gui/services/generate.py` | `run(...)`：封装 MakerCpp/MakerGidFile + stdout 重定向 + 异常捕获 | Task 4 |
| `pyTool/gui/widgets/__init__.py` | 包标识（空） | Task 5 |
| `pyTool/gui/widgets/list_editor.py` | `ListEditor(QWidget)`：可复用字符串/数值列表增删上下移 | Task 5 |
| `pyTool/gui/widgets/table_editor.py` | `TableEditor(QWidget)`：可复用行列可变表格（paramNames、gaussPoints） | Task 6 |
| `pyTool/gui/views/__init__.py` | 包标识（空） | Task 7 |
| `pyTool/gui/views/project_panel.py` | `ProjectPanel`：项目节点字段编辑 | Task 7 |
| `pyTool/gui/views/field_panel.py` | `FieldPanel`：场节点字段编辑 | Task 7 |
| `pyTool/gui/views/element_panel.py` | `ElementPanel`：单元字段编辑（含 DataEleSubG 高斯区分支） | Task 8 |
| `pyTool/gui/views/generate_panel.py` | `GeneratePanel`：mode/mainMode/outPath/sln_cmake_path + 生成按钮 + 日志 | Task 9 |
| `pyTool/gui/main_window.py` | `MainWindow`：菜单/工具栏/三层树/堆叠面板/文件操作 | Task 10 |
| `pyTool/gui/tests/__init__.py` | 包标识（空） | Task 1 |
| `pyTool/gui/tests/test_skeleton.py` | 骨架冒烟：能 import 并构造 MainWindow | Task 1 |
| `pyTool/gui/tests/test_project_model.py` | ProjectModel 增删改 + 脏标记 | Task 2 |
| `pyTool/gui/tests/test_json_io.py` | round-trip（含 G 单元高斯字段不丢失） | Task 3 |
| `pyTool/gui/tests/test_generate.py` | generate 参数捕获（monkeypatch makeAll） | Task 4 |
| `pyTool/gui/tests/test_widgets.py` | ListEditor / TableEditor 数据 get/set | Task 5 / Task 6 |
| `pyTool/gui/tests/test_e2e_truss1d.py` | Truss1D GUI 路径 vs 脚本路径产物 diff | Task 11 |

---

## Task 1: 项目骨架与可启动空主窗口

**目标**：建立 `pyTool/gui/` 目录结构与 pytest 运行环境，交付一个可启动的空 `MainWindow` 与一个能跑通的 offscreen 冒烟测试。后续所有任务在此骨架上叠加。

**Files:**
- Create: `pyTool/gui/__init__.py`
- Create: `pyTool/gui/main.py`
- Create: `pyTool/gui/main_window.py`
- Create: `pyTool/gui/conftest.py`
- Create: `pyTool/gui/tests/__init__.py`
- Create: `pyTool/gui/tests/test_skeleton.py`

**Interfaces:**
- Produces: `MainWindow(QMainWindow)`（构造签名 `MainWindow()`，本期为空壳，Task 10 填充）；`conftest.py` 提供 `qapp` fixture（返回单例 `QApplication`），后续所有 GUI 测试复用。

- [ ] **Step 1: 写失败测试**

创建 `pyTool/gui/tests/test_skeleton.py`：

```python
# SPDX-License-Identifier: GPL-3.0
# pyTool GUI 骨架冒烟测试
from main_window import MainWindow


def test_main_window_can_construct(qapp):
    """空 MainWindow 必须能被构造，且窗口标题含中文标识。"""
    win = MainWindow()
    assert win is not None
    assert "pyTool" in win.windowTitle()
    win.deleteLater()
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_skeleton.py -v`
Expected: FAIL（`ModuleNotFoundError: No module named 'main_window'` 或 PySide6 未装）。

- [ ] **Step 3: 确认 PySide6 已安装**

Run: `python -c "import PySide6; print(PySide6.__version__)"`
若报错，执行：`pip install PySide6 pytest`
Expected: 打印 PySide6 版本号（≥6.x）。

- [ ] **Step 4: 写最小实现**

创建 `pyTool/gui/__init__.py`（空文件）：
```python
# pyTool GUI 包
```

创建 `pyTool/gui/tests/__init__.py`（空文件）：
```python
# 测试包
```

创建 `pyTool/gui/main_window.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# pyTool GUI 主窗口（Task 1 骨架，Task 10 填充内容）
from PySide6.QtWidgets import QMainWindow


class MainWindow(QMainWindow):
    """pyTool 配置式生成器主窗口。"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("pyTool 配置式生成器")
        self.resize(1100, 760)
```

创建 `pyTool/gui/conftest.py`（pytest 入口，被 `pyTool/gui/` 下的所有测试共享）：
```python
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
```

创建 `pyTool/gui/main.py`（运行入口）：
```python
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
```

- [ ] **Step 5: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_skeleton.py -v`
Expected: PASS（1 passed）。

- [ ] **Step 6: 手动启动确认窗口可见**

Run: `cd pyTool/gui && python main.py`
Expected: 弹出标题为「pyTool 配置式生成器」的空白窗口（1100×760），关闭即退出。

- [ ] **Step 7: 提交**

```bash
cd pyTool/gui && cd ../..
git add pyTool/gui/__init__.py pyTool/gui/main.py pyTool/gui/main_window.py pyTool/gui/conftest.py pyTool/gui/tests/__init__.py pyTool/gui/tests/test_skeleton.py
git commit -m "feat(pyTool-gui): 搭建 PySide6 骨架与 offscreen 测试环境"
```

---

## Task 2: ProjectModel（核心 ViewModel）

**目标**：实现 `ProjectModel(QObject)`，包装一个 `DataProject`，提供场/单元的增删改、脏标记管理与 Qt 信号。这是所有面板的数据后端。

**Files:**
- Create: `pyTool/gui/models/__init__.py`
- Create: `pyTool/gui/models/project_model.py`
- Create: `pyTool/gui/tests/test_project_model.py`

**Interfaces:**
- Consumes: `DataProject`、`DataField`、`DataEleSub`、`DataEleSubG`（现有）；`from DataProject import DataProject` 等（pyTool 根已在 sys.path）。
- Produces（供 Task 7/8/10 使用）：
  - `ProjectModel()` → 新建空模型（内部 `DataProject("", 2)`）
  - `ProjectModel.fromProject(proj: DataProject) -> ProjectModel`
  - `projectModel.project -> DataProject`（直接持有）
  - `projectModel.isDirty -> bool`
  - Qt 信号：`dirtyChanged = Signal(bool)`、`structureChanged = Signal()`（增删场/单元后发射）
  - `setProject(proj)`：整体替换，脏标记置 False
  - `addField(name: str) -> DataField`
  - `addEleSub(field: DataField, name: str, gauss: bool) -> DataEleSub | DataEleSubG`（`gauss=True` 建 `DataEleSubG`）
  - `removeField(field: DataField) -> None`
  - `removeEleSub(field: DataField, ele) -> None`
  - `markDirty()` / `markClean()`
  - `toProject() -> DataProject`（返回 `self.project`）

- [ ] **Step 1: 写失败测试**

创建 `pyTool/gui/tests/test_project_model.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# ProjectModel 测试
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from models.project_model import ProjectModel


def test_new_model_is_not_dirty():
    m = ProjectModel()
    assert m.isDirty is False
    assert m.project.name == ""
    assert m.project.dim == 2


def test_add_field_marks_dirty_and_emits():
    m = ProjectModel()
    dirty_hits = []
    struct_hits = []
    m.dirtyChanged.connect(lambda d: dirty_hits.append(d))
    m.structureChanged.connect(lambda: struct_hits.append(1))
    f = m.addField("ElDisp")
    assert isinstance(f, DataField)
    assert f in m.project.fields
    assert m.isDirty is True
    assert dirty_hits == [True]
    assert struct_hits == [1]


def test_add_elesub_gauss_creates_gauss_type():
    m = ProjectModel()
    f = m.addField("ElDisp")
    m.markClean()
    ele = m.addEleSub(f, "ElQ4g", gauss=True)
    assert isinstance(ele, DataEleSubG)
    assert ele.baseClass == "IsoEleBase"
    assert m.isDirty is True


def test_add_elessub_plain_creates_base_type():
    m = ProjectModel()
    f = m.addField("ElDisp")
    m.addEleSub(f, "Truss", gauss=False)
    assert any(isinstance(e, DataEleSub) and not isinstance(e, DataEleSubG)
               for e in f.eleSubs)


def test_remove_field_and_elesub():
    m = ProjectModel()
    f = m.addField("ElDisp")
    ele = m.addEleSub(f, "ElQ4g", gauss=True)
    m.removeEleSub(f, ele)
    assert ele not in f.eleSubs
    m.removeField(f)
    assert f not in m.project.fields


def test_set_project_resets_dirty():
    m = ProjectModel()
    m.addField("X")
    assert m.isDirty is True
    proj = DataProject("New", 3)
    m.setProject(proj)
    assert m.project is proj
    assert m.isDirty is False
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_project_model.py -v`
Expected: FAIL（`No module named 'models.project_model'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/models/__init__.py`（空）：
```python
# models 包
```

创建 `pyTool/gui/models/project_model.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# ProjectModel：DataProject 的 ViewModel 包装（脏标记 + Qt 信号）
from PySide6.QtCore import QObject, Signal

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG


class ProjectModel(QObject):
    """当前会话 DataProject 的包装，负责增删改与脏标记广播。"""

    dirtyChanged = Signal(bool)
    structureChanged = Signal()

    def __init__(self, project: DataProject = None):
        super().__init__()
        self.project = project if project is not None else DataProject("", 2)
        self._dirty = False

    # ---- 工厂 ----
    @classmethod
    def fromProject(cls, project: DataProject) -> "ProjectModel":
        return cls(project)

    # ---- 脏标记 ----
    @property
    def isDirty(self) -> bool:
        return self._dirty

    def markDirty(self):
        if not self._dirty:
            self._dirty = True
            self.dirtyChanged.emit(True)

    def markClean(self):
        if self._dirty:
            self._dirty = False
            self.dirtyChanged.emit(False)

    # ---- 整体替换 ----
    def setProject(self, project: DataProject):
        self.project = project
        self._dirty = False
        self.dirtyChanged.emit(False)
        self.structureChanged.emit()

    def toProject(self) -> DataProject:
        return self.project

    # ---- 增删 ----
    def addField(self, name: str) -> DataField:
        field = self.project.addField(name)
        self._afterStructChange()
        return field

    def addEleSub(self, field: DataField, name: str, gauss: bool):
        ele = DataEleSubG(name) if gauss else DataEleSub(name)
        field.addEleSub(ele)
        self._afterStructChange()
        return ele

    def removeField(self, field: DataField):
        if field in self.project.fields:
            self.project.fields.remove(field)
            self._afterStructChange()

    def removeEleSub(self, field: DataField, ele):
        if ele in field.eleSubs:
            field.eleSubs.remove(ele)
            field.makeData()  # 重新聚合 dispNames/eleResNames
            self._afterStructChange()

    def _afterStructChange(self):
        self.markDirty()
        self.structureChanged.emit()
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_project_model.py -v`
Expected: PASS（6 passed）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/models/__init__.py pyTool/gui/models/project_model.py pyTool/gui/tests/test_project_model.py
git commit -m "feat(pyTool-gui): 实现 ProjectModel（DataProject 包装+脏标记+信号）"
```

---

## Task 3: json_io service（G 单元分发重建）

**目标**：实现工程文件读写。`save` 直接 `toDict` 写 JSON；`load` **禁止**用 `DataField.fromDict`，必须按 `baseClass` 分发重建 `DataEleSubG`/`DataEleSub`，确保高斯字段 round-trip 不丢失。

**Files:**
- Create: `pyTool/gui/services/__init__.py`
- Create: `pyTool/gui/services/json_io.py`
- Create: `pyTool/gui/tests/test_json_io.py`

**Interfaces:**
- Consumes: `DataProject/DataField/DataEleSub/DataEleSubG/DataSch`（现有）。
- Produces（供 Task 10 使用）：
  - `json_io.save(project: DataProject, path: str) -> None`
  - `json_io.load(path: str) -> DataProject`
  - 工程文件扩展名约定常量 `EXT = ".cdfeg.json"`

- [ ] **Step 1: 写失败测试**

创建 `pyTool/gui/tests/test_json_io.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# json_io round-trip 测试（重点：DataEleSubG 高斯字段不丢失）
import os
import tempfile

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from services import json_io


def _buildMixedProject() -> DataProject:
    """含 1 个普通单元 + 1 个高斯积分单元的项目。"""
    proj = DataProject("Mixed", 2)
    field = DataField("ElDisp")
    plain = DataEleSub("Truss", 2)
    plain.dispNames = ["u"]
    plain.paramNames = ["E", "A"]
    field.addEleSub(plain)

    g = DataEleSubG("ElQ4g", 4)
    g.type = 2
    g.dispNames = ["u", "v"]
    g.paramNames = ["pe", "pv"]
    g.gaussPoints = [[0.5, 0.5], [-0.5, 0.5]]
    g.gaussWeights = [1.0, 1.0]
    g.shapeFuns = ["0.25*(1-x)*(1-y)", "0.25*(1+x)*(1-y)"]
    field.addEleSub(g)

    proj.addField(field)
    return proj


def test_roundtrip_preserves_plain_and_gauss(tmp_path):
    proj = _buildMixedProject()
    path = str(tmp_path / "mixed.cdfeg.json")
    json_io.save(proj, path)
    assert os.path.exists(path)

    loaded = json_io.load(path)
    assert loaded.name == "Mixed"
    assert len(loaded.fields) == 1
    fld = loaded.fields[0]
    assert len(fld.eleSubs) == 2

    # 普通单元
    plain = next(e for e in fld.eleSubs if e.name == "Truss")
    assert not isinstance(plain, DataEleSubG)
    assert plain.paramNames == ["E", "A"]

    # 高斯单元：必须是 DataEleSubG 且高斯字段完整
    g = next(e for e in fld.eleSubs if e.name == "ElQ4g")
    assert isinstance(g, DataEleSubG)
    assert g.baseClass == "IsoEleBase"
    assert g.gaussPoints == [[0.5, 0.5], [-0.5, 0.5]]
    assert g.gaussWeights == [1.0, 1.0]
    assert g.shapeFuns == ["0.25*(1-x)*(1-y)", "0.25*(1+x)*(1-y)"]


def test_load_bad_json_raises(tmp_path):
    path = str(tmp_path / "bad.cdfeg.json")
    with open(path, "w", encoding="utf-8") as f:
        f.write("{ 不是合法 json")
    try:
        json_io.load(path)
        assert False, "应抛异常"
    except Exception:
        assert True
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_json_io.py -v`
Expected: FAIL（`No module named 'services.json_io'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/services/__init__.py`（空）：
```python
# services 包
```

创建 `pyTool/gui/services/json_io.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# 工程文件读写：load 时按 baseClass 分发重建 DataEleSubG/DataEleSub，
# 绕开 DataField.fromDict 的基类退化（其内部写死 DataEleSub.fromDict）。
import json

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from DataSch import DataSch

EXT = ".cdfeg.json"


def save(project: DataProject, path: str) -> None:
    """保存为 .cdfeg.json（内容即 DataProject.toDict()）。"""
    with open(path, "w", encoding="utf-8") as f:
        json.dump(project.toDict(), f, indent=4, ensure_ascii=False)


def _buildEle(ele_data: dict):
    """按 baseClass / 是否含 gaussPoints 判断单元子类。"""
    if ele_data.get("baseClass") == "IsoEleBase" or "gaussPoints" in ele_data:
        return DataEleSubG.fromDict(ele_data)
    return DataEleSub.fromDict(ele_data)


def load(path: str) -> DataProject:
    """从 .cdfeg.json 重建 DataProject（G 单元高斯字段不丢失）。"""
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)  # 格式损坏时抛异常，由调用方处理

    project = DataProject(data.get("name", ""), data.get("dim", 2))
    project.coordVars = data.get("coordVars", ["x", "y", "z"][:project.dim])
    project.eleType = data.get("eleType", [])
    project.caculateCode = data.get("caculateCode", "")
    project.preParams = data.get("preParams", [])

    for field_data in data.get("fields", []):
        field = DataField(field_data.get("name", ""))
        field.eleTypes = field_data.get("eleTypes", [])
        field.pdeType = field_data.get("pdeType", 1)
        field.index = field_data.get("index", 1)
        field.bDynamic = field_data.get("bDynamic", False)
        field.preParams = field_data.get("preParams", [])
        for ele_data in field_data.get("eleSubs", []):
            field.eleSubs.append(_buildEle(ele_data))
        if "sch" in field_data:
            field.sch = DataSch.fromDict(field_data["sch"])
        field.makeData()  # 重新聚合 dispNames / eleResNames
        project.addField(field)

    return project
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_json_io.py -v`
Expected: PASS（2 passed）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/services/__init__.py pyTool/gui/services/json_io.py pyTool/gui/tests/test_json_io.py
git commit -m "feat(pyTool-gui): 实现 json_io（G 单元分发重建，绕开 fromDict 退化）"
```

---

## Task 4: generate service（封装 Maker + 日志重定向）

**目标**：封装 `MakerCpp` + `MakerGidFile` 调用，重定向 stdout 到日志回调，捕获异常不上抛。`mainMode==1` 时额外调 `MakerGidFile`。

**Files:**
- Create: `pyTool/gui/services/generate.py`
- Create: `pyTool/gui/tests/test_generate.py`

**Interfaces:**
- Consumes: `MakerCpp`、`MakerGidFile`（现有）；模块级 `import`，便于测试 monkeypatch。
- Produces（供 Task 9 GeneratePanel 使用）：
  - `generate.run(project, mode, mainMode, outPath, sln_cmake_path=None, log=print) -> tuple[bool, str]`
    - 返回 `(成功标志, 日志文本)`；`log` 为每条日志的回调（默认 print）。
  - 模块属性 `generate.MakerCpp` / `generate.MakerGidFile`（测试替换用）。

- [ ] **Step 1: 写失败测试**

创建 `pyTool/gui/tests/test_generate.py`：
```python
# SPDX-License-Identifier: GPL-3.0
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
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_generate.py -v`
Expected: FAIL（`No module named 'services.generate'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/services/generate.py`：
```python
# SPDX-License-Identifier: GPL-3.0
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
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_generate.py -v`
Expected: PASS（3 passed）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/services/generate.py pyTool/gui/tests/test_generate.py
git commit -m "feat(pyTool-gui): 实现 generate service（Maker 封装+日志重定向+异常捕获）"
```

---

## Task 5: ListEditor 可复用 widget

**目标**：实现可复用的字符串列表编辑器（增删 / 上下移），供单元面板编辑 `dispNames`/`eleResNames`/`gaussWeights`/`shapeFuns`。

**Files:**
- Create: `pyTool/gui/widgets/__init__.py`
- Create: `pyTool/gui/widgets/list_editor.py`
- Create: `pyTool/gui/tests/test_widgets.py`（本任务先加 ListEditor 用例，Task 6 追加 TableEditor 用例）

**Interfaces:**
- Produces（供 Task 8 使用）：
  - `ListEditor(label: str = "")`
  - `listEditor.setItems(list[str]) -> None`
  - `listEditor.items() -> list[str]`
  - `listEditor.itemsChanged = Signal()`（内容变化时发射）

- [ ] **Step 1: 写失败测试**

在 `pyTool/gui/tests/test_widgets.py` 写入：
```python
# SPDX-License-Identifier: GPL-3.0
# 可复用 widget 测试
from widgets.list_editor import ListEditor


def test_list_editor_set_get(qapp):
    le = ListEditor("广义位移")
    le.setItems(["u", "v"])
    assert le.items() == ["u", "v"]


def test_list_editor_add_remove_move(qapp):
    le = ListEditor()
    le.setItems(["a", "b", "c"])
    # 选中第 1 行（"b"）下移
    le._list.setCurrentRow(1)
    le._moveDown()
    assert le.items() == ["a", "c", "b"]
    # 删除第 0 行
    le._list.setCurrentRow(0)
    le._remove()
    assert le.items() == ["c", "b"]


def test_list_editor_emits_changed(qapp):
    le = ListEditor()
    hits = []
    le.itemsChanged.connect(lambda: hits.append(1))
    le.setItems(["x"])
    # setItems 不触发；新增才触发
    assert hits == []
    le._add()
    assert len(hits) >= 1
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_widgets.py -v`
Expected: FAIL（`No module named 'widgets.list_editor'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/widgets/__init__.py`（空）：
```python
# widgets 包
```

创建 `pyTool/gui/widgets/list_editor.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# ListEditor：可复用字符串列表编辑器（增/删/上移/下移）
from PySide6.QtCore import Signal
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QListWidget, QPushButton,
    QInputDialog,
)


class ListEditor(QWidget):
    """字符串列表编辑器，带增删与上下移按钮。"""

    itemsChanged = Signal()

    def __init__(self, label: str = "", parent=None):
        super().__init__(parent)
        self._label = label
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        if label:
            layout.addWidget(QLabel(label))

        self._list = QListWidget()
        layout.addWidget(self._list)

        btnRow = QHBoxLayout()
        self._btnAdd = QPushButton("新增")
        self._btnDel = QPushButton("删除")
        self._btnUp = QPushButton("上移")
        self._btnDown = QPushButton("下移")
        for b in (self._btnAdd, self._btnDel, self._btnUp, self._btnDown):
            btnRow.addWidget(b)
        layout.addLayout(btnRow)

        self._btnAdd.clicked.connect(self._add)
        self._btnDel.clicked.connect(self._remove)
        self._btnUp.clicked.connect(self._moveUp)
        self._btnDown.clicked.connect(self._moveDown)

    def setItems(self, items):
        self._list.clear()
        for it in items:
            self._list.addItem(str(it))

    def items(self):
        return [self._list.item(i).text() for i in range(self._list.count())]

    def _add(self):
        text, ok = QInputDialog.getText(self, "新增", "请输入：")
        if ok and text:
            self._list.addItem(text)
            self.itemsChanged.emit()

    def _remove(self):
        row = self._list.currentRow()
        if row >= 0:
            self._list.takeItem(row)
            self.itemsChanged.emit()

    def _moveUp(self):
        row = self._list.currentRow()
        if row > 0:
            item = self._list.takeItem(row)
            self._list.insertItem(row - 1, item)
            self._list.setCurrentRow(row - 1)
            self.itemsChanged.emit()

    def _moveDown(self):
        row = self._list.currentRow()
        if 0 <= row < self._list.count() - 1:
            item = self._list.takeItem(row)
            self._list.insertItem(row + 1, item)
            self._list.setCurrentRow(row + 1)
            self.itemsChanged.emit()
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_widgets.py -v`
Expected: PASS（3 passed）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/widgets/__init__.py pyTool/gui/widgets/list_editor.py pyTool/gui/tests/test_widgets.py
git commit -m "feat(pyTool-gui): 实现 ListEditor 可复用列表编辑器"
```

---

## Task 6: TableEditor 可复用 widget

**目标**：实现行列可变的表格编辑器，供单元面板编辑 `paramNames+paramValues`（2 列：名/值）与 `gaussPoints`（`dim` 列浮点坐标）。

**Files:**
- Create: `pyTool/gui/widgets/table_editor.py`
- Modify: `pyTool/gui/tests/test_widgets.py`（追加 TableEditor 用例）

**Interfaces:**
- Produces（供 Task 8 使用）：
  - `TableEditor(columns: list[str])`（如 `["名称", "默认值"]`）
  - `tableEditor.setRows(list[list[str]]) -> None`
  - `tableEditor.rows() -> list[list[str]]`（空尾行自动剔除）
  - `tableEditor.rowsChanged = Signal()`

- [ ] **Step 1: 写失败测试**

在 `pyTool/gui/tests/test_widgets.py` 末尾追加：
```python
from widgets.table_editor import TableEditor


def test_table_editor_set_get(qapp):
    te = TableEditor(["名称", "默认值"])
    te.setRows([["E", ""], ["A", ""]])
    assert te.rows() == [["E", ""], ["A", ""]]


def test_table_editor_gauss_points(qapp):
    te = TableEditor(["xi", "eta"])
    te.setRows([["0.5", "0.5"], ["-0.5", "0.5"]])
    rows = te.rows()
    assert rows == [["0.5", "0.5"], ["-0.5", "0.5"]]


def test_table_editor_empty_rows_dropped(qapp):
    te = TableEditor(["a"])
    te.setRows([["x"], [""], ["y"]])
    assert te.rows() == [["x"], ["y"]]
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_widgets.py -v`
Expected: FAIL（`No module named 'widgets.table_editor'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/widgets/table_editor.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# TableEditor：可复用行列可变表格编辑器
from PySide6.QtCore import Signal
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTableWidget, QTableWidgetItem,
    QPushButton, QHeaderView,
)


class TableEditor(QWidget):
    """行列可变表格编辑器，带增删行按钮。空行在 rows() 中被剔除。"""

    rowsChanged = Signal()

    def __init__(self, columns, parent=None):
        super().__init__(parent)
        self._ncol = len(columns)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)

        self._table = QTableWidget(0, self._ncol)
        self._table.setHorizontalHeaderLabels(columns)
        self._table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        layout.addWidget(self._table)

        row = QHBoxLayout()
        self._btnAdd = QPushButton("新增行")
        self._btnDel = QPushButton("删除行")
        row.addWidget(self._btnAdd)
        row.addWidget(self._btnDel)
        row.addStretch(1)
        layout.addLayout(row)

        self._btnAdd.clicked.connect(self._addRow)
        self._btnDel.clicked.connect(self._delRow)
        self._table.itemChanged.connect(lambda *_: self.rowsChanged.emit())

    def setRows(self, rows):
        self._table.setRowCount(0)
        for r in rows:
            self._addRowWithData([str(x) for x in r])

    def rows(self):
        out = []
        for i in range(self._table.rowCount()):
            vals = []
            for j in range(self._ncol):
                item = self._table.item(i, j)
                vals.append(item.text() if item is not None else "")
            if any(v.strip() for v in vals):   # 剔除全空行
                out.append(vals)
        return out

    def _addRowWithData(self, values):
        r = self._table.rowCount()
        self._table.insertRow(r)
        for j in range(self._ncol):
            self._table.setItem(r, j, QTableWidgetItem(values[j] if j < len(values) else ""))

    def _addRow(self):
        self._addRowWithData([""] * self._ncol)
        self.rowsChanged.emit()

    def _delRow(self):
        r = self._table.currentRow()
        if r >= 0:
            self._table.removeRow(r)
            self.rowsChanged.emit()
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_widgets.py -v`
Expected: PASS（6 passed：3 ListEditor + 3 TableEditor）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/widgets/table_editor.py pyTool/gui/tests/test_widgets.py
git commit -m "feat(pyTool-gui): 实现 TableEditor 可复用表格编辑器"
```

---

## Task 7: ProjectPanel 与 FieldPanel

**目标**：实现项目节点与场节点的编辑面板，绑定 `ProjectModel` 中对应对象的字段。切换到该节点时回填，编辑后写回并 `markDirty`。

**Files:**
- Create: `pyTool/gui/views/__init__.py`
- Create: `pyTool/gui/views/project_panel.py`
- Create: `pyTool/gui/views/field_panel.py`
- Create: `pyTool/gui/tests/test_panels.py`

**Interfaces:**
- Consumes: `ProjectModel`（Task 2）。
- Produces（供 Task 10 MainWindow 使用）：
  - `ProjectPanel(model: ProjectModel)`：方法 `loadProject()`（从 model 回填）、
  - `FieldPanel(model: ProjectModel)`：方法 `loadField(field: DataField)`（回填指定场）。

- [ ] **Step 1: 写失败测试**

创建 `pyTool/gui/tests/test_panels.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# 面板读写 model 测试
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from models.project_model import ProjectModel
from views.project_panel import ProjectPanel
from views.field_panel import FieldPanel


def test_project_panel_edits_name_and_dim(qapp):
    proj = DataProject("Truss1D", 1)
    m = ProjectModel.fromProject(proj)
    p = ProjectPanel(m)
    p.loadProject()
    p._name.setText("Renamed")
    p._dim.setCurrentIndex(2)  # 0->1D,1->2D,2->3D
    p._commit()
    assert proj.name == "Renamed"
    assert proj.dim == 3
    assert m.isDirty is True


def test_field_panel_edits_pdtype(qapp):
    m = ProjectModel()
    f = m.addField("ElDisp")
    m.markClean()
    fp = FieldPanel(m)
    fp.loadField(f)
    fp._pdeType.setCurrentIndex(1)  # 0->1椭圆,1->2抛物,2->3双曲
    fp._commit()
    assert f.pdeType == 2
    assert m.isDirty is True
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_panels.py -v`
Expected: FAIL（`No module named 'views.project_panel'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/views/__init__.py`（空）：
```python
# views 包
```

创建 `pyTool/gui/views/project_panel.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# 项目节点编辑面板
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QComboBox, QLabel,
)


class ProjectPanel(QWidget):
    """编辑 DataProject 的 name / dim（coordVars 按 dim 派生展示）。"""

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        form = QFormLayout(self)

        self._name = QLineEdit()
        self._dim = QComboBox()
        self._dim.addItems(["1 (一维)", "2 (二维)", "3 (三维)"])
        self._coordVars = QLabel("(由维度派生)")

        form.addRow("项目名称", self._name)
        form.addRow("总体维度", self._dim)
        form.addRow("坐标变量", self._coordVars)

        self._name.editingFinished.connect(self._commit)
        self._dim.currentIndexChanged.connect(self._commit)

    def loadProject(self):
        proj = self._model.project
        self._name.setText(proj.name)
        self._dim.setCurrentIndex(proj.dim - 1)
        self._refreshCoordVars(proj.dim)

    def _refreshCoordVars(self, dim):
        proj = self._model.project
        proj.coordVars = ['x', 'y', 'z'][:dim]
        self._coordVars.setText(str(proj.coordVars))

    def _commit(self):
        proj = self._model.project
        proj.name = self._name.text()
        proj.dim = self._dim.currentIndex() + 1
        self._refreshCoordVars(proj.dim)
        self._model.markDirty()
```

创建 `pyTool/gui/views/field_panel.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# 场节点编辑面板
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QComboBox, QCheckBox, QLabel,
)


class FieldPanel(QWidget):
    """编辑 DataField；dispNames/eleResNames 由单元聚合，只读展示。"""

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        self._field = None
        form = QFormLayout(self)

        self._name = QLineEdit()
        self._pdeType = QComboBox()
        self._pdeType.addItems(["1 椭圆型 (K)", "2 抛物型 (K/M)", "3 双曲型 (K/M/D)"])
        self._bDynamic = QCheckBox("动力学场")
        self._dispNames = QLabel("(由单元聚合)")
        self._eleResNames = QLabel("(由单元聚合)")

        form.addRow("场名称", self._name)
        form.addRow("PDE 类型", self._pdeType)
        form.addRow("动力学", self._bDynamic)
        form.addRow("广义位移", self._dispNames)
        form.addRow("单元变量", self._eleResNames)

        self._name.editingFinished.connect(self._commit)
        self._pdeType.currentIndexChanged.connect(self._commit)
        self._bDynamic.toggled.connect(self._commit)

    def loadField(self, field):
        self._field = field
        self._name.setText(field.name)
        self._pdeType.setCurrentIndex(field.pdeType - 1)
        self._bDynamic.setChecked(field.bDynamic)
        self._refreshAgg()

    def _refreshAgg(self):
        if self._field is None:
            return
        self._field.makeData()
        self._dispNames.setText(str(self._field.dispNames))
        self._eleResNames.setText(str(self._field.eleResNames))

    def _commit(self):
        if self._field is None:
            return
        self._field.name = self._name.text()
        self._field.pdeType = self._pdeType.currentIndex() + 1
        self._field.bDynamic = self._bDynamic.isChecked()
        self._refreshAgg()
        self._model.markDirty()
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_panels.py -v`
Expected: PASS（2 passed）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/views/__init__.py pyTool/gui/views/project_panel.py pyTool/gui/views/field_panel.py pyTool/gui/tests/test_panels.py
git commit -m "feat(pyTool-gui): 实现 ProjectPanel 与 FieldPanel"
```

---

## Task 8: ElementPanel（含 DataEleSubG 高斯区分支）

**目标**：实现单元编辑面板。公共字段区对所有单元生效；当单元 `baseClass=='IsoEleBase'`（即 `DataEleSubG`）时额外显示高斯区（`gaussOrder`/`gaussPoints`/`gaussWeights`/`shapeFuns`）。

**Files:**
- Create: `pyTool/gui/views/element_panel.py`
- Modify: `pyTool/gui/tests/test_panels.py`（追加 ElementPanel 用例）

**Interfaces:**
- Consumes: `ProjectModel`（Task 2）、`ListEditor`（Task 5）、`TableEditor`（Task 6）。
- Produces（供 Task 10 使用）：
  - `ElementPanel(model: ProjectModel)`
  - `elementPanel.loadEleSub(field: DataField, ele) -> None`：回填；按 `isinstance(ele, DataEleSubG)` 显隐高斯区。

- [ ] **Step 1: 写失败测试**

在 `pyTool/gui/tests/test_panels.py` 末尾追加：
```python
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from views.element_panel import ElementPanel


def test_element_panel_plain_writes_back(qapp):
    m = ProjectModel()
    f = m.addField("F")
    ele = m.addEleSub(f, "Truss", gauss=False)
    m.markClean()
    ep = ElementPanel(m)
    ep.loadEleSub(f, ele)
    assert ep._gaussGroup.isVisible() is False  # 普通单元不显示高斯区
    ep._name.setText("Truss2")
    ep._nNodes.setValue(3)
    ep._commit()
    assert ele.name == "Truss2"
    assert ele.nNodes == 3
    assert m.isDirty is True


def test_element_panel_gauss_shows_gauss_group(qapp):
    m = ProjectModel()
    f = m.addField("F")
    g = m.addEleSub(f, "ElQ4g", gauss=True)
    g.gaussPoints = [[0.5, 0.5]]
    g.gaussWeights = [1.0]
    g.shapeFuns = ["N1"]
    ep = ElementPanel(m)
    ep.loadEleSub(f, g)
    assert ep._gaussGroup.isVisible() is True
    assert ep._gaussPoints.rows() == [["0.5", "0.5"]]
    assert ep._gaussWeights.items() == ["1.0"]
    assert ep._shapeFuns.items() == ["N1"]


def test_element_panel_param_table_roundtrip(qapp):
    m = ProjectModel()
    f = m.addField("F")
    ele = m.addEleSub(f, "Truss", gauss=False)
    ele.paramNames = ["E", "A"]
    ele.paramValues = ["", ""]
    ep = ElementPanel(m)
    ep.loadEleSub(f, ele)
    assert ep._params.rows() == [["E", ""], ["A", ""]]
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_panels.py -v`
Expected: FAIL（`No module named 'views.element_panel'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/views/element_panel.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# 单元编辑面板（含 DataEleSubG 高斯区分支）
from PySide6.QtWidgets import (
    QWidget, QFormLayout, QLineEdit, QSpinBox, QComboBox, QCheckBox,
    QGroupBox, QVBoxLayout, QHBoxLayout, QLabel,
)

from DataEleSubG import DataEleSubG
from widgets.list_editor import ListEditor
from widgets.table_editor import TableEditor


class ElementPanel(QWidget):
    """单元字段编辑；DataEleSubG 额外显示高斯积分区。"""

    def __init__(self, model, parent=None):
        super().__init__(parent)
        self._model = model
        self._field = None
        self._ele = None
        root = QVBoxLayout(self)

        # ---- 公共字段 ----
        common = QGroupBox("单元公共属性")
        form = QFormLayout()
        self._name = QLineEdit()
        self._nNodes = QSpinBox()
        self._nNodes.setRange(1, 999)
        self._type = QComboBox()
        self._type.addItems(["0 点单元", "1 线单元", "2 面单元", "3 体单元"])
        self._dim = QSpinBox()
        self._dim.setRange(1, 3)
        self._bBC = QCheckBox("边界单元")
        self._baseClass = QComboBox()
        self._baseClass.addItems(["EleSubBase", "IsoEleBase"])
        self._gidName = QLineEdit()
        form.addRow("单元名称", self._name)
        form.addRow("节点数", self._nNodes)
        form.addRow("几何类型", self._type)
        form.addRow("单元维度", self._dim)
        form.addRow("边界单元", self._bBC)
        form.addRow("基类", self._baseClass)
        form.addRow("GiD 名称", self._gidName)
        common.setLayout(form)
        root.addWidget(common)

        # ---- 列表：dispNames / eleResNames ----
        lists = QGroupBox("位移与变量名")
        lh = QHBoxLayout()
        self._dispNames = ListEditor("广义位移")
        self._eleResNames = ListEditor("单元变量")
        lh.addWidget(self._dispNames)
        lh.addWidget(self._eleResNames)
        lists.setLayout(lh)
        root.addWidget(lists)

        # ---- 参数表（名 / 默认值）----
        params = QGroupBox("材料参数（名 / 默认值）")
        pv = QVBoxLayout()
        self._params = TableEditor(["名称", "默认值"])
        pv.addWidget(self._params)
        params.setLayout(pv)
        root.addWidget(params)

        # ---- 高斯区（仅 DataEleSubG）----
        self._gaussGroup = QGroupBox("高斯积分（仅 IsoEleBase 单元）")
        gv = QVBoxLayout()
        gform = QFormLayout()
        self._gaussOrder = QSpinBox()
        self._gaussOrder.setRange(1, 10)
        gform.addRow("积分阶数", self._gaussOrder)
        gv.addLayout(gform)
        gv.addWidget(QLabel("积分点坐标（每行一个点，列=单元维度）"))
        self._gaussPoints = TableEditor(["x1", "x2", "x3"])
        gv.addWidget(self._gaussPoints)
        gv.addWidget(QLabel("积分权重"))
        self._gaussWeights = ListEditor("权重")
        gv.addWidget(self._gaussWeights)
        gv.addWidget(QLabel("形函数表达式"))
        self._shapeFuns = ListEditor("形函数")
        gv.addWidget(self._shapeFuns)
        self._gaussGroup.setLayout(gv)
        self._gaussGroup.setVisible(False)
        root.addWidget(self._gaussGroup)

        root.addStretch(1)

        # 编辑即提交
        for w in (self._name,):
            w.editingFinished.connect(self._commit)
        for s in (self._nNodes, self._dim, self._gaussOrder):
            s.valueChanged.connect(self._commit)
        for c in (self._type, self._baseClass):
            c.currentIndexChanged.connect(self._commit)
        self._bBC.toggled.connect(self._commit)
        self._gidName.editingFinished.connect(self._commit)
        self._dispNames.itemsChanged.connect(self._commit)
        self._eleResNames.itemsChanged.connect(self._commit)
        self._params.rowsChanged.connect(self._commit)
        self._gaussPoints.rowsChanged.connect(self._commit)
        self._gaussWeights.itemsChanged.connect(self._commit)
        self._shapeFuns.itemsChanged.connect(self._commit)

    def loadEleSub(self, field, ele):
        self._field = field
        self._ele = ele
        # 临时断开信号避免回填触发 commit
        self._blockAll(True)
        try:
            self._name.setText(ele.name)
            self._nNodes.setValue(ele.nNodes)
            self._type.setCurrentIndex(ele.type)
            self._dim.setValue(ele.dim)
            self._bBC.setChecked(ele.bBC)
            self._baseClass.setCurrentIndex(0 if ele.baseClass != "IsoEleBase" else 1)
            self._gidName.setText(ele.gidName or ele.name)
            self._dispNames.setItems(ele.dispNames)
            self._eleResNames.setItems(ele.eleResNames)
            self._params.setRows(list(map(list, zip(
                ele.paramNames,
                [str(v) for v in ele.paramValues] if ele.paramValues else [""] * len(ele.paramNames)
            ))) if ele.paramNames else [])
            is_g = isinstance(ele, DataEleSubG)
            self._gaussGroup.setVisible(is_g)
            if is_g:
                self._gaussOrder.setValue(ele.gaussOrder)
                self._gaussPoints.setRows([list(map(str, pt)) for pt in ele.gaussPoints])
                self._gaussWeights.setItems([str(w) for w in ele.gaussWeights])
                self._shapeFuns.setItems(ele.shapeFuns)
        finally:
            self._blockAll(False)

    def _blockAll(self, on):
        for w in (self._name, self._nNodes, self._dim, self._gaussOrder,
                  self._type, self._baseClass, self._bBC, self._gidName):
            w.blockSignals(on)

    def _commit(self, *_):
        if self._ele is None:
            return
        ele = self._ele
        ele.name = self._name.text()
        ele.nNodes = self._nNodes.value()
        ele.type = self._type.currentIndex()
        ele.dim = self._dim.value()
        ele.bBC = self._bBC.isChecked()
        ele.gidName = self._gidName.text()
        ele.dispNames = self._dispNames.items()
        ele.eleResNames = self._eleResNames.items()
        rows = self._params.rows()
        ele.paramNames = [r[0] for r in rows]
        ele.paramValues = [r[1] if len(r) > 1 else "" for r in rows]
        if isinstance(ele, DataEleSubG):
            ele.gaussOrder = self._gaussOrder.value()
            ele.gaussPoints = [[float(x) for x in r] for r in self._gaussPoints.rows()]
            ele.gaussWeights = [float(w) for w in self._gaussWeights.items()]
            ele.shapeFuns = self._shapeFuns.items()
        if self._field is not None:
            self._field.makeData()
        self._model.markDirty()
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_panels.py -v`
Expected: PASS（5 passed：2 Project/Field + 3 Element）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/views/element_panel.py pyTool/gui/tests/test_panels.py
git commit -m "feat(pyTool-gui): 实现 ElementPanel（含 DataEleSubG 高斯区分支）"
```

---

## Task 9: GeneratePanel（生成参数 + 日志）

**目标**：实现底部生成面板，暴露 `mode/mainMode/outPath/sln_cmake_path`，点「生成」调 `generate.run`，日志写入只读文本框；`mode='add'` 时才启用 `sln_cmake_path`。

**Files:**
- Create: `pyTool/gui/views/generate_panel.py`
- Create: `pyTool/gui/tests/test_generate_panel.py`

**Interfaces:**
- Consumes: `ProjectModel`（Task 2）、`generate` service（Task 4）。
- Produces（供 Task 10 使用）：
  - `GeneratePanel(model: ProjectModel)`
  - `generatePanel.setDefaults(outPath: str, sln_cmake_path: str)`：填默认值。

- [ ] **Step 1: 写失败测试**

创建 `pyTool/gui/tests/test_generate_panel.py`：
```python
# SPDX-License-Identifier: GPL-3.0
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
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_generate_panel.py -v`
Expected: FAIL（`No module named 'views.generate_panel'`）。

- [ ] **Step 3: 写实现**

创建 `pyTool/gui/views/generate_panel.py`：
```python
# SPDX-License-Identifier: GPL-3.0
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
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_generate_panel.py -v`
Expected: PASS（2 passed）。

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/views/generate_panel.py pyTool/gui/tests/test_generate_panel.py
git commit -m "feat(pyTool-gui): 实现 GeneratePanel（生成参数+日志+模式联动）"
```

---

## Task 10: MainWindow 组装（树 + 堆叠面板 + 菜单 + 文件操作）

**目标**：用三层树 + QStackedWidget + GeneratePanel 组装完整主窗口，接通文件菜单（新建/打开/保存/另存为/导出 data.json）与编辑菜单（添加场/添加单元/删除）。

**Files:**
- Modify: `pyTool/gui/main_window.py`（替换 Task 1 的空壳）
- Modify: `pyTool/gui/tests/test_skeleton.py`（追加完整冒烟：建场、建单元、切换、保存）

**Interfaces:**
- Consumes: `ProjectModel`、`json_io`、各 Panel、`GeneratePanel`（前序任务）。
- Produces：可运行的 `MainWindow`，菜单「文件/编辑/帮助」齐全。

- [ ] **Step 1: 写失败测试**

在 `pyTool/gui/tests/test_skeleton.py` 末尾追加：
```python
import os
from models.project_model import ProjectModel


def test_main_window_add_field_and_save(tmp_path, qapp):
    win = MainWindow()
    win.newProject()                       # 新建空项目
    field = win._model.addField("ElDisp")
    win._model.addEleSub(field, "ElQ4g", gauss=True)
    win.refreshTree()
    assert win._tree.topLevelItemCount() == 1            # 项目根

    path = str(tmp_path / "p.cdfeg.json")
    assert win.saveAs(path)
    assert os.path.exists(path)

    # 重新打开
    assert win.openProject(path)
    assert win._model.project.fields[0].name == "ElDisp"
    win.deleteLater()


def test_main_window_switch_panel_on_select(qapp):
    win = MainWindow()
    win.newProject()
    f = win._model.addField("F")
    win._model.addEleSub(f, "T", gauss=False)
    win.refreshTree()
    # 选中单元叶节点（项目根→场→单元）
    root = win._tree.topLevelItem(0)
    field_node = root.child(0)
    ele_node = field_node.child(0)
    win._tree.setCurrentItem(ele_node)
    assert win._stack.currentIndex() == win._STACK_ELE
    win.deleteLater()
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `cd pyTool/gui && python -m pytest tests/test_skeleton.py -v`
Expected: FAIL（`AttributeError: 'MainWindow' object has no attribute 'newProject'` 等）。

- [ ] **Step 3: 写实现**

用以下完整内容**替换** `pyTool/gui/main_window.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# pyTool 配置式生成器主窗口
from PySide6.QtWidgets import (
    QMainWindow, QWidget, QSplitter, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QVBoxLayout, QFileDialog, QMessageBox, QInputDialog,
    QMenu, QLabel,
)
from PySide6.QtCore import Qt

from DataProject import DataProject
from DataEleSubG import DataEleSubG
from models.project_model import ProjectModel
from services import json_io
from views.project_panel import ProjectPanel
from views.field_panel import FieldPanel
from views.element_panel import ElementPanel
from views.generate_panel import GeneratePanel


class MainWindow(QMainWindow):
    _STACK_PROJ, _STACK_FIELD, _STACK_ELE = 0, 1, 2

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("pyTool 配置式生成器")
        self.resize(1100, 760)

        self._model = ProjectModel()
        self._currentFile = None

        # ---- 左：树 ----
        self._tree = QTreeWidget()
        self._tree.setHeaderLabels(["项目结构"])
        self._tree.setContextMenuPolicy(Qt.CustomContextMenu)

        # ---- 右：堆叠面板 ----
        self._projPanel = ProjectPanel(self._model)
        self._fieldPanel = FieldPanel(self._model)
        self._elePanel = ElementPanel(self._model)
        placeholder = QLabel("（请在左侧选择节点）")
        placeholder.setAlignment(Qt.AlignCenter)
        self._stack = QStackedWidget()
        self._stack.addWidget(self._projPanel)    # 0
        self._stack.addWidget(self._fieldPanel)   # 1
        self._stack.addWidget(self._elePanel)     # 2

        splitter = QSplitter(Qt.Horizontal)
        splitter.addWidget(self._tree)
        splitter.addWidget(self._stack)
        splitter.setStretchFactor(0, 1)
        splitter.setStretchFactor(1, 3)

        # ---- 下：生成面板 ----
        self._genPanel = GeneratePanel(self._model)
        self._genPanel.setDefaults("sample/NewProj", "FEMproject/CMakeLists.txt")

        central = QWidget()
        outer = QVBoxLayout(central)
        outer.addWidget(splitter, 3)
        outer.addWidget(self._genPanel, 1)
        self.setCentralWidget(central)

        self._tree.currentItemChanged.connect(self._onTreeCurrentChanged)
        self._model.dirtyChanged.connect(self._updateTitle)
        self._model.structureChanged.connect(self.refreshTree)

        self._buildMenus()
        self.newProject()
        self.refreshTree()
        self._tree.setCurrentItem(self._tree.topLevelItem(0))

    # ---- 菜单 ----
    def _buildMenus(self):
        mb = self.menuBar()
        fileM = mb.addMenu("文件")
        for text, slot in (("新建", self.newProject),
                           ("打开…", self._open),
                           ("保存", self._save),
                           ("另存为…", self._saveAs),
                           ("导出 data.json…", self._exportDataJson)):
            fileM.addAction(text, slot)

        editM = mb.addMenu("编辑")
        editM.addAction("添加场", self._addField)
        editM.addAction("添加单元", self._addEleSub)
        editM.addAction("删除选中", self._deleteSelected)

        mb.addMenu("帮助").addAction("关于", self._about)

    # ---- 文件操作 ----
    def newProject(self):
        self._model.setProject(DataProject("", 2))
        self._currentFile = None
        self.refreshTree()

    def openProject(self, path) -> bool:
        try:
            proj = json_io.load(path)
        except Exception as e:
            QMessageBox.critical(self, "打开失败", f"无法解析工程文件：\n{e}")
            return False
        self._model.setProject(proj)
        self._currentFile = path
        self.refreshTree()
        return True

    def _open(self):
        if not self._confirmDiscard():
            return
        path, _ = QFileDialog.getOpenFileName(
            self, "打开工程文件", "", "pyTool 工程 (*.cdfeg.json);;JSON (*.json)")
        if path:
            self.openProject(path)

    def saveAs(self, path=None) -> bool:
        if path is None:
            path, _ = QFileDialog.getSaveFileName(
                self, "另存为", f"{self._model.project.name or 'project'}.cdfeg.json",
                "pyTool 工程 (*.cdfeg.json)")
            if not path:
                return False
        json_io.save(self._model.project, path)
        self._currentFile = path
        self._model.markClean()
        return True

    def _save(self):
        if self._currentFile:
            self.saveAs(self._currentFile)
        else:
            self._saveAs()

    def _saveAs(self):
        self.saveAs()

    def _exportDataJson(self):
        path, _ = QFileDialog.getSaveFileName(
            self, "导出 data.json", "data.json", "JSON (*.json)")
        if path:
            json_io.save(self._model.project, path)

    def _confirmDiscard(self) -> bool:
        if not self._model.isDirty:
            return True
        btn = QMessageBox.question(
            self, "未保存", "当前项目有未保存修改，是否丢弃？",
            QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        return btn == QMessageBox.Yes

    # ---- 编辑操作 ----
    def _addField(self):
        name, ok = QInputDialog.getText(self, "添加场", "场名称：")
        if ok and name:
            self._model.addField(name)

    def _addEleSub(self):
        field = self._selectedField()
        if field is None:
            QMessageBox.information(self, "添加单元", "请先在左侧选择一个场节点。")
            return
        name, ok = QInputDialog.getText(self, "添加单元", "单元名称：")
        if ok and name:
            gauss_btn = QMessageBox.question(
                self, "单元类型", "是高斯积分单元（IsoEleBase）吗？",
                QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            gauss = (gauss_btn == QMessageBox.Yes)
            self._model.addEleSub(field, name, gauss=gauss)

    def _deleteSelected(self):
        item = self._tree.currentItem()
        if item is None:
            return
        data = item.data(0, Qt.UserRole)
        if data is None:
            return
        kind, obj = data
        if kind == "field":
            self._model.removeField(obj)
        elif kind == "ele":
            field = item.parent().data(0, Qt.UserRole)
            self._model.removeEleSub(field[1], obj)

    def _selectedField(self):
        item = self._tree.currentItem()
        if item is None:
            return None
        data = item.data(0, Qt.UserRole)
        if data and data[0] == "field":
            return data[1]
        if data and data[0] == "ele":
            parent = item.parent()
            pdata = parent.data(0, Qt.UserRole)
            return pdata[1] if pdata else None
        return None

    # ---- 树 ----
    def refreshTree(self):
        self._tree.clear()
        proj = self._model.project
        root = QTreeWidgetItem([f"{proj.name or '(未命名)'} (dim={proj.dim})"])
        root.setData(0, Qt.UserRole, ("project", proj))
        for field in proj.fields:
            fnode = QTreeWidgetItem([field.name or "(未命名场)"])
            fnode.setData(0, Qt.UserRole, ("field", field))
            for ele in field.eleSubs:
                tag = " [G]" if isinstance(ele, DataEleSubG) else ""
                enode = QTreeWidgetItem([f"{ele.name}{tag}"])
                enode.setData(0, Qt.UserRole, ("ele", ele))
                fnode.addChild(enode)
            root.addChild(fnode)
        self._tree.addTopLevelItem(root)
        self._tree.expandAll()
        self._updateTitle()

    def _onTreeCurrentChanged(self, cur, _prev):
        if cur is None:
            return
        data = cur.data(0, Qt.UserRole)
        if not data:
            return
        kind, obj = data
        if kind == "project":
            self._stack.setCurrentIndex(self._STACK_PROJ)
            self._projPanel.loadProject()
        elif kind == "field":
            self._stack.setCurrentIndex(self._STACK_FIELD)
            self._fieldPanel.loadField(obj)
        elif kind == "ele":
            field = cur.parent().data(0, Qt.UserRole)[1]
            self._stack.setCurrentIndex(self._STACK_ELE)
            self._elePanel.loadEleSub(field, obj)

    def _updateTitle(self):
        dirty = "*" if self._model.isDirty else ""
        name = self._model.project.name or "(未命名)"
        f = f" — {self._currentFile}" if self._currentFile else ""
        self.setWindowTitle(f"pyTool 配置式生成器 — {name}{dirty}{f}")

    def _about(self):
        QMessageBox.about(self, "关于", "pyTool 配置式生成器\nPySide6 GUI\nCDFEG 项目")

    def closeEvent(self, e):
        if self._confirmDiscard():
            e.accept()
        else:
            e.ignore()
```

- [ ] **Step 4: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_skeleton.py -v`
Expected: PASS（3 passed：原骨架 1 + 新增 2）。

- [ ] **Step 5: 全量回归**

Run: `cd pyTool/gui && python -m pytest -v`
Expected: 全部 PASS（骨架 3 + project_model 6 + json_io 2 + generate 3 + widgets 6 + panels 5 + generate_panel 2 = 27 passed）。

- [ ] **Step 6: 手动走查**

Run: `cd pyTool/gui && python main.py`
Expected：
1. 窗口打开，左侧树有「(未命名) (dim=2)」项目根；标题栏无 `*`。
2. 「编辑 → 添加场」→ 输入 `ElDisp`，树出现场节点，标题显 `*`。
3. 选中场 →「编辑 → 添加单元」→ 名 `ElQ4g`、选「是」（高斯），树出现 `ElQ4g [G]`，标题 `*`。
4. 选中 `ElQ4g [G]` → 右侧显示单元面板且高斯区可见；填参数。
5. 「文件 → 另存为」存为 `el2d.cdfeg.json` → 标题 `*` 消失。
6. 「文件 → 新建」→ 弹丢弃确认；确认后清空。
7. 「文件 → 打开」刚才的 `el2d.cdfeg.json` → 场与 `ElQ4g [G]` 完整恢复，高斯区可见。
8. 底部生成面板：选 `add` → sln 路径可编辑；选 `new` → sln 路径禁用。

- [ ] **Step 7: 提交**

```bash
git add pyTool/gui/main_window.py pyTool/gui/tests/test_skeleton.py
git commit -m "feat(pyTool-gui): 组装 MainWindow（三层树+堆叠面板+文件/编辑菜单）"
```

---

## Task 11: 端到端集成验证（Truss1D GUI vs 脚本产物 diff）

**目标**：用 Truss1D 在 GUI 数据路径下生成，与 `test/test1DTruss.py` 直接产物对比，确保 GUI 路径与脚本路径**逐字节一致**（排除复制的 CDFEG/ 与 third/ 库目录）。

**Files:**
- Create: `pyTool/gui/tests/test_e2e_truss1d.py`

**Interfaces:**
- Consumes: `generate`（Task 4）、`MakerCpp`（现有）。

- [ ] **Step 1: 写测试（即验证脚本）**

创建 `pyTool/gui/tests/test_e2e_truss1d.py`：
```python
# SPDX-License-Identifier: GPL-3.0
# 端到端：GUI 数据路径 vs test1DTruss.py 脚本路径产物 diff
import filecmp
import os
import shutil

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from MakerCpp import MakerCpp


def _buildTruss1DProject() -> DataProject:
    """复刻 test/test1DTruss.py 的数据结构。"""
    project = DataProject("Truss1D", 1)
    field = DataField("Truss1DDisp")
    ele = DataEleSub("Truss1D")
    ele.dispNames = ["u"]
    ele.eleResNames = ["T"]
    ele.paramNames = ["E", "A"]
    field.addEleSub(ele)
    project.addField(field)
    project.cmds.append(("imp", 0))
    return project


def _projectFiles(root):
    """收集项目代码文件（排除复制的 CDFEG/ 与 third/ 库）。"""
    out = {}
    for dirpath, _, files in os.walk(root):
        rel = os.path.relpath(dirpath, root)
        if rel.startswith("CDFEG") or rel.startswith("third"):
            continue
        for fn in files:
            p = os.path.join(dirpath, fn)
            out[os.path.relpath(p, root).replace("\\", "/")] = p
    return out


def test_gui_path_matches_script_path(tmp_path):
    proj_for_gui = _buildTruss1DProject()      # GUI 路径用相同数据
    proj_for_script = _buildTruss1DProject()   # 脚本路径用相同数据

    gui_dir = str(tmp_path / "gui_out")
    script_dir = str(tmp_path / "script_out")

    # GUI 路径：直接经 MakerCpp（generate.run 内部即此调用）
    mk = MakerCpp(proj_for_gui, gui_dir, mode="new")
    mk.mainMode = 0
    mk.makeAll()

    # 脚本路径：完全照搬 test1DTruss.py 的调用
    mk2 = MakerCpp(proj_for_script, script_dir, mode="new")
    mk2.mainMode = 0
    mk2.makeAll()

    gui_files = _projectFiles(gui_dir)
    script_files = _projectFiles(script_dir)
    assert set(gui_files.keys()) == set(script_files.keys()), \
        f"文件列表不一致:\n仅GUI:{set(gui_files)-set(script_files)}\n仅脚本:{set(script_files)-set(gui_files)}"

    diffs = []
    for rel, gp in gui_files.items():
        if not filecmp.cmp(gp, script_files[rel], shallow=False):
            diffs.append(rel)
    assert diffs == [], f"以下文件内容不一致: {diffs}"
```

- [ ] **Step 2: 运行测试，确认通过**

Run: `cd pyTool/gui && python -m pytest tests/test_e2e_truss1d.py -v`
Expected: PASS。若 FAIL，根据报告的 diff 文件排查 GUI 数据结构与脚本是否一致（最常见：`paramNames`/`dispNames`/`eleResNames` 漏填或 `cmds` 缺失）。

- [ ] **Step 3: 全量回归确认**

Run: `cd pyTool/gui && python -m pytest -v`
Expected: 全部 PASS（28 passed）。

- [ ] **Step 4: 更新 pyTool CLAUDE.md（记录 GUI 入口）**

Modify `pyTool/CLAUDE.md` 的「入口」一节，在现有"每个 test/test*.py 既是测试也是生成入口"后追加一行：
```
- GUI：`python pyTool/gui/main.py`（PySide6 配置式生成器，可视化编辑三层结构 → 存档 .cdfeg.json → 一键生成；详见 `docs/superpowers/specs/2026-07-18-pytool-gui-design.md`）。
```

- [ ] **Step 5: 提交**

```bash
git add pyTool/gui/tests/test_e2e_truss1d.py pyTool/CLAUDE.md
git commit -m "test(pyTool-gui): 端到端验证 Truss1D GUI 路径与脚本产物逐字节一致"
```

---

## Self-Review 记录（plan 作者执行）

**1. Spec 覆盖**：逐条对照 spec——
- 配置式生成器定位 → 全计划。
- 单项目会话 → `MainWindow` 单 `ProjectModel`。
- 不编辑代码片段 → ElementPanel 无 `runCode` 入口（Global Constraints 声明）。
- 独立工程文件 → `json_io` + `.cdfeg.json` + 文件菜单。
- 固定生成面板 → `GeneratePanel`。
- 表格/列表编辑 → `ListEditor`/`TableEditor`。
- 添加单元子类型 → `addEleSub(gauss=...)` + 树 `[G]` 标记 + 高斯区显隐。
- G 单元分发重建 → `json_io._buildEle` + round-trip 测试。
- 错误处理（未保存确认/异常不崩/字段校验） → MainWindow `_confirmDiscard`/closeEvent、generate 异常捕获、GeneratePanel 空路径校验。
- 测试策略（pytest round-trip + monkeypatch + e2e diff） → Task 3/4/11。

**2. 占位符扫描**：无 TBD/TODO/「适当处理」；每步均含实际代码或实际命令。

**3. 类型一致性**：
- `ProjectModel` 方法名（`addField/addEleSub/removeField/removeEleSub/markDirty/markClean/setProject/isDirty/fromProject/dirtyChanged/structureChanged`）在 Task 2 定义，Task 7/8/9/10 使用一致。
- `json_io.save/load/EXT`、`generate.run` 签名、`ListEditor.items/setItems/itemsChanged`、`TableEditor.rows/setRows/rowsChanged`、面板 `loadProject/loadField/loadEleSub` 均跨任务一致。
- `_STACK_PROJ/FIELD/ELE` 常量 Task 10 定义即用。
