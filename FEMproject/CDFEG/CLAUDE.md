[根 CLAUDE.md](../../CLAUDE.md) > [FEMproject](../) > **CDFEG 核心库**

# CDFEG 核心库（有限元基础库 DLL）

> 路径：`FEMproject/CDFEG/`  
> 类型：C++ 动态链接库（DLL），命名空间 `CDFEG::`，C++14，依赖 Eigen 3.4  
> 维护者：xwx14（谢文玺）

## 一、模块职责

提供有限元程序的**通用计算函数和通用父类**，使具体有限元程序只需派生并填充少量虚函数。本库本身不包含 main，编译为 `CDFEG.dll`（Windows）/ 共享库供示例链接。

## 二、入口与启动

- 无可执行入口；本库编译为目标 `CDFEG`（SHARED）。
- 使用方通过 `target_link_libraries(示例 PRIVATE CDFEG)` 链接，并在源码中 `#include "CDFEG/FemData.h"` 等头文件（根 CMake 设 `${CMAKE_SOURCE_DIR}` 为包含目录，引用路径为 `CDFEG/xxx.h`）。
- 导出宏 `CDFEG_API` 由 `CDFEG.h` 定义，受 `CDFEG_EXPORTS` 宏控制；`CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE` 自动导出全部符号。

## 三、架构与对外接口

### 3.1 三层架构核心类

#### 顶层 `FEMData`（FemData.h/.cpp）—— 总体数据类
管理网格、材料、物理场，控制整个计算流程。

| 类别 | 成员/方法 | 说明 |
| --- | --- | --- |
| 网格 | `setNPts(n)` / `addNode(id,x,y,z)` / `addNodeEnd()` | 节点管理，`addNodeEnd` 触发编号收尾 |
| 网格 | `addEle(id,nodeIds,eleType)` / `addEdge(...)` | 单元管理，按 `eleType` 字符串分发到对应 `ElementBase` |
| 材料 | `addMate(matParam,name)` → 材料号 / `setEleMateId` / `setEleMateByName` / `getElemMatParams` | 材料参数存为 `map<string,double>` |
| 关键数据 | `_nodes` / `_eleNodes` / `_eleTypes` / `_elePt` / `_nodeIdMap` / `_eleIdMap` / `_phyDatas` / `_mateParams` | 平铺存储 + ID 映射（文件号↔程序号） |
| 时间 | `_dt` / `_tMax` | 动力学步长与总时长（dat 的 time 段读入） |
| **虚函数** | `virtual int caculate() {return -1;}` | 派生类**必须**实现，控制计算流程 |
| **虚函数** | `virtual int main() {return -1;}` | 派生类**必须**实现（部分示例用全局 main 替代） |
| 前后处理 | `_processors: vector<Processor*>` | 持有前后处理器实例 |
| 额外参数 | `_addParams` / `_paramValues` / `getParam(组名,参数名)` | **三层均有**：声明参数组、gidPrePost 回填后取值（见 §5.5） |

#### 中层 `PhyFieldData`（PhyFieldData.h/.cpp）—— 物理场类
包含一个或多个单元，负责该场的方程组装、求解与后处理。

| 类别 | 成员/方法 | 说明 |
| --- | --- | --- |
| 单元管理 | `addEleSub(ElementBase*)` → 单元序号 | 添加单元类型 |
| 节点初始化 | `setNPts(n)` / `initMatrix()` | `initMatrix` 根据边界条件与单元初始化稀疏骨架 |
| 边界条件 | `addBoundary` / `setFirstBoundry` / `setSecondBoundry` | 一类（位移）/二类（力）边界，存于 `_nodeBC1s` / `_nodeBC2s`（key=dofId） |
| **计算** | `virtual int eProgram() {return eProgram_el();}` | 默认转发椭圆求解；非椭圆派生类**必须重写** |
| 计算 | `int eProgram_el()` | 已实现：遍历单元 `run()` → `EquationSystem.adda` 组装总刚 → 施加边界 → `solve` |
| 求解 | `int solve()` | 调用 `_equSys.solve()`（Eigen `SimplicialLDLT`） |
| 后处理 | `virtual int uPhy()` | 根据结果计算节点/单元物理量（应力、轴力等） |
| 系数 | `getCoef1(nodeIds)` / `getCoef(nodeIds)` / `getNodeDisps(...)` | 取形函数系数、节点位移 |
| 方程系统 | `_ida: vector<int>` / `_equSys: EquationSystem` / `_neq` | `_ida` 是节点规格数（自由度→方程号映射） |
| 结果 | `_nodeRes` / `_elemRes: map<string,vector<double>>` | 后处理器从这里取值输出 |
| 关联场 | `_assPhys: vector<PhyFieldData*>` | 多场耦合用 |

