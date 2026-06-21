---
name: generate-sample
description: Use when 用 pyTool 代码生成器创建或更新 FEMproject/sample 下的有限元示例（生成 C++ 源码/CMake/GiD 文件），需要组织数据结构、选择生成模式，并避免覆盖手写示例源码。
---

# 用 pyTool 生成示例（generate-sample）

## Overview

pyTool 根据"数据结构描述"（项目→场→单元）自动生成符合 CDFEG 核心库约定的 C++ 程序 + CMake + GiD 文件。生成器只产"脚手架"（类结构 + 构建配置），**计算逻辑（run/uEle/uPhy）仍需人工填充**（详见 `.claude/rules/pyTool能力边界.md`）。

入口：每个 `pyTool/test/test*.py` 既是测试也是生成入口。运行方式：在 `pyTool/` 目录下 `python test/testXxx.py`。

## When to Use

- 新建一个有限元示例项目
- 用 pyTool 重新生成已有示例的骨架
- 排查生成代码的编译 / 运行问题

## 步骤

### 1. 组织数据结构（三层）

```python
from DataProject import DataProject
from DataField import DataField
from DataEleSubG import DataEleSubG

project = DataProject("el2d", 2)            # 项目名, 维度
field = DataField("ElDisp")                 # 场名
ele = DataEleSubG("ElQ4g", 4)               # 单元名, 节点数
ele.type = 2                                # 0点/1线/2面/3体；≥2 自动选 IsoEleBase
ele.dispNames = ["u", "v"]
ele.paramNames = [...]
ele.gaussPoints = [...]; ele.gaussWeights = [...]; ele.shapeFuns = [...]  # 形函数字符串，x[1]/x[2] 占位符
field.addEleSub(ele)
project.addField(field)
```

> 所有数据类提供 `toDict()/fromDict()`，test 脚本末尾会写 `data.json`。

### 2. 选生成模式

| 模式 | 用途 | 行为 |
| --- | --- | --- |
| `mode='new'` | 创建独立解决方案 | 复制 CDFEG 库 + third，生成解决方案级 CMake |
| `mode='add'` | 追加到现有 FEMproject | 只生成项目文件，向现有 CMake 追加 `add_subdirectory` |

### 3. 生成

```python
from MakerCpp import MakerCpp
from MakerGidFile import MakerGidFile

maker = MakerCpp(project, "sample/El2D", mode='add', sln_cmake_path="CMakeLists.txt")
maker.makeAll()                              # C++ 源码 + CMake
MakerGidFile(project, "sample/El2D").makeAll()  # GiD .bas/.prb/.cnd/.bat
```

### 4. 运行 test 脚本（生成入口）

在 `pyTool/` 目录下 `python test/testEl2D.py`。脚本内 `sys.path.append` 上级目录以导入 Maker。

## Common Mistakes（陷阱）

| 现象 | 原因 | 修复 |
| --- | --- | --- |
| 手填逻辑被覆盖 | 重运行 test 脚本覆盖了手填的 `run`/`uEle`/`uPhy` 体 | 重生成前备份手填逻辑。DEl2D 已对齐命名（`DelQ4g`），`testDEL2D.py` 生成框架、Newmark 逻辑手填，可安全重生成（勿覆盖手填体） |
| 等参元编译报错 | `baseClass` 未设 `IsoEleBase` | `type≥2` 自动设，或手动 `ele.baseClass = "IsoEleBase"` |
| shapeFun 占位符未替换 | `shapeFuns` 未用 `x[i]` 占位符 | 用 `x[1]/x[2]`，由 `_replaceCoordVars` 替换为 `coordVars` |
| VTK 单元类型错 | 未调 `inferVTKCellType()` | 构造后调用 |
| `mode='add'` 未追加 | `sln_cmake_path` 未提供 / 文件不存在 | 传入存在的根 CMake 路径；重复运行会跳过 |
| emoji 报 UnicodeEncodeError | Windows GBK 控制台 | `safePrint` 已降级，或设 `PYTHONUTF8=1` |

> 生成器能力边界（能 / 不能生成什么）详见 `.claude/rules/pyTool能力边界.md`。
