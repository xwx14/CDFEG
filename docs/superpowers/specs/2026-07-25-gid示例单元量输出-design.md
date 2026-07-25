# 设计：4 个 GiD 示例添加单元量（eleStress）输出

> 日期：2026-07-25
> 范围：sample 示例集（DEl2D/El2D/ElT3/Hel2D 共 4 个 GidPrePost 示例）+ pyTool 代码生成器 + 回归基线
> 不涉及：核心库计算/装配逻辑；truss1D/2D/3D（命令行文本输出，非 GiD）；HeatQ4g 温度场（无单元量）
> 前序：[`2026-07-25-前后处理配置上提-design.md`](./2026-07-25-前后处理配置上提-design.md) —— **已实施**：`_eleResItems`/`_nodeResItems` 两桶已就位，`gidPrePost::post` 与 `vtkPost::post` 均遍历 `_eleResItems` 写 OnGaussPoints/CellData。本设计是该管线的**首次实际启用**（4 个 GiD 示例此前只注册了节点量）。

## 1. 背景与目标

前序改造已把"单元结果输出管线"全线打通：

```
单元 uEle() 产出 eleResult[σxx/σyy/σxy/volume]
  → PhyFieldData::uPhy() 写入 _elemRes[name][eleID]   （仅对 _eleResNames 声明的名字 resize/写入）
  → GidPrePost::post()  写 OnGaussPoints 段            （.post.res，每单元 1 高斯点 = 单元平均）
  → vtkPost::post()     写 CellData                    （.vtu/.pvd）
  → 回归 parser 解析 → comparator 按 (result_name, step) 对比
```

但 4 个 GiD 示例的 `main.cpp` 此前**只注册了 `_nodeResItems`**（位移、节点应力外推），`_eleResItems` 为空，故单元量从不写出。

**目标**：

1. 在 4 个 GiD 示例注册单元量结果项 `eleStress`（+ `eleVolume`），使 `.post.res` 与 `.vtu` 均输出单元平均应力；
2. pyTool 生成脚本（`test*.py`）同步声明 OnGaussPoints 输出项，使生成器产出与手写一致的 main 注册代码；
3. 回归基线刷新，把新增单元量段纳入冻结基线，确保后续核心库改动可被单元量回归捕获。

## 2. 设计决策（已确认）

| 决策 | 选择 | 理由 |
| --- | --- | --- |
| 目标示例 | DEl2D/El2D/ElT3/Hel2D（4 个 GidPrePost 示例） | truss 走命令行文本（truss_txt），非 GiD；HeatQ4g 温度场无单元量 |
| result_name | **`eleStress`**（驼峰，区别于节点 `stress`） | parser 按 `(name,step)` 建 key，同名 OnGaussPoints 会覆盖节点段，必须换名 |
| volume 归属 | **单独 `eleVolume`（Scalar）** | 语义清晰，应力与体积各自独立结果集；不混入应力 result |
| 输出分量 | 应力 σxx/σyy/σxy + volume | 单元 `uEle` 已产出（按权重平均到单元中心）；ElT3 单元无 volume，仅输出应力 |
| pyTool 改动 | 仅补 `test*.py` 的 `addOutputItem` | `mainGid.cpp.j2` 已按 location 分流两桶，模板无需改 |
| 基线刷新 | 逐用例 `--update` 重新生成 | 沿用既有基线更新流程（强制人工确认） |

## 3. 详细设计

### 3.1 各示例注册内容（核心改动）

单元量分量名遵循各示例既有 `_eleResNames`（已在 PhyFieldData 派生类声明，`uPhy` 据此 resize `_elemRes`）：

| 示例 | eleStress 分量 | iField | eleVolume | 注册时机 |
| --- | --- | --- | --- | --- |
| El2D | sigmaXX/sigmaYY/sigmaXY | 0 | volume | `data.post(0)` 前 |
| DEl2D | sigmaXX/sigmaYY/sigmaXY | 0 | volume | `caculate()` 前（已有注册块内追加） |
| Hel2D | sigmaXX/sigmaYY/sigmaXY | **1**（位移场） | volume | `data.post(0)` 前 |
| ElT3 | **Sxx/Syy/Sxy** | 0 | **无**（单元未产出） | `data.post(0)` 前 |

> Hel2D 两场：温度场 index=0（`_eleResNames={}` 空，无单元量），位移场 index=1（DelQ4g，有应力）。eleStress/eleVolume 绑位移场。
> ElT3 单元分量名为 `Sxx/Syy/Sxy`（非 sigmaXX），与 `Elastic2DDispFieldData::_eleResNames` 一致；且 `ElT3::uEle` 不产出 volume，故不注册 eleVolume。

