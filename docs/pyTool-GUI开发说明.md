# pyTool 配置式生成器 GUI 开发说明

> **文档日期**：2026-07-19
> **工作周期**：2026-07-18 ~ 2026-07-19
> **基线 commit**：`e5d87c4`　**终点 commit**：`8447b8d`（共 15 个 commit，本地 main 分支）
> **关联文档**：[设计 spec](./superpowers/specs/2026-07-18-pytool-gui-design.md)、[实现计划](./superpowers/plans/2026-07-18-pytool-gui.md)

---

## 一、项目概述

### 1.1 目标

为 `pyTool/` 有限元代码生成工具链增加一个 **PySide6 桌面界面**，可视化编辑有限元项目三层结构（项目 → 场 → 单元），存档为工程文件，一键调用既有生成器产出 C++/CMake/GiD 代码，**替代此前每个示例手写一份 `test/testXxx.py` 脚本**的现状。

### 1.2 定位

**配置式生成器**（非集成开发工作台）。GUI 只负责元数据与数值参数的编辑；**不编辑代码片段**（`runCode`/`uCode`/`shapeFunCode` 等留空字符串），延续本项目「pyTool 生成框架 + 人工填充计算逻辑」的一贯模式。

### 1.3 技术栈

- **GUI**：PySide6 6.11.1（Qt 官方 Python 绑定，LGPL；由 PyQt5 调整而来）
- **运行**：Python 3.13.3，Windows 11 + Git Bash
- **测试**：pytest 9.0.2，GUI 测试用 `QT_QPA_PLATFORM=offscreen` 无头运行
- **数据层**：直接复用 pyTool 既有 `DataProject` / `DataField` / `DataEleSub(G)` 及其 `toDict()/fromDict()`

### 1.4 执行方式

采用 **subagent-driven development**（每任务派发独立子代理实现 → 子代理审查 → 控制器裁决 → 进入下一任务），全程 11 个任务 + 3 处任务级修正 + 1 次 final-review 清理。

---

## 二、完成的工作

### 2.1 任务总览

| # | 任务 | 主 commit | 测试用例 |
|---|---|---|---|
| T1 | 项目骨架与可启动空主窗口 | `56208b7` | skeleton 1 |
| T2 | ProjectModel（ViewModel） | `9a0dc19` | project_model 6 |
| T3 | json_io service（G 单元分发重建） | `5cfb198` | json_io 2 |
| T4 | generate service（Maker 封装） | `f831226` | generate 3 |
| T5 | ListEditor 可复用 widget | `1a48a81` | widgets 3 |
| T6 | TableEditor 可复用 widget | `d699d91` | widgets 3 |
| T7 | ProjectPanel + FieldPanel | `3b975a1` | panels 2 |
| T8 | ElementPanel（含 DataEleSubG 高斯分支） | `ee62061` | panels 3 |
| T9 | GeneratePanel | `c1a9da9` | generate_panel 2 |
| T10 | MainWindow 组装 | `7184795` | skeleton +2 |
| T11 | 端到端 Truss1D diff 验证 | `e3bebbf` | e2e 1 |

**合计 29 个 pytest 用例全绿**，覆盖 ViewModel、Service、Widget、面板、端到端五个维度。

### 2.2 各任务详解

#### T1 项目骨架

建立 `pyTool/gui/` 目录结构、空 `MainWindow`、`conftest.py`（统一 offscreen 设置、sys.path 注入 `pyTool/gui` 与 `pyTool` 根、会话级 `qapp` fixture）、入口 `main.py`。后续 10 个任务在此骨架上叠加。

#### T2 ProjectModel（核心 ViewModel）

`ProjectModel(QObject)` 包装一个 `DataProject`，提供：

- 脏标记 `isDirty` + 信号 `dirtyChanged(bool)`、`structureChanged()`
- 增删：`addField(name)`、`addEleSub(field, name, gauss)`、`removeField/removeEleSub`
- `addEleSub(gauss=True)` 创建 `DataEleSubG`（`baseClass='IsoEleBase'`），`gauss=False` 创建 `DataEleSub`
- `setProject/fromProject/toProject/markDirty/markClean`

它是所有编辑面板的数据后端。

#### T3 json_io service（G 单元分发重建）

工程文件读写 `save/load`，扩展名 `.cdfeg.json`。

**关键约束**：`load` **禁止**直接用 `DataField.fromDict`——经核实其源码内部写死 `DataEleSub.fromDict(ele_data)`，会把 `DataEleSubG` 退化为基类、丢失 `gaussPoints/gaussWeights/shapeFuns`。`json_io._buildEle` 改为按 `baseClass=='IsoEleBase'`（或字典含 `gaussPoints` 键）分发到 `DataEleSubG.fromDict` / `DataEleSub.fromDict`，确保高斯字段 round-trip 零丢失（7 个断言逐字段验证）。

