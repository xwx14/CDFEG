# macs → CDFEG 迁移 skill 设计

> 日期：2026-06-22
> 状态：待批准
> 关联：[pyTool 能力边界](../../../.claude/rules/pyTool能力边界.md)、[核心库实现细节](../../../.claude/rules/核心库实现细节.md)、skill [generate-sample](../../../.claude/skills/generate-sample/SKILL.md)、[derive-element](../../../.claude/skills/derive-element/SKILL.md)

## 1. 背景与目标

`E:\mfelProject\RegTest\testData\macs` 下是旧 mfel 系统（C 语言 + GiD 前后处理 + SuperLU 求解）的多个有限元回归测试项目（el / heat / newmark / ...）。每个项目由描述文件（`*.pre`/`*.ges`/`*.gcn`/`*.mdi`/`*.cnd`）+ 手写 C 源码（`solution/src/`）构成。

目标：建立通用 skill 与解析脚本，把 macs 项目**半自动迁移**到 CDFEG 框架（C++ 三层架构 + GiD 前后处理 + Eigen 求解），先以 **el** 项目跑通，后续可直接复用于 heat / newmark 等。

迁移的本质是「描述文件解析 → pyTool 生成框架 → 人工/AI 参考旧 C 源码填充计算逻辑」。

## 2. 产物清单

| 产物 | 路径 | 职责 |
|---|---|---|
| SKILL 指南 | `.claude/skills/macs-to-cdfeg/SKILL.md` | 三步流程操作指南，对标 generate-sample |
| 通用解析器 | `pyTool/preParser.py` | 解析 `<proj>.pre` + `*.ges` + `<proj>.gcn`，构造 `DataProject` |
| 参数化生成入口 | `pyTool/test/testMacs.py` | `python test/testMacs.py <项目名>`，调用解析器 + MakerCpp/MakerGidFile |
| 核心库改动 | `FEMproject/CDFEG/PhyFieldData.{h,cpp}` | `getCoef` 改 virtual，逐场收集 `_nodeRes` |

生成的 C++ 项目落地 `macs/<Proj>`（项目根下，独立于 `FEMproject`），采用 `mode='new'`（自带 CDFEG 库 + third 副本，独立解决方案）。

## 3. 整体流程（SKILL 三步）

### 步骤 1：解析生成框架
`preParser` 读 macs 项目的 pre/ges/gcn → 构造 `DataProject` → `testMacs.py` 以 `mode='new'` 生成 C++ 类骨架（FEMData/PhyFieldData/EleSub 派生类）+ CMake + GiD 文件。

### 步骤 2：填充计算逻辑
参考旧 C 源码改造填充（**每个场的 `eProgram` 必须重写**，单元 `run`/`uEle` 手写）。映射见第 6 节。

### 步骤 3：比较 cnd
核对 pyTool 生成的 cnd 与原 `<proj>.cnd` 的边界条件 / 材料赋值项，判定一致。标准见第 8 节。

## 4. 解析器字段映射（pre + ges + gcn → pyTool 数据结构）

### 4.1 `<proj>.pre` → 项目/场/单元骨架 + 参数

| pre 内容 | 解析目标 |
|---|---|
| 首行 `2dxy 2 3 8 10` 的 `2dxy` | `DataProject.dim`（2dxy→2，3dxyz→3） |
| `ela 0 2 6 u v`（场名 + dof + ... + dispNames） | `DataField("ela")`，`dispNames=["u","v"]` |
| 单元段 `a1eq4g2 4 pe pv fu fv rou alpha`（单元名 + nNode + paramNames） | `DataEleSubG("a1eq4g2", 4)`，`paramNames` |
| `matedata` 段 `ela a1eq4g2` + 数值行 | `ele.paramValues` |

### 4.2 `*.ges` → 单元积分/形函数/结果名

| ges 段 | 解析目标 |
|---|---|
| `node 4` | 校验 `nNodes` |
| `gaus = 4` + 积分点行（坐标 + 权重） | `gaussPoints`、`gaussWeights` |
| `shap` 段形函数表达式 | `shapeFuns`（用 `x[1]/x[2]` 占位符，由 pyTool `_replaceCoordVars` 替换） |
| `func = exx,eyy,exy` | `eleResNames` |
| `coef u,v`（beq4g2） | 标记该单元 run 需跨场取值；填充时用 `coef["ela::u"]` 等 |
| `refc rx,ry` + `coor x,y` + nNode/dim | 推断 `type`（4 节点 2D→`type=2` IsoEleBase；2 节点→`type=1` 线/边界，`bBC=True`） |

