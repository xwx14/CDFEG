[根 CLAUDE.md](../../CLAUDE.md) > [FEMproject](../) > **sample 示例集**

# FEMproject/sample — 有限元示例集

> 路径：`FEMproject/sample/`  
> 类型：6 个独立 CMake 子项目，每个链接 CDFEG 核心库
> 用途：演示核心库三层架构的派生用法，兼作回归基线

## 变更记录 (Changelog)

| 时间 | 说明 |
| --- | --- |
| 2026-06-18 15:43:22 | init-architect 首次生成模块文档 |
| 2026-06-18 | 删除遗留示例 el3d，示例数 7→6 |
| 2026-06-18 | 深挖补充 | 新增第六节「等参元派生参考（以 ElQ4g 为样板）」，梳理构造/shapeFun/run/uEle 要点与单刚存储顺序陷阱 |
| 2026-06-18 | 同步 | §6.3 单刚存储顺序描述更新：核心库 `adda` 已改为行主序读取，约定统一 |

---

## 一、模块职责

每个子目录是一个独立的 CMake 项目（含 `CMakeLists.txt` + `main.cpp` + 派生类源码），编译为可执行文件，演示如何派生 `FEMData` / `PhyFieldData` / `ElementBase` 解决具体有限元问题。部分示例由 `pyTool/test/` 生成，部分为手写。

## 二、入口与启动

- 每个示例有自己的 `main.cpp`。
- **全部 6 个示例均纳入根 CMake**：truss1D、truss2D、truss3D、El2D、ElT3、DEl2D（根 `FEMproject/CMakeLists.txt` 第 21-26 行 `add_subdirectory`）。

### 2.1 两种 main 形式

| 形式 | mainMode | 代表 | 入口逻辑 |
| --- | --- | --- | --- |
| makeData 形式 | 0 | truss1D | `main()` 内 `makeData(data)` 手工 `addNode/addEle`，无文件依赖 |
| GiD 数据文件形式 | 1 | El2D、DEl2D | 命令行 `<project> <path>`，`GidPrePost.pre()` 读 `<project>.dat` |

## 三、示例索引

| 示例 | 路径 | 维度 | 问题类型 | 单元 | 入口形式 | 派生物理场是否重写 eProgram | 由 pyTool 生成 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| truss1D | `sample/truss1D/` | 1D | 桁架静力 | `Truss1D` | makeData | 否（用基类 `eProgram_el`） | 是 | 输出 `Truss1D.txt`（节点位移/单元内力） |
| truss2D | `sample/truss2D/` | 2D | 桁架静力 | `Truss2D` | GiD/makeData | 否 | 是 | 含方向余弦 `calcDir2D` |
| truss3D | `sample/truss3D/` | 3D | 桁架静力 | `Truss3D` | — | 否 | 是 | 含 `calcDir3D` |
| El2D | `sample/El2D/` | 2D | 平面应力静力 | `ElQ4g`(Q4) / `ElT3g`(T3) / `StressBL2g`(边界线) | GiD | 否 | 是（testEl2D.py，mainMode=1） | 静力升级参考样板 |
| ElT3 | `sample/ElT3/` | 2D | 弹性力学 | `ElT3` | — | 否 | 是 | 含 `.gid` 子目录与 bat |
| DEl2D | `sample/DEl2D/` | 2D | Newmark-β 动力学 | `NewmarkQ4g` | GiD | **是**（必须，组合有效矩阵 K+a0M+a1C） | **否（手写）** | 见 `升级说明.md`，记录 12 个易错点 |

## 四、关键依赖与配置

- 全部 `target_link_libraries(示例 PRIVATE CDFEG)` 链接核心库。
- 包含目录：`${CMAKE_CURRENT_SOURCE_DIR}` + `${EIGEN_INCLUDE_DIR}` + `${CMAKE_SOURCE_DIR}`（故引用 `CDFEG/FemData.h` 形式路径）。
- 头文件引用约定：`#include "CDFEG/FemData.h"`、`#include "CDFEG/gidPrePost.h"`（见 El2D/DEl2D main.cpp）。
- DEl2D 额外依赖 Eigen 求解器（通过核心库间接）。

## 五、典型用法（以 El2D 为模板）

```cpp
#include "CDFEG/gidPrePost.h"
int main(int argc, char* argv[]) {
    if (argc < 3) { /* usage */ return 1; }
    elData data;
    CDFEG::GidPrePost gidPrePost(&data);
    gidPrePost.setFilePath(argv[2], argv[1]);  // path, project
    gidPrePost.pre();                           // 读 <project>.dat
    data.caculate();                            // 计算
    // 注册结果项
    CDFEG::GidResItem resItem1("disp", CDFEG::GidResultType::Vector);
    resItem1.addVal(0, "u"); resItem1.addVal(0, "v");
    gidPrePost._resItems.push_back(resItem1);
    gidPrePost.post2();                         // 写 <project>.flavia.res
}
```

DEl2D（动力学）差异：`caculate` 内含时间步循环，每步 `eProgram→solve→uPhy→post2(it)`，且 `eProgram`/`uPhy` 被物理场子类重写。

