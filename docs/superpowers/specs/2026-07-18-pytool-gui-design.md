# pyTool 配置式生成器 GUI 设计

**⚠️ 术语变更（2026-07-29）**：本文档中的 `gidName` 已重命名为 `matName`（pyTool `DataEleSub` 类）。下文出现的 `gidName` 均指现今的 `matName`，原文保留未改。

- **日期**：2026-07-18
- **状态**：已确认（待实现）
- **范围**：为 `pyTool/` 新增一个 PySide6 桌面界面，可视化编辑有限元项目结构并一键生成 C++/CMake/GiD 代码

---

## 1. 背景与动机

`pyTool` 当前无统一 CLI：每个示例对应一个手写的 `test/testXxx.py` 脚本，在脚本里用 Python 代码拼装 `DataProject → DataField → DataEleSub(G)` 三层结构，再调 `MakerCpp.makeAll()` + `MakerGidFile.makeAll()` 生成代码。

痛点：

1. 单元字段多（`dispNames` / `eleResNames` / `paramNames` / `gaussPoints` / `gaussWeights` / `shapeFuns` …），手写脚本易错。
2. 每个示例一份脚本，复用与回看成本高。
3. 无项目存档机制——尽管 `DataProject.toDict()/fromDict()` 已具备 JSON 序列化能力（`testEl2D.py` 末尾已实践导出 `data.json`），但缺乏加载/再编辑入口。

**目标**：用一个桌面 GUI 替代手写脚本，可视化完成「配置三层结构 → 存档 → 一键生成」，降低出错率、提升复用性。

---

## 2. 范围

### 2.1 本期纳入

- 可视化编辑三层结构（项目 / 场 / 单元）的全部**元数据与数值参数**字段。
- 项目存档为独立工程文件 `<name>.cdfeg.json`，支持新建 / 打开 / 保存 / 另存为。
- 底部固定生成面板：暴露 `mode` / `mainMode` / `outPath` / `sln_cmake_path`，一键调用 `MakerCpp` + `MakerGidFile`，日志输出到面板。
- 列表型字段（`paramNames`、`gaussPoints`、`gaussWeights`、`shapeFuns` 等）用表格 / 列表编辑器增删改。

### 2.2 本期明确排除（YAGNI）

| 排除项 | 原因 |
|---|---|
| **代码片段编辑**（`runCode` / `uCode` / `shapeFunCode` / `initCode` / `coordTransFunCode`） | 符合项目「pyTool 生成框架 + 人工填充计算逻辑」模式；GUI 只生成骨架，代码片段留空字符串，由开发者人工填入生成后的源文件。 |
| CMake 构建 / 可执行程序运行 | 属于「集成开发工作台」定位，超出本期「配置式生成器」范围。 |
| GiD 结果可视化 | 同上。 |
| 多项目工作区 / 批量生成 | 本期为单项目会话；多项目管理留待后续。 |
| 打包发布 / 国际化 | 作者自用，中文界面即可。 |

---

## 3. 技术栈

- **GUI 框架**：PySide6（Qt 官方 Python 绑定，LGPL，Qt6）。用户指定由 PyQt5 调整而来。
- **依赖**：仅 PySide6 + 现有 pyTool 模块，不引入其他第三方库。
- **Python**：与 pyTool 现有一致（Python 3）。
- **界面语言**：简体中文。

---

## 4. 架构

严格复用现有 pyTool 数据层，**不重造数据模型**，分三层：

```
View        PySide6 窗口 / 面板（MainWindow + 各 Panel + 可复用 widget）
    ↕  Qt 信号/槽
ViewModel   ProjectModel：当前 DataProject 的包装，持有脏标记、提供增删改查、发射变更信号
    ↕  直接持有对象引用
Model       DataProject / DataField / DataEleSub(G) + toDict()/fromDict()   ← 现有，不改
    ↕
Service     json_io（工程文件读写）/ generate（封装 MakerCpp + MakerGidFile + 日志重定向）
```

