---
name: derive-element
description: Use when 派生或新增一种有限元单元类型（如 Q4/T3/六面体/桁架），需要确定继承哪个基类、实现构造/shapeFun/run/uEle，并遵循单刚行主序与 dat 材料名等约定。
---

# 新增有限元单元（derive-element）

## Overview

CDFEG 中"一种单元"= 一种材料在某物理场下的一种单元。派生单元继承 `ElementBase`（直接给单刚）或 `IsoEleBase`（等参元，形函数 + 高斯积分）。本 skill 给出从零派生一个单元的完整步骤与必踩陷阱。

样板：`FEMproject/sample/El2D/ElQ4g.*`（4 节点四边形平面应力）。

## When to Use

- 新增一种单元类型（新拓扑、新物理场组合）
- 修改现有单元的 `run` / `uEle` 实现
- 排查"单刚全 0""材料读不到""结果逐单元累加"等单元级问题

## 步骤

### 1. 选基类

| 场景 | 基类 | 例子 |
| --- | --- | --- |
| 直接给单刚（桁架、梁等） | `CDFEG::ElementBase` | truss1D/2D/3D |
| 需形函数 + 高斯积分的等参元（Q4/T3/六面体） | `CDFEG::IsoEleBase` | ElQ4g、ElT3g |

> pyTool 端：`DataEleSub.type ≥ 2`（面/体）自动选 `IsoEleBase`，由 `baseClass` 字段控制。

### 2. 构造函数：元信息 + 积分参数

- 继承 `CDFEG::IsoEleBase(nNode, pData)`（或 `ElementBase`）；
- 设 `_name`（**须与 dat 的 elem name、`mat_<name>` 完全一致**）、`_dispNames`、`_paramNames`、`_types`；
- 设维度 / 积分参数：`_dim` / `_nNode` / `_nDisp` / `_nVar(=_nNode*_nDisp)` / `_nGaus` / `_nRefc` / `_nCoor`；
- 填 `_gaus`（权重）、`_refc`（积分点参考坐标，按 `[iGaus][dim]` 平铺），**调 `caculateShapeCoef(dim)`** 预算形函数数值差分表；
- resize 结果：`_result.estif(_nVar²)` / `edamp(_nVar²)` / `emass(_nVar)` / `eload(_nVar)`，设 `_vtkCellType`。

### 3. `shapeFun(refc)`（纯虚，必须实现）

返回各节点形函数在参考坐标 `refc` 处的值，顺序对应节点编号约定（Q4 为 4 个双线性形函数）。

### 4. `run(r, coef, matParams)`（单刚组装核心）

1. **清零** `_result.estif / eload / emass`（每次 `run` 必须清零——核心库 `adda` 是累加 `+=`，不清零会叠加错误值）；
2. 高斯积分循环：
   - `dcoor(r, iGaus, coor, rctr)` 求坐标雅可比；
   - `inverse(rctr, crtr)` 求逆 + 行列式 `det`；
   - `shapn(iGaus, coor, crtr, cu)` 得 `cu[i][0..2]`（形函数值、∂N/∂x、∂N/∂y）；
   - `weight = _gaus[iGaus] * det`，由 `cu` 构建 B 矩阵，累加 `K += BᵀDB·weight`、`eload += Nᵀf·weight`；
3. 若 `_bSaveResult` 则 `_results.push_back(_result)`。

### 5. `uEle(r, coef, matParams)`（后处理，可选）

从 `coef`（物理场填入的节点位移）按高斯点求应力（平面应力 D 矩阵），再以形函数加权外推到节点；返回 `eleResult`（单元平均应力）+ `nodeResult`（节点应力与 `weight`）。

### 6. 物理场注册

在物理场派生类构造函数 `push_back(new XxxEle(this))` 注册单元；一个物理场可注册多种单元。

### 7. dat 材料名约定

dat 文件 elem 段 `name` 与材料段 `* name=mat_<name>` 必须与 `ElementBase::_name` **完全一致**，否则 `GidPrePost::readMate` 匹配失败、单刚全为 0。

## Common Mistakes（陷阱）

| 现象 | 原因 | 修复 |
| --- | --- | --- |
| 单刚全 0 | dat 材料 name 与 `_name` 不一致 | 改 dat 为 `mat_<_name>` |
| 结果逐单元累加爆炸 | `run` 未清零 `_result` | `run` 开头清零 estif/eload/emass |
| 单刚行列错位 / 非对称错误 | `estif` 未按行主序填充 | 按 `index = i*_nVar + j`（外行内列）填充 |
| 应力为 0 | 未重写物理场 `uPhy`（基类仅回填位移） | 派生 `uPhy` 做应力外推 |

> 单刚存储顺序与总刚组装机制详见 `.claude/rules/核心库实现细节.md`「总刚组装」节。