## 六、等参元派生参考（以 `ElQ4g` 为样板）

> 以 `sample/El2D/ElQ4g.*`（4 节点四边形、平面应力）为样板，梳理派生一个等参元的完整步骤。同样适用于 `ElT3g`、`NewmarkQ4g` 等 `IsoEleBase` 派生类。

### 6.1 构造函数：设置元信息 + 积分参数

- 继承 `CDFEG::IsoEleBase(nNode, pData)`；
- 设 `_name`（须与 dat 的 elem name、`mat_<name>` 一致）、`_dispNames`、`_paramNames`、`_types`；
- 设维度/积分参数：`_dim` / `_nNode` / `_nDisp` / `_nVar(=_nNode*_nDisp)` / `_nGaus` / `_nRefc` / `_nCoor`；
- 填 `_gaus`（权重）、`_refc`（积分点参考坐标，按 `[iGaus][dim]` 平铺），**调 `caculateShapeCoef(dim)`** 预算形函数数值差分表；
- resize 结果：`_result.estif(_nVar²)` / `edamp(_nVar²)` / `emass(_nVar)` / `eload(_nVar)`，设 `_vtkCellType`。

### 6.2 `shapeFun(refc)`（纯虚，必须实现）

返回各节点形函数在参考坐标 `refc` 处的值（Q4 为 4 个双线性形函数），顺序对应节点编号约定。

### 6.3 `run(r, coef, matParams)`（单刚组装核心）

1. **清零** `_result.estif/eload/emass`（每次 `run` 必须清零，与核心库 `adda` 的累加配合）；
2. 高斯积分循环：
   - `dcoor(r, iGaus, coor, rctr)` 求坐标雅可比；
   - `inverse(rctr, crtr)` 求逆 + 行列式 `det`；
   - `shapn(iGaus, coor, crtr, cu)` 得 `cu[i][0..2]`（形函数值、∂N/∂x、∂N/∂y）；
   - `weight = _gaus[iGaus] * det`，由 `cu` 构建 B 矩阵，累加 `K += BᵀDB·weight`、`eload += Nᵀf·weight`；
3. 若 `_bSaveResult` 则 `_results.push_back(_result)`。

> **单刚存储顺序**：`estif` 按**行主序**填充（`index = i*_nVar + j`，外循环行、内循环列），核心库 `adda` 已统一为行主序读取（`estifn[i*nd+j]`），二者一致。派生单元按行展开即可（详见核心库 CLAUDE.md §9.3）。

### 6.4 `uEle(r, coef, matParams)`（后处理，可选）

从 `coef`（物理场填入的节点位移）按高斯点求应力（平面应力 D 矩阵），再以形函数加权外推到节点；返回 `eleResult`（单元平均应力）+ `nodeResult`（节点应力与 `weight`）。

### 6.5 物理场派生（`ElDispFieldData`）扩展点

| 扩展点 | El2D 做法 | 说明 |
| --- | --- | --- |
| 注册单元 | 构造函数 `push_back(new ElQ4g/ElT3g/StressBL2g(this))` | 一个物理场可注册多种单元 |
| `eProgram` | **不重写**（基类 `eProgram_el` 静力组装） | 静力问题无需重写 |
| `uPhy` | **重写**：节点应力按 `weight` 加权平均 + von Mises | 基类 `uPhy` 仅回填位移，应力后处理需派生 |
| 结果配置 | `_eleResNames` / `_resForm="Vector OnNodes"` | 供 GidPrePost 输出 |

> 派生模式：**静力等参元 = 单元派生(`IsoEleBase`) + 物理场重写 `uPhy`(应力外推) + 数据类派生(`FEMData`，从 GiD 读网格)**。动力学(DEl2D) 另需重写 `eProgram`（Newmark 有效矩阵）并每步重置 `_bSavedData0`。

## 七、常见问题 (FAQ)

1. **Q: 修改某示例后编译报"标识符未声明"？**  
   A: 多为 UTF-8 中文注释被 MSVC 按 GBK 解码。根 CMake 已加 `/utf-8`，确认未绕过该配置。

2. **Q: 能否运行 pyTool 重新生成手写示例（如 DEl2D）？**  
   A: **不能**。`testDEL2D.py` 用 `ElQ4g` 名生成会覆盖手写的 `NewmarkQ4g`，丢失动力学逻辑。pyTool 仅用于生成 dat 数据文件（见 `sample/DEl2D/升级说明.md` 5.11）。

3. **Q: GiD 数据文件格式？**  
   A: `.dat` 由 `GidPrePost` 解析，含 `coord`/`element`/`mate`/`time`/边界段，详见 `TextReader` 与 `gidPrePost.cpp`。材料段名必须为 `mat_<单元_name>`。

## 八、相关文件清单

构建：各子目录 `CMakeLists.txt`（6 个）
升级说明：`DEl2D/升级说明.md`（Newmark 动力学升级，含 12 个易错点）  
GiD 配置：`ElT3/ElasticT3.gid/ElasticT3.bat`、`ElT3/gid/ElasticT3.gid/ElasticT3.bat`