### 4.3 `<proj>.gcn` → caculate 命令流

gcn（el 示例）：
```
defi
a ell
b str a          ← b 依赖 a
START a
SOLVc a          ← a 场标准椭圆求解
SOLVstr b a      ← b 场最小二乘平滑（依赖 a）
gidres(coor0);
```

解析产出：
- **场依赖顺序**：a 先 b 后（填入 `DataProject.caculateCode`，保证 elb 取 ela 位移时 ela 已求解）。
- **求解类型**：`SOLVc`=标准椭圆（initMatrix→eProgram→solve→uPhy）；`SOLVstr`=最小二乘平滑（同流程，但 eProgram 重写逻辑不同）。
- 每场生成一组命令；具体命令文本在步骤 2 按场补全（pyTool 现仅有 `imp` 一种命令模板，最小二乘需定制）。

## 5. 核心库改造：getCoef 跨场取数

### 5.1 现状
- `PhyFieldData::getCoef`/`getCoef1`（`PhyFieldData.cpp:96-106`）为**空实现**，返回空 map。
- `eProgram_el:68` 中 `coef = getCoef(nodeIds);` **被注释**，`eProgram_el` 中 coef 始终为空。

### 5.2 改造（仅 getCoef，eProgram_el 不变）

```cpp
// PhyFieldData.h：getCoef 改为 virtual
virtual std::map<std::string, std::vector<double>>
    getCoef(const std::vector<int>& nodeIds);

// PhyFieldData.cpp：逐场收集 _nodeRes，key = "场名::变量名"
std::map<std::string, std::vector<double>>
PhyFieldData::getCoef(const std::vector<int>& nodeIds) {
    std::map<std::string, std::vector<double>> coef;
    for (PhyFieldData* fd : _femData->_phyDatas) {
        for (auto& kv : fd->_nodeRes) {                 // 逐场逐变量
            std::vector<double> v;
            v.reserve(nodeIds.size());
            for (int nid : nodeIds)
                v.push_back(nid < (int)kv.second.size() ? kv.second[nid] : 0.0);  // 未求解/越界填 0
            coef[fd->_name + "::" + kv.first] = std::move(v);  // 如 "ela::u"
        }
    }
    return coef;
}
```

### 5.3 使用约定
- **`eProgram_el` 保持原样**（标准椭圆，coef 空）。跨场需求由各场**重写的 `eProgram`** 自行调 `getCoef(nodeIds)` 取 coef。
- `beq4g2.run` 通过 `coef["ela::u"]`、`coef["ela::v"]` 取 ela 场位移。
- 调用时序保证：caculate 命令流按 gcn 依赖顺序先解 ela，elb 调 getCoef 时 ela 的 `_nodeRes` 已由 `uPhy` 回填。
- 防御：目标场未求解时 `_nodeRes` 为空向量，`v.push_back` 越界——getCoef 内对 `kv.second` 长度不足需填 0 兜底（见第 9 节）。

## 6. el 项目迁移映射（步骤 2 填充依据）

| 旧 mfel（C） | CDFEG（C++）目标 | 填充要点 |
|---|---|---|
| `el.c` main | `ElData::caculate` | 命令流：a 场 init→eProg→solve→uPhy；b 场同（依赖 a） |
| `starta.c` | 框架 `initMatrix` | **不改**。框架保持 `_kVar` 规模，仅 `_ida==-1` 不进方程组，与旧 starta 减行等价 |
| `eela.c` | `ElaFieldData::eProgram`（重写） | 组装 a1eq4g2 单刚；**手动把 a2ll2（边界荷载线单元）的 eload 累加到 `_equSys._f`**（eProgram_el 不处理 eload→右端） |
| `eelb.c` | `ElbFieldData::eProgram`（重写） | `coef=getCoef(nodeIds)` 取 ela 位移；beq4g2 产 estif（=lumped mass）+ eload（应力载荷）；组装后 solve 解 `M·σ=f`；uPhy 回填应力 |
| `a1eq4g2.c` | `A1eq4g2::run`/`uEle` | 高斯积分 + B 矩阵 + D 矩阵；单刚按行主序填 `EleSubResult.estif`；uEle 算应变应力 |
| `beq4g2.c` | `Beq4g2::run` | 用 `coef["ela::u"/"ela::v"]` 求导得应变→本构→应力→eload；lumped mass 进 estif |
| `gidpre/gidres/gidmsh.c` | 框架已实现 | `mainMode=1`（GiD 入口），不手写 |

