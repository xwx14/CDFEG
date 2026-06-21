[根 CLAUDE.md](../../CLAUDE.md) > [FEMproject](../) > **CDFEG 核心库**

# CDFEG 核心库（有限元基础库 DLL）

> 路径 `FEMproject/CDFEG/`；C++14 DLL，命名空间 `CDFEG::`，依赖 Eigen 3.4。本库无 main，编译为 `CDFEG.dll`，供示例 `target_link_libraries(示例 PRIVATE CDFEG)` 链接。

## 三层架构

| 层 | 类 | 职责 | 派生要点 |
| --- | --- | --- | --- |
| 总体数据 | `FEMData` | 网格 / 材料 / 物理场 / 流程控制 | 重写 `caculate()`、`main()` |
| 物理场 | `PhyFieldData` | 自由度 / 边界 / 方程组装求解 / 后处理 | 线性椭圆问题用默认 `eProgram_el()`；**非线性 / 非椭圆 PDE 必须重写 `eProgram`** |
| 单元 | `ElementBase` / `IsoEleBase` | `run()` 算单刚单质单阻载荷、`uEle()` 后处理 | 派生流程见 skill `derive-element` |

> 自由度宏 `DOF_ID(nodeId, iDof) = nodeId*_dof + iDof`。各类成员签名查 codegraph。

## 关键陷阱与约定

- **非线性 / 非椭圆 PDE 必须重写 `eProgram`**：默认 `eProgram()` 转发的 `eProgram_el()` 仅是**线性椭圆偏微分方程**的求解算法（组装刚度 → 施加边界 → 求解位移一次）。凡非线性或非椭圆问题（动力学 Newmark、非线性材料、几何非线性等）都必须完全重写 `eProgram`（装配有效矩阵、处理历史状态等），通常同时重写 `uPhy`。
- **`_bSavedData0` 基线陷阱**：`applyFirstBCs` 首次调用缓存纯净总刚 `_data0`，之后每次从基线恢复。动力学每步总刚变化时，**必须每步装配后 `_equSys._bSavedData0 = false`** 强制刷新，否则用首步过时基线导致载荷 / 刚度错误。详见 `.claude/rules/核心库实现细节.md`。
- **单刚行主序**：`adda` 按 `estifn[i*nd+j]`（行主序）读取，派生单元填充 `EleSubResult.estif` 须按行展开（外行内列）。
- **`_name` 一致**：单元 `_name` 须与 dat elem `name`、材料 `mat_<name>` 完全一致，否则单刚全 0。
- **自由度编号 `_ida`**：`-1` 表示该自由度无单元参与 / 被约束，不进方程组；`≥0` 为方程号。
- **额外参数 `_addParams`**：三层均可声明参数组，`gidPrePost::pre()` 读 dat 按组名回填 `_paramValues`，`getParam(组名,参数名)` 取值。DEl2D Newmark `gamma/beta` 为首个应用，设计稿见 `docs/superpowers/specs/2026-06-19-额外参数前处理机制-design.md`。

## 编码与构建

- Eigen 3.4（`FEMproject/third/Eigen`），仅用 `Eigen/Sparse` + `SimplicialLDLT`。
- 包含路径：根 CMake 设 `${CMAKE_SOURCE_DIR}` 为包含目录，源码引用 `CDFEG/xxx.h`。
- MSVC `/utf-8`；`CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE` + `CDFEG_EXPORTS` 自动导出全部符号。

## 深度参考

- 类结构与成员详解：[`代码解释.md`](./代码解释.md)（FEMData / PhyFieldData / ElementBase 层次与关键成员方法）
- 总刚组装 / 边界划行列法 / 求解 / 后处理机制与陷阱：[`.claude/rules/核心库实现细节.md`](../../.claude/rules/核心库实现细节.md)
