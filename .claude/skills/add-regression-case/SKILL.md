---
name: add-regression-case
description: Use when 为 FEMproject/sample 示例创建有理论解的 .dat 测试算例并接入 test/ 回归框架（e2e 套件）：造网格/边界/解析解、放 test/models、注册 config.toml、--update 冻结基线、跑回归对比 max|Δ|。改核心库后想把某算例纳入稳定性回归时同样适用。
---

# 造测例并加入回归（add-regression-case）

## Overview

CDFEG 回归测试（`test/`，纯 Python 编排）的核心闭环：**造一个有理论解的 `.dat` 算例 → 放 `test/models/<case>.gid/` → `config.toml` 注册 → 用 `--update` 冻结基线 `.post.res` → `--suite e2e` 重跑对比**。改核心库后一条命令量化全示例输出漂移（`max|Δ|`），守住计算稳定性。

框架流程：构建 cmake target → 隔离目录跑 `<target>.exe <project> .` → `parser` 解析产出 res → `comparator` 与基线对比（**结构一致性优先** + 逐点容差）。基线永不被 exe 覆盖。

入口：`python test/run_tests.py`。框架全貌见 `docs/superpowers/plans/2026-07-17-测试系统.md`。

## When to Use

- 为新示例（或已有示例）造一个回归测例
- 改核心库后想加一个稳定性锚点
- 把手写的 `.dat` 算例纳入自动对比
- 不用于：pyTool 生成器测试（`--suite generator`）、Catch2 C++ 单测（`--suite unit`）

## 步骤

### 1. 造有理论解的 dat

选**单元能精确表达**的问题——这样除"比基线漂移"外还能"比理论解"：

| 单元 | 精确场 | 推荐算例 |
| --- | --- | --- |
| CST 三角形（ElT3） | 均匀应变/应力 | 单轴拉伸（强制位移，体力 0） |
| Q4 等参（ElQ4g） | 均匀应变 + 规则网格纯弯 | 单轴拉伸 / 纯弯曲梁 |
| 桁架（Truss*） | 常轴力 | 简单桁架节点 |

dat 段格式（`GidPrePost::pre` 解析；段头 `* name=…,type=…`；**文本文件**，非二进制）：

```
* name=baseData,structure="I32"
<npoin> <nelem>
* name=time,structure="F64"
<dt> <tMax>                          # 静力给 1.0 1.0
* name=mat_<EleName>,structure="F64**N",type="mat",index=0
<E P1 P2 ...>                         # 按 Ele::_paramNames 顺序，一行一种材料
* name=coord,structure="I32*1 F64*2",type="coord",index=1
<nodeId> <x> <y>
* name=ubf<FieldName>,structure="I32*1 F64*D",type="ubf",index=0
<nodeId> <u_val> <v_val>              # 第一类强制位移（约束靠此段）
* name=<EleName>,structure="I32*M",type="elem",index=1
<eleId> <n1> <n2> ... <mateId>
```

要点：
- **约束靠 `ubf` 段**（`setFirstBoundry` 强制位移）；`id` 段是**空实现**，约束值实际由 ubf 给。
- **载荷靠材料体力 `fx/fy` 或强制位移**（dat 无面力段）。
- 文件名 = `<project>.dat`；外圈节点强制理论位移、内部节点留自由（被求解，作校验点）。
- 先手算理论值（如 σx=E·ε₀），跑出后核对，再冻结基线。

### 2. 放入 models（一测例一文件夹）

```
test/models/<case>.gid/<project>.dat      # 输入；基线 res 由 update 生成，勿手写
```

文件夹名带 `.gid` 后缀（约定，如 `elt3_1.gid`）。**一测例一文件夹**。

### 3. config.toml 注册

`test/config.toml` 的 `[[suite.e2e.cases]]` 追加：

```toml
[[suite.e2e.cases]]
name     = "<case>"            # 唯一，如 elt3_1
target   = "<cmakeTarget>"     # CMake target 名，如 ElasticT3
project  = "<argv1>"           # main 第1参数，如 ElasticT3（= dat/res 文件名前缀）
case_dir = "models/<case>.gid"
baseline = "<project>.post.res"
output   = "<project>.post.res"
tol_atol = 1e-12               # 同 mingw 工具链，位元级
tol_rtol = 0.0
```

> `target` = 根 CMakeLists `add_subdirectory` 的 target；`project` = sample `main.cpp` 的 `argv[1]`（决定读 `<project>.dat`、产 `<project>.post.res`）。两者**必须先从 sample main 确认，勿臆测**。

### 4. 构建目标（确认能编）

```bash
cmake --build test/build --target <target> -j        # mingw Unix Makefiles
```

首次或源码改动后跑；exe 落 `test/build/output/<target>.exe`。编不过先修编译错误。

### 5. 冻结基线

```bash
echo yes | python test/run_tests.py --update <case>
```