设计原则：

- **DRY**：复用现有 `DataXxx` 与 `toDict/fromDict`，GUI 不重复定义字段。
- **SOLID-S/D**：面板只负责展示与采集输入，业务逻辑在 `ProjectModel` / `Service`；面板与模型经 Qt 信号解耦。
- **KISS**：无代码编辑器、无构建集成，每个文件单一职责。

---

## 5. 主窗口布局

采用「左侧三层树 + 右侧堆叠面板」形态（与三层数据模型同构）。

```
┌─ pyTool 配置式生成器 ─────────────────────────────[─][□][×]┐
│ 文件  编辑  生成  帮助                                       │
│ [新建][打开][保存]        [添加场][添加单元][▶ 生成]         │
├──────────────────┬─────────────────────────────────────────┤
│ 项目树           │ 编辑面板（随选中节点切换）                │
│ ▼ Truss1D (1D)   │ ┌─ 单元: Truss1D ─────────────────────┐ │
│   ▼ Truss1DDisp  │ │ 名称 [Truss1D]  节点数 [2]           │ │
│     • Truss1D    │ │ 类型 [线]  基类 [EleSubBase]          │ │
│                  │ │ 边界单元 □   GiD名 [Truss1D]          │ │
│                  │ │ ── 广义位移 ──   ── 参数(名/值) ──    │ │
│                  │ │ [u         ]↑↓+x [E | (空)]↑↓+x       │ │
│                  │ │                  [A | (空)]           │ │
│                  │ │ (G 单元额外：高斯点表格/权重/形函数)  │ │
│                  │ └──────────────────────────────────────┘ │
├──────────────────┴─────────────────────────────────────────┤
│ 生成：mode[add▼] mainMode[GiD▼] outPath[.../sample/Truss1D][…] │
│       sln_cmake_path[FEMproject/CMakeLists.txt]  [▶ 生成]      │
├───────────────────────────────────────────────────────────────┤
│ 日志：[14:03] 开始生成...                                     │
│       MakerCpp: 写入 sample/Truss1D/main.cpp ...              │
│       [完成] 12 文件，0.3s                                    │ ← 只读
└───────────────────────────────────────────────────────────────┘
```

### 5.1 菜单与工具栏

- **文件**：新建、打开、保存、另存为、导出 `data.json`（写到 outPath，兼容 `testEl2D.py` 既有实践）、最近打开、退出。
- **编辑**：添加场、添加单元、删除选中、重命名。
- **生成**：生成、生成并打开输出目录。
- **帮助**：关于。

### 5.2 左侧项目树（QTreeWidget）

三层：项目根 → 场节点 → 单元节点。节点文本附带关键摘要（如 `Truss1D (1D)`、场名、单元名）。右键菜单：添加 / 删除 / 重命名。选中节点 → 右侧切换对应面板。

**添加单元时弹子类型选择**：

- **普通单元** → 实例化 `DataEleSub`（`baseClass='EleSubBase'`），面板不显示高斯区；
- **高斯积分单元** → 实例化 `DataEleSubG`（`baseClass='IsoEleBase'`），面板显示高斯区。

存档时该子类型由 `baseClass` 字段标识；加载时据此重建对应类（见第 6 节）。

### 5.3 右侧编辑面板（QStackedWidget）

三个面板，按选中节点类型切换：

**项目面板**

| 字段 | 控件 | 说明 |
|---|---|---|
| `name` | QLineEdit | 项目名 |
| `dim` | QComboBox(1/2/3) | 总体维度 |
| `coordVars` | 只读标签 | 由 `dim` 自动派生（`['x']` / `['x','y']` / `['x','y','z']`） |

**场面板**

| 字段 | 控件 | 说明 |
|---|---|---|
| `name` | QLineEdit | 场名 |
| `pdeType` | QComboBox(1椭圆/2抛物/3双曲) | PDE 类型 |
| `bDynamic` | QCheckBox | 是否动力学 |
| `dispNames` / `eleResNames` | 只读标签 | 由其下属单元聚合派生，不可直接编辑 |