#### T4 generate service

`generate.run(project, mode, mainMode, outPath, sln_cmake_path, log)` 封装 `MakerCpp` + `MakerGidFile`：

- 用 `contextlib.redirect_stdout` 捕获 `safePrint`（即 `print`）输出到日志回调
- `mainMode==1` 时额外调用 `MakerGidFile`
- `except Exception` 捕获、写 traceback、返回 `(False, text)`，**进程不崩**
- `MakerCpp/MakerGidFile` **模块级 import**，测试靠 `monkeypatch` 替换

#### T5 ListEditor / T6 TableEditor（可复用 widget）

- `ListEditor(QWidget)`：字符串列表编辑器（新增/删除/上移/下移），`setItems/items/itemsChanged`
- `TableEditor(QWidget)`：行列可变表格编辑器，`setRows/rows/rowsChanged`，`rows()` 剔除全空行

二者供 `ElementPanel` 编辑 `dispNames`/`eleResNames`/`paramNames`/`gaussPoints`/`gaussWeights`/`shapeFuns`。

#### T7 ProjectPanel + FieldPanel

- `ProjectPanel`：编辑 `name`/`dim`，`coordVars` 按 `dim` 自动派生展示
- `FieldPanel`：编辑 `name`/`pdeType`/`bDynamic`，`dispNames`/`eleResNames` 由其单元聚合只读展示

编辑回写后调 `markDirty`。

#### T8 ElementPanel（含 G 分支）

单元字段编辑面板。公共字段区（name/nNodes/type/dim/bBC/baseClass/gidName + dispNames/eleResNames + paramNames&paramValues 表格）对所有单元；当单元为 `DataEleSubG` 时额外显示高斯区（gaussOrder/gaussPoints 表格/gaussWeights/shapeFuns）。`loadEleSub` 回填、`_commit` 写回。

#### T9 GeneratePanel

底部生成参数面板：mode(new/add)、mainMode(0/1)、outPath(浏览)、sln_cmake_path（仅 add 时启用）。「▶ 生成」调 `generate.run`，日志写只读 `QPlainTextEdit`，空 outPath 校验。

#### T10 MainWindow 组装

完整主窗口：

- 左侧三层 `QTreeWidget`（项目 → 场 → 单元，G 单元带 `[G]` 标记），右键增删
- 右侧 `QStackedWidget` 按选中节点切换 Project/Field/Element 面板
- 底部 `GeneratePanel`
- 文件菜单：新建/打开/保存/另存为/导出 `data.json`/丢弃确认
- 编辑菜单：添加场/添加单元（**含普通 vs 高斯子类型询问**）/删除选中
- `closeEvent` 守卫未保存修改

#### T11 端到端 Truss1D 验证

`test_e2e_truss1d.py`：GUI 数据路径（经 `json_io` 存档/加载 round-trip + `generate.run`，即 GUI 实际调用链）与 `test/test1DTruss.py` 脚本路径（裸 `MakerCpp`）产物对比，排除复制的 `CDFEG/` 与 `third/` 库目录后**逐字节一致**。这是 GUI 生成链正确性的关键证据。

---

## 三、修正的缺陷

开发过程中经「子代理实现 → 子代理审查 → 控制器裁决」流程发现并修复 4 类缺陷（含 1 个 Critical）。

### 3.1 GPL 头缺失（T1，Minor→已修）

**现象**：`pyTool/gui/__init__.py` 与 `tests/__init__.py` 缺少 `SPDX-License-Identifier: GPL-3.0` 头，与 pyTool 现有所有 `.py` 文件惯例不一致。

**根因**：实现计划 brief 在这两个文件只写了简短注释，遗漏 GPL 头；实现子代理忠实执行了 brief。

**修复**（`06c771b`）：补完整 15 行 GPL-3.0 头。**教训已传递给后续所有任务的 implementer**，后续新建文件均带完整头。

### 3.2 TableEditor.setRows 信号污染（T6→影响 T8，Minor→已修）

**现象**：`TableEditor.setRows` 在填充时触发 `rowsChanged` 信号（因内部 `setItem` 触发 `itemChanged`），与 `ListEditor.setItems` **不触发**信号的行为不一致。

**根因**：实现时 `itemChanged` 直接连接 `rowsChanged.emit`，未在编程式 `setRows` 时屏蔽。

**后果**：`ElementPanel.loadEleSub` 回填时调用 `_params.setRows` 会误触发 `_commit → markDirty`，导致「加载项目即被标脏」。

**修复**（`4efb8fd`）：`setRows` 用 `self._table.blockSignals(True/False)` 包裹，使其不触发 `rowsChanged`，与 `ListEditor.setItems` 行为对齐。仅 2 行改动，6 passed。