### elb 场本质（最小二乘应力平滑）
- `beq4g2.ges` 的 `dist=+[dxx;dxx]*0.0`（刚度为 0），靠 `mass`（lump）+ `load`（高斯点应力）解 `M·σ=f`，把高斯点应力平滑到节点。
- elb 的"单刚"实际是 lumped mass，eload 是应力载荷；`eProgram` 重写时按此组装。

## 7. 为什么「每场 eProgram 必须重写」

1. **eProgram_el 不处理 eload→右端**：仅 `adda(estif)`，单元载荷 `eload` 不会进 `_equSys._f`。ela 的边界荷载（a2ll2）、elb 的应力载荷都需手动累加。
2. **elb 非标准椭圆**：最小二乘平滑的"刚度"是 lumped mass，与 eProgram_el 的纯刚度组装语义不同。
3. **跨场 coef**：elb 需 `getCoef` 取 ela 位移，eProgram_el 不调 getCoef。

## 8. cnd 比较标准（步骤 3）

生成 cnd（`MakerGidFile`）vs 原 `el.cnd`，逐条核对：
- **边界条件项**：ela 的 `u-I/u-D/v-I/v-D`；elb 的 `dxx/dyy/dxy-I/D`。各场对 volume/surface/line/point 四种 CONDMETHOD 均应齐备。
- **材料赋值项**：每个单元类型的 `mate_Num` 条件（如 `Surface-a1eq4g2`、`Line-a2ll2`、`Surface-beq4g2`）。

**判定「大致相同」**：条件数量、每条的 QUESTION/VALUE 结构、CONDMETHOD 覆盖一致即合格；NUMBER 编号、字段顺序差异可接受。

## 9. 错误处理与防御

| 风险 | 防御 |
|---|---|
| ges 形函数表达式格式变体（空格/正负号） | 解析器按 ges 既定语法宽松匹配，失败时报告具体行号，不静默跳过 |
| pre 缺字段（如某场无 matedata） | 缺省值 + 警告，不中断；生成框架后人工补 |
| getCoef 跨场时目标场 `_nodeRes` 未初始化/长度不足 | `kv.second` 长度 ≤ nodeId 时填 0.0 兜底，避免越界 |
| elb 在 ela 之前求解（依赖倒置） | caculate 命令流严格按 gcn 依赖顺序生成；解析器校验依赖无环 |
| new 模式复制库路径错误 | `testMacs.py` 校验 outPath 可写，复制失败明确报错 |
| 重生成覆盖手填逻辑 | 步骤 2 强调：重跑生成前备份 run/uEle/eProgram 手填体（同 generate-sample 约定） |

## 10. el 验证标准（测试通过判据）

1. **编译通过**：`macs/El` 下 CMake 配置 + 构建无错（MinGW 或 MSVC，`/utf-8`）。
2. **运行不崩**：读 el 的 GiD 网格（.msh/.dat）→ 求解 → 输出结果。
3. **位移正确**：ela 场节点位移与旧 el.exe 结果一致（容差 1e-6）。
4. **应力正确**：elb 场节点应力（dxx/dyy/dxy）与旧结果一致。
5. **cnd 一致**：第 8 节判定通过。

## 11. 通用性（后续项目扩展）

- `preParser` 不硬编码 el：以项目名参数化，pre/ges/gcn 按文件名约定（`<proj>.pre`、`<ele>.ges`、`<proj>.gcn`）定位。
- heat（热传导）、newmark（动力学）等后续项目：步骤 1 解析生成可直接复用；步骤 2 按各项目旧 C 源码填充（newmark 涉及 `_bSavedData0` 动力学陷阱，见核心库实现细节）。
- 不同项目可能引入新的 gcn 命令类型（如 newmark 的时间步循环），届时 `preParser` 的 gcn 解析与 caculateCode 生成按需扩展。

## 12. 范围与非目标（YAGNI）

- 本 skill **不**自动把旧 C 代码翻译为 C++（步骤 2 由 AI/人工参考填充）。
- **不**扩展 pyTool 命令模板（imp 之外的新命令在 caculateCode 手填）。
- **不**迁移 SuperLU 求解器（CDFEG 用 Eigen SimplicialLDLT）。
- **不**处理 macs 下所有项目，仅先跑通 el；通用性靠参数化保证，但其余项目验证留待后续。