> 自由度宏 `DOF_ID(nodeId, iDof) = (nodeId)*_dof + (iDof)`。

#### 底层 `ElementBase`（ElementBase.h/.cpp）—— 单元子程序基类
"一种单元"= 一种材料在某物理场下的一种单元。

| 类别 | 成员/方法 | 说明 |
| --- | --- | --- |
| **核心虚函数** | `virtual EleSubResult& run(r, coef, matParams)` | 派生类**必须**实现，计算单刚/单质/单阻/载荷 |
| 核心虚函数 | `virtual uResult uEle(r, coef, matParams)` | 后处理，派生类可选实现 |
| 基本信息 | `_name` / `_types: set<string>` / `_nNode` / `_nElement` / `_dispNames` / `_paramNames` / `_vtkCellType` | 单元名（用于 dat 匹配）、自由度名、材料参数名 |
| 结果 | `_result: EleSubResult` / `_results: vector<EleSubResult>` | 实时/历史结果，由 `_bSaveResult` 控制是否保存每单元 |
| 结果结构体 | `EleSubResult{estif, emass, edamp, eload, nodeIds}` | 单刚、单质、单阻、外力向量、节点编号 |
| 结果结构体 | `uResult{eleResult, nodeResult}` | 后处理结果（单元级 map 与节点级 map） |
| 所属 | `_phyData` / `_femData` | 反向指针，用于取材料、坐标 |
| 编号 | `_eleIds` / `_eleMatIDMap` | 单元编号与材料号映射（旧版兼容） |

### 3.2 等参元基类 `IsoEleBase`（IsoEleBase.h/.cpp）

继承 `ElementBase`，提供等参坐标变换与高斯积分基础设施。派生等参单元（如 Q4、T3）继承此类并实现 `shapeFun(refc)`。

| 成员/方法 | 说明 |
| --- | --- |
| `shapeFun(refc) = 0` | **纯虚**，由具体单元给形函数表达式 |
| `coordTransFun(x, refc)` | 坐标变换（局部→全局） |
| `dcoor(r, iGaus, fx, dfdx, dord)` | 坐标雅可比矩阵 |
| `dshap(iGaus, fx, dfdx, dord)` | 形函数雅可比矩阵 |
| `shapn` / `shapc` / `caculateShapeCoef` | 形函数值/系数的数值差分计算 |
| `_gaus` / `_nGaus` / `_refc` / `_refShapCoef` | 积分点权重、坐标、形函数值表 |
| `_ndord` | 导数最大阶数（默认 1） |

> `_refShapCoef[iGaus][i][0..偏移]` 的偏移编码见源码注释（一维 0/±1，二维 9 种组合，三维 27 种），用于数值差分求偏导。

### 3.3 方程系统 `EquationSystem`（EquationSystem.h/.cpp）

基于 Eigen 稀疏矩阵（`SpMat = Eigen::SparseMatrix<double, RowMajor>`）。

| 方法 | 说明 |
| --- | --- |
| `init(mht)` | 由行指示初始化稀疏结构 |
| `adda(estifn, equIds)` | 单元刚阵累加到总刚 |
| `applyFirstBCs(bc1s, ida)` | 批量施加一类边界（划行列法） |
| `applySecondBCs(bc2s, ida)` | 批量施加二类边界 |
| `solve()` | Eigen 求解 |
| `convert2Eigen()` | 转 Eigen 矩阵 |
| `calRightVals()` | 计算右值 |

> **重要陷阱（动力学必读）**：`applyFirstBCs` 内有缓存基线机制（`_bSavedData0`），首次调用保存 `_data0/_f0`，后续恢复。动力学每步 `_f` 变化，必须**每步装配后强制 `_equSys._bSavedData0 = false`** 再施加边界，否则后续步载荷被恢复成首步值（详见 `sample/DEl2D/升级说明.md` 5.2 节）。完整机制见 §9.4。

### 3.4 矩阵工具 `MatrixFun`（MatrixFun.h/.cpp）

自由函数集合（非类成员），全部 `CDFEG_API` 导出：

- 行列式 `determinant`、逆矩阵 `inverse`、转置 `transpose`
- 矩阵乘 `multiply(A,B)` / 矩阵-向量乘 `multiply(A,x)`
- 向量：`crossProduct` / `dotProduct` / `add` / `subtract` / `scalarMultiply` / `norm`
- 方向余弦：`calcDir2D` / `calcDir3D` / `calcDir2D2` / `calcDir3D2`（桁架单元用）
- 转换矩阵：`computeTransformMatrix(k,n,m,r)` / `multiplyTAT(T,A)` / `multiplyT(T,v)`