### 3.3 ElementPanel `_commit` 高斯空串崩溃（T8，**Critical**→已修）

**现象**：`_commit` 中
```python
ele.gaussPoints = [[float(x) for x in r] for r in self._gaussPoints.rows()]
```
`_gaussPoints` 是**固定 3 列**表格（x1/x2/x3）。2D 单元积分点 `[0.5, 0.5]` 写入后第 3 列为空串，`float("")` 抛 `ValueError`。**用户加载或编辑任意 2D/1D G 单元都会崩溃**。

**根因**：测试只断言了回填后的 UI 显示状态，未做 `_commit` round-trip，未捕获该路径。

**修复**（`04fd913`）：
- `gaussPoints`/`gaussWeights` 的 `float` 转换加 `if x.strip()` 过滤空串
- `baseClass` 下拉设为 `setEnabled(False)` 只读（单元类型由实例决定，不应可改）
- 新增回归测试 `test_element_panel_gauss_commit_filters_empty`（2D G 单元 `_commit` 不崩且正确剔除空列）

### 3.4 final-review follow-up 清理（`8447b8d`）

final whole-branch review 判定 **Ready to merge: Yes**，但指出若干非阻塞 follow-up，治本清理：

| 项 | 问题 | 治本修复 |
|---|---|---|
| **e2e 虚假安全感（最重要）** | `test_gui_path_matches_script_path` 两路径都裸调 `MakerCpp.makeAll()` 且输入相同，diff 必然为空——**测试不可能失败**，没真正走 GUI 链路 | 改「GUI 路径」为 `json_io.save → json_io.load → generate.run`（GUI 真实调用链），脚本路径保持裸 `MakerCpp`。现真正覆盖「GUI 数据→存档→加载→生成」全链路 |
| `_add` 生产 code smell | `ListEditor._add` 含 `QT_QPA_PLATFORM=="offscreen"` 分支以避开测试时 `QInputDialog` 阻塞——生产代码嗅探测试环境 | 移除该分支，恢复 `_add` 纯粹逻辑；改 `test_list_editor_emits_changed` 用 `_remove` 触发 `itemsChanged` |
| 死代码 | `MainWindow` 的 `placeholder QLabel` 创建未加入布局；冗余 `QMenu` import | 删除 |
| **`cmds` 序列化缺口** | `DataProject.toDict/fromDict` 不含 `cmds` 字段，Truss1D（依赖 `cmds=[("imp",0)]`）经 `json_io` round-trip 后会丢失命令流 | `json_io` 层补全：`save` 写 `data["cmds"]`、`load` 读 `project.cmds`。e2e 现经 round-trip 仍逐字节一致即证明 cmds 正确恢复 |
| GPL 头风格不一 | `test_generate.py`/`test_skeleton.py` 仅单行 SPDX 标识 | 统一为完整 15 行块 |

> **`cmds` 缺口的后续注意**：`cmds` 序列化当前在 `json_io`（GUI 层）补全。若核心库 `DataProject` 未来在 `toDict/fromDict` 原生支持 `cmds`，需同步移除 `json_io` 层补丁，避免重复写入。

---

## 四、关键设计决策

1. **复用而非重造数据层**：GUI 直接持有并操作既有 `DataProject`/`DataField`/`DataEleSub(G)` 对象，不重复定义字段，保证与生成器、与既有脚本的语义完全一致（e2e 逐字节一致即证明）。

2. **G 单元分发重建**：识别并绕开 `DataField.fromDict` 的退化 bug，在 `json_io` 按类型分发重建，是整个存档/加载可靠性的基石。

3. **独立工程文件**：`<name>.cdfeg.json` 内容即 `DataProject.toDict()`，与既有 `data.json` 同构，可互导；扩展名仅作区分。

4. **不编辑代码片段**：GUI 只生成框架骨架，计算逻辑由开发者人工填入生成后的源文件，与 pyTool 既有协作模式一致。

5. **offscreen 无头测试**：所有 GUI 测试在 `QT_QPA_PLATFORM=offscreen` 下运行，无需显示设备，可在 CI/无头环境执行；关键交互（模态对话框）通过避开或改测触发方式保证可自动验证。

6. **subagent-driven 开发**：每任务 fresh 子代理 + 独立审查 + 控制器裁决，缺陷在任务边界即被发现修复（如 T8 Critical 在 review 阶段捕获，未流入下游）。

---

## 五、测试与验证