**单元面板**

公共字段（`DataEleSub`）：

| 字段 | 控件 |
|---|---|
| `name` | QLineEdit |
| `nNodes` | QSpinBox |
| `type` | QComboBox(0点/1线/2面/3体) |
| `dim` | QSpinBox |
| `bBC` | QCheckBox（是否边界单元） |
| `baseClass` | QComboBox(EleSubBase / IsoEleBase) |
| `gidName` | QLineEdit |
| `dispNames` | ListEditor（增删 / 上下移） |
| `eleResNames` | ListEditor |
| `paramNames` + `paramValues` | TableEditor（两列：名 / 默认值，行可增删） |

`DataEleSubG`（高斯积分单元）额外字段：

| 字段 | 控件 |
|---|---|
| `gaussOrder` | QSpinBox |
| `gaussPoints` | TableEditor（列数 = 单元 `dim`，每行一个积分点坐标） |
| `gaussWeights` | ListEditor（数值） |
| `shapeFuns` | ListEditor（字符串，每行一个形函数表达式） |

> 单元面板根据 `baseClass == 'IsoEleBase'`（即 `DataEleSubG`）动态显隐高斯区。

### 5.4 底部生成面板

| 控件 | 说明 |
|---|---|
| `mode` | QComboBox(`new` / `add`) |
| `mainMode` | QComboBox(`0=makeData` / `1=GiD`) |
| `outPath` | QLineEdit + 浏览按钮（选目录） |
| `sln_cmake_path` | QLineEdit，仅 `mode='add'` 时启用 |
| `▶ 生成` | 触发生成 |
| 日志区 | QPlainTextEdit（只读），显示时间戳 + 生成过程 + 结果 |

`mode='add'` 时 `outPath` 通常为 `sample/<Proj>`，`sln_cmake_path` 为 `FEMproject/CMakeLists.txt`——面板提供这两个默认值。

---

## 6. 数据流

- **编辑**：面板控件 `editingFinished` / `dataChanged` → `ProjectModel` 修改对应 `DataXxx` 对象 → 置 `dirty=True` → 主窗口标题栏追加 `*`。
- **树切换**：树当前选中变化 → `ProjectModel` 取对应 `DataXxx` → 回填右侧面板（面板从模型读，不自己缓存状态）。
- **保存**：`ProjectModel` → `DataProject.toDict()` → `json_io.save()` 写 `<name>.cdfeg.json` → `dirty=False`。
- **打开**：`json_io.load()` 读取 JSON → 重建 `DataProject`。**注意：不能直接用 `DataField.fromDict()`**——其内部写死调用 `DataEleSub.fromDict(ele_data)`，会把 `DataEleSubG` 退化为基类、丢失 `gaussPoints` / `gaussWeights` / `shapeFuns`。`json_io` 必须自行遍历场与单元，按单元字典的 `baseClass`（或是否含 `gaussPoints` 键）判断，分别调用 `DataEleSubG.fromDict` / `DataEleSub.fromDict` 重建。重建后 → `ProjectModel` 替换当前项目 → 重建树 → 选中最顶层节点。
- **生成**：`generate.run(project, mode, mainMode, outPath, sln_cmake_path, log_sink)` → 内部 `MakerCpp(...).makeAll()` + `MakerGidFile(...).makeAll()`，`stdout/stderr` 经 `contextlib.redirect_stdout` 重定向到日志面板；异常捕获写入日志，不上抛。

---

## 7. 工程文件格式

`<name>.cdfeg.json` 内容即 `DataProject.toDict()` 的纯 JSON。扩展名 `.cdfeg.json` 仅用于在文件管理器中与普通 `data.json` 区分；内容与现有 `data.json` 同构，可直接被 `DataProject.fromDict()` 加载，也可被现有脚本复用。

---

## 8. 错误处理

