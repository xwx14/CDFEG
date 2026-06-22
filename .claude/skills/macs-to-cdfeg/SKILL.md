---
name: macs-to-cdfeg
description: Use when 把 mfel 旧项目（macs 测试集：el/heat/newmark 等）迁移到 CDFEG 框架，需解析 pre/ges/gcn 生成框架、参考旧 C 源码填充计算逻辑、比较 cnd。
---

# macs → CDFEG 迁移（macs-to-cdfeg）

## Overview
把 `E:/mfelProject/RegTest/testData/macs/<proj>` 下的旧 mfel（C+GiD）项目迁移到 CDFEG（C++ 三层架构）。三步：解析生成框架 → 填充逻辑 → 比较 cnd。

## When to Use
- 新迁移一个 macs 项目到 CDFEG
- 排查迁移后编译/结果与旧程序不一致

## 步骤

### 1. 解析生成框架
```bash
cd pyTool && python test/testMacs.py <项目名>
```
解析 `<proj>.pre`+`*.ges`+`<proj>.gcn` → 生成 `macs/<Proj>`（new 模式，自带库副本）。产物：FEMData/PhyFieldData/EleSub 派生类骨架 + CMake + GiD。

### 2. 填充计算逻辑（参考旧 C 源码）
**每个场的 `eProgram` 必须重写**（基类 `eProgram_el` 不处理 eload→右端，也不调 getCoef）。映射：
| 旧 C（solution/src） | CDFEG 目标 |
|---|---|
| `<proj>.c` main 流程 | `<Proj>Data::caculate`（gcn 解析已生成命令流） |
| `ee<field>a.c` | `<Field>FieldData::eProgram` 重写（组装单刚 + eload→_f） |
| `<ele>.c` | `<Ele>::run`/`uEle`（高斯积分+B矩阵+行主序单刚） |
| `gidpre/gidres/gidmsh.c` | 框架已实现（mainMode=1） |

**多场 coef**：依赖场（如 elb）的 `eProgram` 调 `getCoef(nodeIds)` 取位移，单元 `run` 用 `coef["<场名>::<变量>"]`（如 `coef["ela::u"]`）。时序由 caculate 命令流保证（先 ela 后 elb）。

### 3. 比较 cnd
核对生成 cnd 与原 `<proj>.cnd`：边界条件项（各场 -I/-D × volume/surface/line/point）+ 材料赋值项（mate_Num）。数量与 QUESTION/VALUE 结构一致即合格。

## Common Mistakes
| 现象 | 原因 | 修复 |
|---|---|---|
| 单刚全 0 | dat elem name 与单元 `_name` 不一致 | 改 dat 为 `mat_<_name>` |
| elb 位移全 0 | getCoef 时 ela 未求解 | caculate 顺序按 gcn 依赖 |
| 载荷未生效 | eProgram_el 不处理 eload | 重写 eProgram，手动 `_equSys._f += eload` |
| 重生成覆盖手填 | 重跑 testMacs | 先备份 run/uEle/eProgram 体 |

> 深度机制见 `.claude/rules/核心库实现细节.md`；pyTool 能力边界见 `.claude/rules/pyTool能力边界.md`。