### 3.5 前后处理器（Processor 体系）

| 类 | 头文件 | 职责 |
| --- | --- | --- |
| `Processor`（抽象基） | Processor.h | `pre()` / `post(it)` 虚函数，持有 `_femData` 与 `_phyFieldData` |
| `GidPrePost` | gidPrePost.h/.cpp | GiD 文件读写：`setFilePath` / `pre()` 读 `.dat`（含按三层 `_addParams` 声明收集并回填额外参数到各层 `_paramValues`，见 §5.5）；`post()` / `post2(it)` 写 `.msh`/`.res`；持 `_resItems: vector<GidResItem>` |
| `GidPrePost2` | GidPrePost2.h/.cpp | GiD 后处理变体（多结果项） |
| `GidResItem` | GidResItem.h/.cpp | 结果项描述：名称、类型（`GidResultType::Vector/Matrix/Scalar`）、`addVal(iField, valName)` 绑定物理场结果列 |
| `vtkPost` | vtkPost.h/.cpp | VTK 后处理：`writeVTK(fn)` |
| `inpReader` | inpReader.h/.cpp | Abaqus INP 读取 |
| `TextReader` | TextReader.h/.cpp | 通用文本/键值对读取（供 `GidPrePost` 解析 `.dat`） |
| `InpDataStructures` | InpDataStructures.h | INP 数据结构定义 |

辅助：`vtkCellType.h` 定义 `VTKCellType` 枚举（pyTool 中 `vtkCellType.py` 是其镜像）。

## 四、关键依赖与配置

- **Eigen 3.4.0**（header-only）：位于 `FEMproject/third/Eigen`，由根 CMake 通过 `EIGEN_INCLUDE_DIR` 变量传入。仅用 `Eigen/Sparse`（稀疏矩阵 + `SimplicialLDLT` 求解器）。
- **C++14**，无其他运行时依赖。
- Windows DLL：`CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE` + `CDFEG_EXPORTS` 宏。
- 编码：源文件 UTF-8（含中文注释），MSVC 下需 `/utf-8`（根 CMake 已配置）。

## 五、数据模型

### 5.1 ID 映射机制

文件中节点/单元号可能非连续，库内部用平铺数组存储，通过 `_nodeIdMap`（`map<int,int>`：文件号→程序号）、`_eleIdMap` 维护映射。`addNode` / `addEle` 时自动建立映射。

### 5.2 单元节点平铺

`_eleNodes` 平铺存储所有单元的节点号，`_elePt[i]` 记录第 i 个单元节点在 `_eleNodes` 的起始位置（`_elePt` 长度 = 单元数 + 1），支持混合单元类型。

### 5.3 自由度编号

`PhyFieldData::_ida`（节点规格数）：-1 表示自由度不存在；≥0 表示相同整型数对应相同方程号（用于约束/耦合）。`initMatrix` 据此生成方程号与稀疏骨架。

### 5.4 材料参数

`FEMData::_mateParams: vector<map<string,double>>` + `_mateNames`。`getElemMatParams(eleID, ele)` 按 `_eleMateIds` 索引返回。dat 中材料名必须为 `mat_<单元_name>`（见根文档 7.6）。

### 5.5 额外参数前处理读取（`_addParams` 机制）

三层（`FEMData` / `PhyFieldData` / `ElementBase`）均有 `_addParams: vector<vector<string>>`（每组首元素为组名、其余为参数名）与 `_paramValues: map<组名, vector<double>>`。

- **声明与回填**：`gidPrePost::pre()` 开头 `collectPreParamDecls()` 收集三层声明表（跳过保留名 `time/basedata/coord/id/ubf/elem`、重名告警）；读 dat 时按组名命中、读一行 double 截断回填到声明层 `_paramValues`（用原始组名作 key）。
- **取值**：各层 `getParam(组名, 参数名)` 按 `_addParams` 定位、从 `_paramValues` 取值；组/参数名不存在或值未读到时返回 `0.0`。
- **dat 格式**：与 `time` 段同构（单星 `* name=<组名>` + 一行值），属项目级 GenData；`.bas` 模板用双星 `** name=`（GiD 渲染成单星）。
- **pyTool 端**：由 `preParams`（`{name, params, defaults}`）→ `gidbas.j2`（`*GenData` 段）/ `gidprb.j2`（`QUESTION` 段）生成。
- **首个应用**：DEl2D Newmark `gamma/beta`（`NewmarkDispFieldData._addParams = {{"newmark","gamma","beta"}}`，`NewmarkData::caculate` 用 `getParam` 取值并校验 `beta>0`）。设计稿见 `docs/superpowers/specs/2026-06-19-额外参数前处理机制-design.md`。