| 场景 | 处理 |
|---|---|
| 未保存而新建 / 打开 / 退出 | `dirty=True` 时弹确认对话框（保存 / 丢弃 / 取消） |
| 生成抛异常 | 日志面板写 traceback，状态栏红字提示，**进程不崩**，保留当前编辑状态 |
| 生成前校验 | 项目 / 场 / 单元 `name` 非空、`nNodes > 0`、`outPath` 非空；失败时定位并高亮对应树节点，日志列出第一处错误 |
| 工程文件格式损坏 | `fromDict` 失败时弹框提示具体原因，不加载 |

---

## 9. 测试策略

| 层 | 方式 | 内容 |
|---|---|---|
| Model / Service | pytest | ① round-trip：`toDict → json_io 自有 load`（非 `DataField.fromDict`）后全部字段无丢失，**重点覆盖 `DataEleSubG` 的 `gaussPoints` / `gaussWeights` / `shapeFuns`**（直接 `DataField.fromDict` 会丢，必须经 `json_io` 分发重建）；② `ProjectModel` 增删改场 / 单元后树结构正确；③ `generate.run`：`monkeypatch` 掉 `MakerCpp.makeAll` / `MakerGidFile.makeAll`，断言传入的 `mode` / `mainMode` / `outPath` / `sln_cmake_path` 正确 |
| GUI（View） | 手动 | PySide6 控件自动化测试 ROI 低，作者自用项目，以手动走查为主 |
| 端到端集成 | 脚本 + diff | 用 Truss1D 在 GUI 配置 → 生成，与 `test/test1DTruss.py` 直接产物 diff，确保 GUI 路径与脚本路径产物**逐字节一致** |

测试置于 `pyTool/gui/tests/`，与现有 pyTool 测试风格一致。

---

## 10. 代码组织

```
pyTool/gui/
  main.py                  # QApplication 入口
  main_window.py           # MainWindow：菜单 / 工具栏 / 树 / 堆叠面板 / 生成面板 / 日志
  models/
    project_model.py       # 当前 DataProject 包装：脏标记 + 增删改 + Qt 信号
  services/
    json_io.py             # 工程文件读写；load 时按 baseClass 分发重建 DataEleSub/G（绕开 DataField.fromDict 的基类退化问题）
    generate.py            # 封装 MakerCpp/MakerGidFile + stdout 重定向 + 异常捕获
  views/
    project_panel.py       # 项目编辑面板
    field_panel.py         # 场编辑面板
    element_panel.py       # 单元编辑面板（含 DataEleSubG 分支）
    generate_panel.py      # 底部生成面板
  widgets/
    list_editor.py         # 可复用列表编辑器（增删 / 上下移）
    table_editor.py        # 可复用表格编辑器（paramNames+值 / gaussPoints）
  tests/
    test_project_model.py
    test_json_io.py
    test_generate.py
```

文件粒度遵循单一职责：每个面板 / 服务 / widget 各一文件，便于独立理解与测试。

---

## 11. 验收标准

1. 能新建一个含 1 场 1 单元的项目，填入字段，保存为 `.cdfeg.json`，关闭后重新打开，全部字段无丢失。
2. 能加载现有 `testEl2D.py` 等价配置（手工在 GUI 复刻或直接导入其 `data.json`），生成产物与脚本产物 diff 为空。
3. 生成面板的 `mode=add` / `mainMode=1` 组合可正确向 `FEMproject/CMakeLists.txt` 追加 `add_subdirectory` 并写出项目文件。
4. 生成失败（如 `outPath` 为空）时给出明确提示且不崩溃。
5. pytest 全绿（round-trip + 模型增删改 + generate 参数捕获）。

---

## 12. 后续可选演进（不在本期）

- 多项目工作区：左侧项目列表，批量生成。
- 代码片段编辑：内嵌 QPlainTextEdit + C++ 语法高亮，把 `runCode` 等纳入 GUI。
- CMake 构建集成：生成后调用 cmake / mingw32-make。
- GiD 结果预览。