- **29 个 pytest 用例全绿**，分布：
  - `test_skeleton.py`：3（主窗口构造、增场存档加载、面板切换）
  - `test_project_model.py`：6（增删改、脏标记、信号）
  - `test_json_io.py`：2（round-trip 含 G 单元、坏 JSON 异常）
  - `test_generate.py`：3（new 模式参数、mainMode=1 调 GidFile、异常返回 False）
  - `test_widgets.py`：6（ListEditor 3 + TableEditor 3）
  - `test_panels.py`：6（Project/Field 2 + Element 3 + 高斯空串回归 1）
  - `test_generate_panel.py`：2（mode 联动、生成调 service）
  - `test_e2e_truss1d.py`：1（GUI 全链路 vs 脚本路径逐字节一致）
- **端到端**：Truss1D 经 `json_io round-trip → generate.run` 产出与 `test1DTruss.py` 直接产出排除库目录后逐字节一致。
- **GUI 视觉布局**：按 spec 声明以手动走查为主（PySide 控件自动化测试 ROI 低）。

运行测试：
```bash
cd pyTool/gui && python -m pytest -v
```

---

## 六、运行方式

启动 GUI：
```bash
python pyTool/gui/main.py
```

典型流程：
1. 「编辑 → 添加场」→ 输入场名
2. 选中场 → 「编辑 → 添加单元」→ 输入单元名，选择是否高斯积分单元
3. 选中单元 → 右侧面板填写 name/nNodes/type/位移名/参数等（高斯单元额外填积分点/权重/形函数）
4. 「文件 → 另存为」存为 `<name>.cdfeg.json`
5. 底部生成面板：选 mode（new/add）、mainMode（0/1）、outPath，点「▶ 生成」，日志区查看过程
6. 生成后的源文件人工填充 `run`/`uEle` 等计算逻辑

---

## 七、已知 follow-up（非阻塞）

以下 Minor 项已记录、不影响使用，可日后机遇清理：

- `cmds` 序列化在 `json_io` 层补全（见 §3.4 注意事项）
- 部分 `__init__` 构造函数缺类型注解；个别测试文件 import 位置非顶部
- `ProjectModel.addEleSub` 增加了 brief 未要求的默认参数（无害便利）
- `json_io.load` 未恢复 `dof2`/`headerGuard` 可选字段（GUI 用户不设，`eleResNames` 由 `makeData` 重建，影响有限）

---

## 八、文件清单

```
pyTool/gui/
├── main.py                      # QApplication 入口
├── main_window.py               # MainWindow（菜单/树/堆叠面板/生成面板）
├── conftest.py                  # offscreen + sys.path + qapp fixture
├── models/
│   └── project_model.py         # ProjectModel（ViewModel）
├── services/
│   ├── json_io.py               # 工程文件读写（G 单元分发重建 + cmds 补全）
│   └── generate.py              # Maker 封装 + 日志重定向 + 异常捕获
├── widgets/
│   ├── list_editor.py           # ListEditor
│   └── table_editor.py          # TableEditor
├── views/
│   ├── project_panel.py         # ProjectPanel
│   ├── field_panel.py           # FieldPanel
│   ├── element_panel.py         # ElementPanel（含 G 分支）
│   └── generate_panel.py        # GeneratePanel
└── tests/
    ├── test_skeleton.py
    ├── test_project_model.py
    ├── test_json_io.py
    ├── test_generate.py
    ├── test_widgets.py
    ├── test_panels.py
    ├── test_generate_panel.py
    └── test_e2e_truss1d.py
```

---

## 九、Commit 记录（`e5d87c4..8447b8d`，15 个）

| commit | 类型 | 说明 |
|---|---|---|
| `56208b7` | feat | 搭建 PySide6 骨架与 offscreen 测试环境（T1） |
| `06c771b` | fix | 补 `__init__.py` 的 GPL-3.0 头（T1 fix） |
| `9a0dc19` | feat | 实现 ProjectModel（T2） |
| `5cfb198` | feat | 实现 json_io（G 单元分发重建）（T3） |
| `f831226` | feat | 实现 generate service（T4） |
| `1a48a81` | feat | 实现 ListEditor（T5） |
| `d699d91` | feat | 实现 TableEditor（T6） |
| `4efb8fd` | fix | TableEditor.setRows 不触发 rowsChanged（T6 fix） |
| `3b975a1` | feat | 实现 ProjectPanel 与 FieldPanel（T7） |
| `ee62061` | feat | 实现 ElementPanel（含 G 高斯分支）（T8） |
| `04fd913` | fix | ElementPanel `_commit` 过滤高斯空串 + baseClass 只读（T8 fix，Critical） |
| `c1a9da9` | feat | 实现 GeneratePanel（T9） |
| `7184795` | feat | 组装 MainWindow（T10） |
| `e3bebbf` | test | 端到端验证 Truss1D GUI 路径与脚本产物一致（T11） |
| `8447b8d` | refactor | 清理 final-review follow-up（e2e 走 GUI 链路 + 移除 smell + 删死代码） |

> 以上 commit 均在本地 `main` 分支，**未推送**（按用户「保持本地」选择）。