注册写法示例（El2D）：

```cpp
CDFEG::ResItem eleStress("eleStress", CDFEG::ResType::Matrix, CDFEG::ResLocation::OnGaussPoints);
eleStress.addVal(0, "sigmaXX");
eleStress.addVal(0, "sigmaYY");
eleStress.addVal(0, "sigmaXY");
data._prePostConfig._eleResItems.push_back(eleStress);

CDFEG::ResItem eleVolume("eleVolume", CDFEG::ResType::Scalar, CDFEG::ResLocation::OnGaussPoints);
eleVolume.addVal(0, "volume");
data._prePostConfig._eleResItems.push_back(eleVolume);
```

### 3.2 GiD 输出形态（无需改 `gidPrePost::post`，仅说明）

`post` 对每个 `_eleResItems` 项、每个单元子程序写出：

```
GaussPoints "GP_<eleName>" ElemType <Quadrilateral|Triangle|...>     # it==0 时定义一次
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "<analysis>" <step> Matrix OnGaussPoints "GP_<eleName>"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY"
Values
    <单元id> <σxx> <σyy> <σxy>
    ...
End Values
```

每单元 1 个高斯点 = 单元平均应力（与 `_elemRes[name][eleID]` 单值模型一致）。

### 3.3 VTK 输出（无需改 `vtkPost::post`，自动生效）

`vtkPost::post` 已遍历 `_eleResItems` 写 `<CellData>`（vtkPost.cpp:133-152）。注册后 `.vtu` 自动含 eleStress/eleVolume 的 CellData；`parse_vtu_file` 已处理 CellData → `OnCells`。del2d1_vtk 算例（pvd）因此同步获得单元量。

### 3.4 pyTool 同步

4 个 `test*.py` 各补 `addOutputItem`（与手写 main 一致）：

```python
# testEl2D.py / testDEL2D.py / testHel2d.py
project.addOutputItem("eleStress", "Matrix", "OnGaussPoints",
                      [(iField, "sigmaXX"), (iField, "sigmaYY"), (iField, "sigmaXY")])
project.addOutputItem("eleVolume", "Scalar", "OnGaussPoints", [(iField, "volume")])

# testElT3.py（分量名不同，无 volume）
project.addOutputItem("eleStress", "Matrix", "OnGaussPoints",
                      [(0, "Sxx"), (0, "Syy"), (0, "Sxy")])
```

- Hel2D 的 `iField=1`（位移场），其余 `iField=0`。
- `mainGid.cpp.j2` 已按 `item.location` 分流到 `_eleResItems`/`_nodeResItems`（22/26 行），**模板无需改**。
- **DEl2D/Hel2D 手填 main 保护**：二者 main 为动力学/耦合手填逻辑，重跑 `test*.py` 仅作配置同步记录，**不得覆盖手填 main**（pyTool 既有约定：重生成前保留手填 `run`/`uEle`/`uPhy`/main 体）。实施时确认 `testDEL2D.py`/`testHel2d.py` 的 mainMode 不触发手写 main 覆盖。

### 3.5 回归基线刷新

`python test/run_tests.py --update <case>` 逐用例刷新（强制 `yes` 确认）。受影响算例（共 10 个）：

| 算例 | target | 格式 | 新增段 |
| --- | --- | --- | --- |
| del2d1 / del2d_mini1 | del2d | gid (.res) | eleStress + eleVolume（每步） |
| hel2d1 | hel2d | gid (.res) | eleStress + eleVolume（位移场） |
| el2d1 / el2d_bf1 / el2_mfel1 / el2_mfel_noedge1 | el2d | gid (.res) | eleStress + eleVolume |
| elt3_1 / elt3_4x4 | ElasticT3 | gid (.res) | eleStress（Sxx/Syy/Sxy，无 volume） |
| del2d1_vtk | del2d | pvd (.vtu) | eleStress + eleVolume（CellData） |

刷新后 `comparator._check_structure` 要求 actual/baseline 的 `(name,step)` key 集合一致——新增段必须同步进基线，否则报"新增结果段"结构漂移。

## 4. 文件改动清单

**示例**（`FEMproject/sample/`）：
- 改：`El2D/main.cpp`、`ElT3/main.cpp`、`Hel2D/main.cpp`、`DEl2D/main.cpp`（各追加 eleStress/eleVolume 注册块）

**pyTool**（`pyTool/`）：
- 改：`test/testEl2D.py`、`test/testElT3.py`、`test/testDEL2D.py`、`test/testHel2d.py`（各补 `addOutputItem`）