## 六、测试与质量

- 机制级回归测试：`FEMproject/testPreParam/testPreParam.cpp`（验证三层 `_paramValues` 回填与 `getParam` 防御分支：组/参数名不存在、组行长度<2、向量越界均返回 0.0），已纳入根 CMake 的 `testPreParam` target。
- 其它回归依赖 `FEMproject/sample/` 各示例（truss1D 输出 `Truss1D.txt` 可人工核对）。
- 历史测试数据：`FEMproject/test/el2dData/`（注意：`.gitignore` 第 57 行忽略 `/FEMproject/test`，即此目录不入库）。
- 质量工具：未见 eslint/ruff/golangci 等配置；Python 端靠 `pyTool/test/test*.py` 脚本（既是测试也是代码生成入口）。

## 七、常见问题 (FAQ)

1. **Q: 为什么 `eProgram_el` 不能用于动力学？**  
   A: 它只装配刚度，不处理 `eload`、无质量/阻尼组合、无历史状态。动力学必须完全重写 `eProgram`。

2. **Q: 派生单元的 `_name` 与 dat 文件如何对应？**  
   A: dat 的 elem 段 `name` 与材料段 `* name=mat_<name>` 必须与 `ElementBase::_name` 完全一致。

3. **Q: 如何选择继承 `ElementBase` 还是 `IsoEleBase`？**  
   A: 桁架等直接给单刚的单元继承 `ElementBase`；Q4/T3/六面体等需要形函数+高斯积分的等参元继承 `IsoEleBase`。pyTool 的 `DataEleSub.type ≥ 2` 时自动选 `IsoEleBase`。

4. **Q: 头文件包含路径？是否还有 `include/` 副本？**  
   A: 统一用 `FEMproject/CDFEG/` 下的头文件（根 CMake 设 `${CMAKE_SOURCE_DIR}` 为包含目录，引用 `CDFEG/xxx.h`）。历史 `FEMproject/include/CDFEG/` 公共头副本已于 2026-06-19 删除（无构建/示例引用）。

## 八、相关文件清单

核心类：`CDFEG.h` / `ElementBase.h/.cpp` / `IsoEleBase.h/.cpp` / `PhyFieldData.h/.cpp` / `FemData.h/.cpp`  
方程与矩阵：`EquationSystem.h/.cpp` / `MatrixFun.h/.cpp`  
前后处理：`Processor.h/.cpp` / `gidPrePost.h/.cpp` / `GidPrePost2.h/.cpp` / `GidResItem.h/.cpp` / `vtkPost.h/.cpp` / `vtkCellType.h` / `inpReader.h/.cpp` / `InpDataStructures.h` / `TextReader.h/.cpp`  
构建：`CMakeLists.txt`  
架构说明（人工）：`../程序说明.md`

测试：`../testPreParam/testPreParam.cpp`（额外参数前处理机制回归，根 CMake 的 `testPreParam` target）

## 九、核心实现细节（总刚组装 / 边界施加 / 求解）

> 本节基于 `PhyFieldData.cpp` 与 `EquationSystem.cpp` 源码梳理，供理解计算链路、以及派生动力学/非线性问题时参考。

### 9.1 稀疏矩阵存储格式（自定义 CSR 变体）

`EquationSystem` 不直接使用 Eigen 内部格式，而维护四个并行数组：

| 成员 | 含义 |
| --- | --- |
| `_numCol` | 行指针：`_numCol[i]` = 第 i 行非零元在 `_colId/_data` 的起始下标，末元素 = 非零元总数（长度 = 行数 + 1） |
| `_colId` | 列号数组（每个非零元的列索引） |
| `_data` | 数值数组（组装与边界施加在此原地修改） |
| `_colMap` | `_colMap[row] = {列号 → 在 _colId/_data 中的下标}`，O(log n) 定位 `_data[row,col]` |

`init(mht)` 由每行列集合 `mht`（`vector<set<int>>`）填充上述结构；`convert2Eigen()` 用 `Eigen::Map<...RowMajor>` 零拷贝映射到 Eigen 稀疏矩阵，受 `_bEigenConverted` 缓存（矩阵变动后需置 `false`）。

### 9.2 自由度编号（`initMatrix`）

