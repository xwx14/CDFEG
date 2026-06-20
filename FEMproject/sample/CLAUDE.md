[根 CLAUDE.md](../../CLAUDE.md) > [FEMproject](../) > **sample 示例集**

# FEMproject/sample — 有限元示例集

> 路径 `FEMproject/sample/`；6 个独立 CMake 子项目，各链接 CDFEG 核心库，演示三层架构派生用法兼作回归基线。

## 示例索引

| 示例 | 维度 | 问题 | 单元 | 入口 | 重写 eProgram | pyTool 生成 |
| --- | --- | --- | --- | --- | --- | --- |
| truss1D | 1D | 桁架静力 | `Truss1D` | makeData | 否 | 是 |
| truss2D | 2D | 桁架静力 | `Truss2D` | GiD/makeData | 否 | 是 |
| truss3D | 3D | 桁架静力 | `Truss3D` | — | 否 | 是 |
| El2D | 2D | 平面应力静力 | `ElQ4g`/`ElT3g`/`StressBL2g` | GiD | 否 | 是 |
| ElT3 | 2D | 弹性力学 | `ElT3` | — | 否 | 是 |
| DEl2D | 2D | Newmark-β 动力学 | `NewmarkQ4g` | GiD | **是** | **否（手写）** |

## 入口形式

| 形式 | mainMode | 代表 | 逻辑 |
| --- | --- | --- | --- |
| makeData | 0 | truss1D | `main()` 内手工 `addNode/addEle`，无文件依赖 |
| GiD 数据文件 | 1 | El2D、DEl2D | 命令行 `<project> <path>`，`GidPrePost.pre()` 读 `<project>.dat` |

## 关键陷阱

- **DEl2D 是手写动力学示例，勿运行 `testDEL2D.py`**：pyTool 会用空壳 `ElQ4g` 覆盖手写的 `NewmarkQ4g`，丢失动力学逻辑。注意动力学每步须 `_equSys._bSavedData0 = false` 刷新基线（见 `.claude/rules/核心库实现细节.md`）。

## 关联

- 派生新单元的操作流程：skill `derive-element`。
- 头文件引用：`#include "CDFEG/FemData.h"`、`#include "CDFEG/gidPrePost.h"`。