`--update` 在隔离目录跑示例 → diff 旧基线（首次显示"将新建"）→ `yes` 确认 → 覆盖 `case_dir/<project>.post.res`，并追加 `test/reports/update_log.txt`。**禁用 `--update all`**（强制逐用例过目每个变更）。

### 6. 验证

```bash
python test/run_tests.py --suite e2e                 # 全部 e2e
python test/run_tests.py --suite e2e --case <case>   # 单个
```

通过判据：`pass`、`max|Δ|≈0`（同工具链重跑位元一致）、退出码 0。

## 关键约定

| 项 | 约定 |
| --- | --- |
| parser 兼容 | `OnNodes`（节点）+ `OnGaussPoints`（单元结果，按实体号解析）；`GaussPoints` 定义块被忽略。disp 与 stress 都能对比 |
| 结构一致性优先 | 结果段 `(name,step)`、分量名、实体集合任一不一致 → 直接 `fail`（硬信号），先于数值容差 |
| 基线隔离 | exe 在 `build/run/<case>/` 跑，拷贝 dat 时**排除 `.post.res`**，基线永不被覆盖（不会"自己比自己"假 pass） |
| 容差档 | regression `tol_atol=1e-12`（同工具链）；跨工具链（MSVC↔mingw）末位抖动 → 放宽或同工具链重新冻结 |
| 工具链 | config.toml: mingw `Unix Makefiles` + `mingw32-make`，`dll_dirs=["C:/dev/mingw64/bin"]` |
| 输出格式 `format` | config case 字段：`gid`（默认，GiD `.post.res`，走 `parser`）/ `truss_txt`（分节文本 `.txt`，走 `txt_parser`）。两者都映射 `ResBlock`，复用 `comparator` |

## Common Mistakes（陷阱）

| 现象 | 原因 | 修复 |
| --- | --- | --- |
| 首次跑 `error: 基准缺失` | 未先 `--update` 冻结 | 先 `--update <case>` 生成基线 |
| `未产出 <project>.post.res` | target/project/文件名不匹配 | 核对 sample main 的 argv 与 setFilePath，config 的 target/project 对齐 |
| `max\|Δ\|` 大（>1e-8） | 基线与当前工具链不符 | 同 mingw 重新 `--update`；基线若来自 MSVC 则末位抖动，放宽 tol |
| 单元结果（应力）没被对比 | sample main 未注册 `ResItem`（`post`） | 在 main 注册应力项（范例：`FEMproject/sample/ElT3/main.cpp` 的 `stress`/`OnGaussPoints`） |
| 改核心库后多测例 fail | 预期回归（真漂移） | 逐个 `--update` 重冻前，**先确认漂移合理**（对齐理论解/基准程序） |
| `--update` 无反应 | 未输 `yes`（交互确认） | `echo yes \|` 管道喂入，或手动输 yes |
| dat 被 Read 工具当二进制拒读 | 误判（CRLF/特殊字节） | dat 是文本文件，用 Bash `file`/`cat` 读（见根 CLAUDE.md） |

## 变体：无输入的 makeData 示例（truss1D / truss2D）

`main()` 内 `makeData` 硬编码网格、**无 dat 输入**、输出自定义 `.txt`（非 GiD）的示例，接入差异：

- **跳过造 dat**：`case_dir` 不放输入文件，只放基线 `<output>.txt`（拷当前输出即可，等同 `--update` 首次冻结）。
- **config 加 `format = "truss_txt"`**：框架按此走 `txt_parser`（`========== 节名 ==========` 分节 + 表头 + 数据行 → `ResBlock`，复用 `comparator`）。
- **project 参数无意义**：truss `main` 无 argv、忽略它；`output`/`baseline` 用 main 硬编码产出的固定名（`Truss1D.txt` / `Truss2D.txt`）。
- **基线隔离**：`_prepare_work_dir` / `_run_in_work_dir` 额外排除 `baseline` 同名文件，防止基线 `.txt` 被拷进 work_dir 造成"自己比自己"假 pass。

## 范例

`test/models/elt3_1.gid/`（3×3，8 三角形）与 `elt3_4x4.gid/`（4×4，18 三角形）：ElT3 平面应力 CST 单轴拉伸，E=1e6 / ν=0.3 / ε₀=1e-3，外圈节点强制理论位移、内部节点自由。理论 σx=E·ε₀=1000、σy=τxy=0；回归 `max|Δ|=0`。算例推导见 `FEMproject/sample/ElT3/单轴拉伸算例说明.md`。

`test/models/truss1D1.gid/`（`Truss1D.txt`）与 `truss2D1.gid/`（`Truss2D.txt`）：**无输入** makeData 桁架示例，`format = "truss_txt"`，基线=当前 `.txt` 输出（节点位移 + 单元轴力/应力分节），回归 `max|Δ|=0`。