**回归基线**（`test/models/<case>.gid/`）：
- 刷新：上表 10 个算例的 `.post.res` / `.pvd`+`.vtu` 基线文件

**核心库**：无改动（管线已就绪）。
**文档**：本 spec；必要时同步 sample `CLAUDE.md` 的示例索引（若提及输出项）。

## 5. 验证方法

1. **clean 重建**：`--clean-first` 构建核心库 + 4 个示例，编译通过。
2. **手动跑 1 个示例确认输出形态**：跑 El2D → 检查 `el.post.res` 含 `GaussPoints "GP_ElQ4g"` 段 + `Result "eleStress" ... OnGaussPoints` 段 + `Result "eleVolume" ...` 段；分量数与单元数正确。
3. **逐用例刷新基线**：`--update` 上表 10 个算例，确认每个 diff 摘要 `结构错误=无`（仅新增段，无数值漂移）。
4. **全量回归绿**：`python test/run_tests.py`，全部 e2e case pass（容差 1e-12）；generator suite 通过（pyTool 生成 main 与手写注册一致）。
5. **VTK 核查**：ParaView 打开 `del2d.pvd`，CellData 可见 eleStress/eleVolume；多步动画正常。

## 6. 风险与对策

| 风险 | 对策 |
| --- | --- |
| 分量名与 `_eleResNames` 不一致 → `_elemRes` 未 resize、值为 0 | 严格按下表对齐：El2D/DEl2D/Hel2D 用 sigmaXX/YY/XY，ElT3 用 Sxx/Syy/Sxy |
| Hel2D 误把 eleStress 绑温度场（iField=0）→ 取空 `_elemRes` | Hel2D eleStress/eleVolume 必须 `addVal(1, ...)`（位移场） |
| DEl2D 多步：每步 eleStress 段使基线膨胀 | 基线本就多步；parser 按 (name,step) 分键，无冲突；刷新即可 |
| `testDEL2D.py`/`testHel2d.py` 重跑覆盖手填 main | 实施前确认 mainMode；必要时仅改 test*.py 的 addOutputItem 不重跑生成，或重跑后核对 main |
| 多单元类型同场 → post 写同名 Result 多段、parser 后者覆盖 | 6 个 El2D/DEl2D 算例触发覆盖（详见 §7）；Hel2D/ElT3 单 sub-type 无影响。物理 `.res` 输出正确，仅 e2e 回归对比盲区（非功能 bug） |
| 基线刷新遗漏某算例 → 该算例回归报"新增结果段" | 按 §3.5 表逐个 `--update`，run_tests 全绿为准 |

## 7. 已知限制（本次不修）

1. **每单元 1 高斯点**：OnGaussPoints 输出单元平均应力（`_elemRes` 单值模型），非逐高斯点应力。符合当前数据结构，未来逐高斯点输出需扩展 `_elemRes` 为多维。
2. **多单元类型同场同名 Result 覆盖**：`post` 对每单元类型写一个同名 `Result` 段，parser 以 `(result_name, step)` 为 key，后者覆盖前者。实际 **6 个算例触发此覆盖**：el2d1、el2d_bf1、el2_mfel1、el2_mfel_noedge1（ElQ4g/ElT3g/StressBL2g 混用）、del2d1、del2d_mini1（DelQ4g/StressBL2g 混用）。后果：GiD `.res` 物理输出本身正确（各 sub-type 段独立，GiD 可见体单元真实应力），仅 e2e 回归对比存在盲区——体单元（ElQ4g/ElT3g/DelQ4g）真实应力在 parser 层被边单元 StressBL2g 段（全 0）覆盖，max|Δ|=0 为虚假通过。仅 Hel2D（位移场单 DelQ4g）、ElT3（单 ElT3）无此问题。改进方向（后续独立 task）：parser `test/framework/parser.py` 的 key 由 `(result_name, step)` 扩展为含 GaussPoints 名的维度，或 `gidPrePost::post` 按 sub-type 拼 result_name（如 `eleStress_ElQ4g`）；任一改动需重刷 10 基线。

## 8. 行为变化小结

1. 4 个 GiD 示例的 `.post.res` 新增 `eleStress`（+ `eleVolume`）OnGaussPoints 结果段；`.vtu`/`.pvd` 新增对应 CellData。
2. pyTool 4 个 `test*.py` 声明 OnGaussPoints 输出项，生成 main 含单元量注册。
3. 10 个回归算例基线刷新，纳入单元量段；后续核心库改动可被单元量回归捕获。

**迁移策略**：一次性全改（4 示例 main + 4 test*.py + 10 基线），核心库零改动，输出数值零回归（仅新增段）。