`initMatrix()` 遍历所有单元，**仅当节点某自由度出现在「单元子程序的 `_dispNames`」中**时才分配方程号（首次遇到即 `_ida[iDof] = ++nEq`）。因此：

- `_ida[i] == -1`：该自由度无单元参与（被约束或本场不涉及）→ 不进方程组；
- `_ida[i] >= 0`：方程号；`_neq` = 实际方程数（≤ `_kVar = nPts*_dof`）。

同时构建稀疏骨架 `mht`：单元内所有自由度编号两两配对，形成非零位置。

### 9.3 总刚组装（`eProgram_el` + `adda`）

`eProgram_el()` 流程：
1. 遍历每个单元子程序的每个单元：取节点坐标 `r`、材料参数（`getElemMatParams`）→ 调 `eleSub->run(r,coef,matParams)` 得单刚 `estif`；
2. 构建单元定位向量 `lm`：单元节点自由度经 `_ida` 映射为方程号；
3. `_equSys.adda(estif, lm)` 组装；
4. 全部单元完成后：`applyFirstBCs` + `applySecondBCs`。

`adda(estifn, equIds)` 关键细节：
- **单刚按行主序存储**：`estif` 以 `index = i*nd + j`（`i`=行、`j`=列，外循环行、内循环列）存储，`adda` 按 `estifn[i*nd + j]` 读取，与所有单元（`ElQ4g`/`ElT3g`/`NewmarkQ4g`/truss 等）的行主序填充一致。派生单元填充 `EleSubResult.estif` 时按行展开即可。
- `equIds < 0`（无方程的自由度）被跳过。
- `adda` 是**累加（`+=`）**，不清零 `_data`。

### 9.4 第一类边界 —— 划行列法 + 基线缓存（`_bSavedData0`）

`applyFirstBCs(bc1s, ida)`（及单条 `addFirstBC`）采用**划行（列）修改法**，配合基线缓存保证幂等：

1. **首次调用**（`_bSavedData0 == false`）：快照纯净总刚 `_data0 = _data`、`_f0 = _f`，置 `_bSavedData0 = true`；
2. **每次调用**：先从基线恢复 `_data = _data0; _f = _f0;`，再对 `_firstBCMap` 中**全部**一类边界重新施加：
   - 边界方程行：对角元置 1，该行其余非零元置 0；
   - 其余方程的边界列：`_f[i] -= a_ij * bcVal`（`a_ij` 取自 `_data0`），矩阵元素清零；
   - 右端项 `_f[bcEquId] = bcVal`；
3. 置 `_bEigenConverted = false`。

**设计意图**：划行列法会破坏性修改 `_data`（不可逆），故每次从 `_data0` 干净重来，使「重复施加 / 修改边界值」结果幂等且正确。

> **⚠️ 动力学/多步陷阱**：`_bSavedData0` 只在首次为 false 时保存基线，之后恒为 true。若后续步总刚 `_data` 发生变化（动力学每步重装、非线性迭代、有效矩阵 `K+a0M+a1C` 更新），**必须手动 `_equSys._bSavedData0 = false`** 强制刷新基线，否则 `applyFirstBCs` 会从首步过时的 `_data0` 恢复，导致载荷/刚度错误。此外 `adda` 累加不清零，多步组装前需先清零 `_data`。详见 `sample/DEl2D/升级说明.md`。

### 9.5 第二类边界

`applySecondBCs(bc2s, ida)` / `addSecondBC`：仅右端项叠加 `_f[equId] += val`（节点力），**不修改矩阵**，不涉及基线。

### 9.6 求解（`solve`）

`solve()`：`convert2Eigen()` → `Eigen::SimplicialLDLT<SpMat>(_eigenMat)` 分解 → `chol.solve(_f)` → 结果写入 `_rhs`。`_rhs` 即节点位移解（按方程号排列）。

### 9.7 后处理（`uPhy`）

`uPhy()`：
1. 把 `_rhs[id]`（`id = _ida[...]`）回填 `_nodeRes[dispName][node]`（节点位移）；`_ida == -1` 的自由度（一类边界节点）未被填入，保持初值；
2. 逐单元以节点位移为 `coef` 调 `eleSub->uEle(r,coef,matParams)`，将其 `eleResult` / `nodeResult` 写入 `_elemRes`。

### 9.8 节点力 / 反力（`calRightVals`）

`calRightVals()` 用**未经划行处理的原始总刚 `_data0`** 乘以位移解：`_rightVals[i] = Σ_j _data0[i,j] * _rhs[j]`。这是 `_data0` 基线的第二个用途——计算节点力（含支座反力）必须用纯净总刚，而非被划行修改后的 `_data`。
