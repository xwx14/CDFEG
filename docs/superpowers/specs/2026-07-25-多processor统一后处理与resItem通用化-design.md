# 设计：多 Processor 统一后处理 + ResItem 通用化

> 日期：2026-07-25
> 范围：CDFEG 核心库（DLL）+ sample 示例集 + pyTool 生成器 + test 回归框架
> 不涉及：`GidPrePost2`（保持现状）

## 1. 背景与目标

当前后处理调用方式分散：

- `del2dData` 自持 `_prePost` 指针，`caculate()` 内每步 `_prePost->post(it)`；
- 静力示例（El2D/Hel2D/ElT3/truss）在 `main` 里直接调 `processor.post()`；
- `DomainData::_processors` 已由 `Processor` 构造函数自动维护（`Processor.cpp:23`），但**无人遍历它**；
- 结果项定义 `GidResItem` 是 GidPrePost 专属，带 GiD 专有枚举，vtkPost 无法复用；
- `vtkPost` 仅 `it==0` 写一次硬编码 `result.vtk`，不支持多步、无路径配置、不按结果项输出。

**目标**：

1. `DomainData` 提供 `post(int it)` 统一入口，遍历 `_processors` 调各自 `post(it)`；派生类不再自持 processor 指针。
2. `GidResItem` → `ResItem`，提升为 `Processor` 基类通用结果项；GiD 专有枚举通用化。
3. `vtkPost` 支持多时间步 PVD 时间序列输出，按 `ResItem` 输出，可配置路径。
4. DEl2D 新增 vtkPost，演示多 processor 并存 + 动力学 PVD 动画。
5. 全部示例迁移到 `data.post(0)` / `post(it)` 统一风格。
6. vtkPost 输出纳入回归测试。

## 2. 总体架构

```
DomainData::_processors  (Processor 构造即自动登记：GidPrePost / vtkPost 实例)
        │
        ▼
DomainData::post(it)  ──for each p──▶  p->post(it)
         (新增, void)                     ├── GidPrePost::post  → 写 *.post.res (多步追加)
                                           └── vtkPost::post     → 写 <base>_<it>.vtu + <base>.pvd

Processor (基类)
  ├── _resItems : vector<ResItem>   (从 GidPrePost 提升到此，每实例一份)
  └── post(it) / pre()              (虚接口，不变)
```

## 3. 详细设计

### 3.1 ResItem 通用化（核心库）

**文件重命名**：`GidResItem.h/.cpp` → `ResItem.h/.cpp`（`FEMproject/CDFEG/CMakeLists.txt` 源列表同步）。

**类与枚举重命名**（`ResItem.h`）：

| 旧 | 新 |
|---|---|
| `class GidResItem` | `class ResItem` |
| `enum class GidLocation { OnNodes, OnGaussPoints }` | `enum class ResLocation { OnNodes, OnGaussPoints }` |
| `enum class GidResultType { 17 个 GiD 专有值 }` | `enum class ResType { Scalar, Vector, Matrix }` |

**辅助函数**（`ResItem.cpp`）：

| 旧 | 新 |
|---|---|
| `gidLocationToStr / strToGidLocation` | `resLocationToStr / strToResLocation` |
| `gidResultTypeToStr / strToGidResultType` | `resTypeToStr / strToResType`（仅 Scalar/Vector/Matrix 三值） |
| `gidResultTypeComponents` | **删除**（分量数改用 `item._ValNames.size()`） |

`ResItem` 成员不变：`_name / _type(ResType) / _loc(ResLocation) / _iFields / _ValNames`，构造与 `addVal` 签名不变（仅类型名替换）。

**`_resItems` 归属迁移**：`std::vector<ResItem> _resItems` 从 `GidPrePost`（`gidPrePost.h:70`）移到 `Processor` 基类（`Processor.h`）。每个 Processor 实例各持一份。`GidPrePost2` 继承到该成员但不使用（无影响）。

**GidPrePost 内部适配**（`gidPrePost.cpp`）：

- `for (GidResItem& item : _resItems)` → `for (ResItem& item : _resItems)`；
- `item._loc == GidLocation::OnGaussPoints` → `ResLocation::OnGaussPoints`；
- `gidResultTypeToStr(item._type)` → `resTypeToStr(item._type)`（返回 "Scalar"/"Vector"/"Matrix"，GiD res 文件格式兼容）。

### 3.2 DomainData 统一后处理接口（核心库）

`DomainData.h` 新增声明，`DomainData.cpp` 新增实现：

```cpp
// DomainData.h（public 区）
void post(int it = 0);

// DomainData.cpp
void DomainData::post(int it) {
    for (Processor* p : _processors) {
        p->post(it);
    }
}
```

返回 `void`：post 为输出操作，错误在子 processor 内部 `std::cerr` 报告。`DomainData.h` 顶部 `class Processor;` 前向声明已存在。

### 3.3 vtkPost 多步 PVD 输出（核心库）

`vtkPost.h` 改造：

```cpp
class CDFEG_API vtkPost : public Processor {
public:
    vtkPost(DomainData* data);
    ~vtkPost();
    // 与 GidPrePost 一致的路径配置：输出 parentPath/baseName_<it>.vtu + baseName.pvd
    void setFilePath(const std::string& parentPath, const std::string& baseName);
    virtual int post(int it = 0);   // 每步写一个 vtu，累积到 _steps；末步或析构时写 pvd
private:
    int writeVTU(const std::string& fn);          // 原 writeVTK 改名，内容按 _resItems
    int writePVD(const std::string& fn);          // 新增：汇总 _steps
    std::string _outPath = ".";                   // 默认当前目录
    std::string _baseName = "result";             // 默认基名
    // 已写步记录：(it, time=it*_dt)，供 pvd 汇总；post(it) 每步 push_back
    std::vector<std::pair<int, double>> _steps;
};
```

**输出依据**：遍历 `Processor::_resItems`（基类成员）：

- `OnNodes`：取 `phy = _femData->_phyDatas[item._iFields[0]]`，按 `item._ValNames` 从 `phy->_nodeRes[name]` 取值，按节点写 `<DataArray Name=item._name NumberOfComponents=N>`；
- `OnGaussPoints`：从 `phy->_elemRes[name]` 取值，按单元写 CellData；
- `N = item._ValNames.size()`（Scalar=1，Vector=2/3，Matrix=3/6 由实际分量数决定）。

**文件命名**：`<outPath>/<baseName>_<it:04d>.vtu`（零填充 4 位）+ `<outPath>/<baseName>.pvd`。

**PVD 内容**：标准 Collection 格式，每步一个 `<DataSet timestep="time" part="0" file="<baseName>_<it:04d>.vtu"/>`，`time = it * _femData->_dt`。

**PVD 写入时机**：每次 `post(it)` 写完 vtu 后，重写整个 `.pvd`（基于 `_steps` 全量重写，幂等）。这样即便程序中途异常退出，已写步的 pvd 仍有效。析构不写（避免与手动重写重复）。

**数值精度**（回归关键）：vtu 输出数值用 `std::setprecision(15) << std::scientific`（当前默认 6 位不足以支撑 1e-12 回归容差）。

**静力兼容**：`it=0` 单步 → `base_0000.vtu` + `base.pvd`（单步 Collection）。

**扩展名正名**：`.vtk` → `.vtu`（内容本就是 VTU XML UnstructuredGrid）。

### 3.4 示例迁移

#### DEl2D（`sample/DEl2D/`）

- `del2dData.h`：删除 `void setPost(CDFEG::GidPrePost* prePost);` 与 `CDFEG::GidPrePost* _prePost = nullptr;`，删除 `class GidPrePost;` 前向声明（不再需要）。
- `del2dData.cpp`：删除 `setPost` 实现；`caculate()` 内 `if (_prePost) _prePost->post(it);` → `post(it);`（调本类继承自 DomainData 的 `post`）。
- `main.cpp`：
  - 删除 `data.setPost(&gidPrePost);`；
  - `GidResItem`/`GidResultType` → `ResItem`/`ResType`；
  - 新增 `CDFEG::vtkPost vtkpost(&data); vtkpost.setFilePath(path, project);`；
  - 用一个 lambda 对 `gidPrePost` 与 `vtkpost` 各注册一份相同的 ResItem（disp/velocity/acceleration/stress），避免重复书写：
    ```cpp
    auto registerItems = [](CDFEG::Processor& p) {
        CDFEG::ResItem disp("disp", CDFEG::ResType::Vector);
        disp.addVal(0, "u"); disp.addVal(0, "v");
        p._resItems.push_back(disp);
        // velocity / acceleration / stress 同理
    };
    registerItems(gidPrePost);
    registerItems(vtkpost);
    ```
  - `data.caculate();` 内部已通过 `post(it)` 同时驱动 GidPrePost + vtkPost。

#### El2D / Hel2D / ElT3（`sample/{El2D,Hel2D,ElT3}/main.cpp`）

- `gidPrePost.post();` → `data.post(0);`
- `GidResItem`/`GidResultType` → `ResItem`/`ResType`（`OnGaussPoints` 处 `GidLocation` → `ResLocation`）。

#### truss1D / truss2D（`sample/truss{1D,2D}/main.cpp`）

- 删除 `CDFEG::vtkPost vtkpost(&data); vtkpost.post();` 两行；
- `data.caculate();` 之后改调 `data.post(0);`（驱动 vtkPost）；
- **补注册 ResItem**（否则 vtkPost 输出为空）：
  - truss2D：`disp`(Vector, OnNodes, u, v)、`force`(Scalar, OnGaussPoints, T)、`stress`(Scalar, OnGaussPoints, sigma)；
  - truss1D：`disp`(Scalar, OnNodes, u)、`force`(Scalar, OnGaussPoints, T)、`stress`(Scalar, OnGaussPoints, sigma)；
- 保留原有 `*.txt` 手工输出（未纳入回归，仅 vtkPost 纳入）。

### 3.5 pyTool 同步（`pyTool/`）

- `template/mainGid.cpp.j2`：`CDFEG::GidResItem` → `CDFEG::ResItem`，`GidResultType::X` → `ResType::X`，`GidLocation::OnGaussPoints` → `ResLocation::OnGaussPoints`；`gidPrePost._resItems` 引用不变（继承自基类）。
- `DataProject.py`：注释/文档串中 `GidResItem` → `ResItem`。
- `test/testElT3.py` 注释同步。
- 生成器测试（`pyTool/test/`）需重跑确认生成产物与手写示例一致。

### 3.6 回归测试扩展（`test/`）

**目标**：新增 `del2d1_vtk` e2e case，对比 vtkPost 产出的 PVD 时间序列。

**parser 扩展**（`test/framework/parser.py`）：

- 新增 `parse_vtu_file(path, step) -> dict[(name, step), ResBlock]`：解析 VTU XML，PointData/CellData 每个 `<DataArray Name="X">` 产出一个 ResBlock：
  - `result_name = Name`，`step = step`（由调用方传入），
  - `components = [f"comp_{i}" for i in range(NumberOfComponents)]`（VTU 不存分量名，统一用 comp_N；actual/baseline 同规则即可对齐），
  - `values = {entity_id: [v0, v1, ...]}`（PointData 按节点序，CellData 按单元序，entity_id 从 1 起递增），
  - `location = "OnNodes"` / `"OnCells"`。
- 新增 `parse_pvd_file(path) -> dict[(name, step), ResBlock]`：解析 `.pvd`，按 `<Collection>` 中 `<DataSet>` 顺序（顺序索引即 step=0,1,2,…）逐个 `parse_vtu_file`，合并结果。

**case.py 扩展**：

- `_parse` 新增分支：`format == "pvd"` → `parse_pvd_file(path)`。
- `_prepare_work_dir` 拷贝排除规则追加：排除 `.vtu`、`.pvd`（vtk 基线不拷入工作目录，避免被 exe 误读为输入）。

**update.py 扩展**：

- `format == "pvd"` 时，`_run_in_work_dir` 返回主文件 `.pvd`；确认更新时，把工作目录中 `<base>_*.vtu` 群 + `<base>.pvd` 整体拷贝到 `case_dir`（覆盖旧基线群）。
- diff 摘要对 pvd 解析结果照常 `compare`。

**config.toml 新增 case**：

```toml
[[suite.e2e.cases]]
name = "del2d1_vtk"
target = "del2d"
project = "del2d"
case_dir = "models/del2d1_vtk.gid"
baseline = "del2d.pvd"
output = "del2d.pvd"
tol_atol = 1e-12
tol_rtol = 0.0
format = "pvd"
```

**新 case_dir**：`test/models/del2d1_vtk.gid/`，初始含 `del2d.dat`（拷贝自 `del2d1.gid/del2d.dat`）。vtk 基线（`del2d.pvd` + `del2d_0000.vtu`…`del2d_0010.vtu`）由首次 `--update del2d1_vtk` 生成冻结。

> 说明：`del2d1`（res）与 `del2d1_vtk`（pvd）复用同一 `del2d` target，build 复用，仅 run 两次；del2d 动力学 11 步耗时秒级，开销可接受。独立 case_dir 使 res/vtk 基线职责分离。

### 3.7 文档同步

- `FEMproject/CDFEG/代码解释.md`：4.6 节 `GidResItem` → `ResItem`，4.3 节 vtkPost 描述更新为多步 PVD。
- `FEMproject/CDFEG/升级说明.md`：`vtkPost`/`gidPrePost` 描述同步。
- `FEMproject/sample/DEl2D/`、`ElT3/` 等说明 md：`GidResItem`/`post2` 字样更新。
- `.claude/rules/pyTool能力边界.md`、`.claude/skills/{macs-to-cdfeg,add-regression-case}/SKILL.md`：`GidResItem` → `ResItem`、`post2` → `post` 字样。

## 4. 文件改动清单

**核心库**（`FEMproject/CDFEG/`）：

- 重命名：`GidResItem.h/.cpp` → `ResItem.h/.cpp`
- 改：`ResItem.h/.cpp`（类/枚举/函数重命名）、`Processor.h`（加 `_resItems` 成员）、`DomainData.h/.cpp`（加 `post`）、`vtkPost.h/.cpp`（多步 PVD + setFilePath）、`gidPrePost.h/.cpp`（去 `_resItems`、用新枚举名）、`CMakeLists.txt`（文件重命名同步）

**示例**（`FEMproject/sample/`）：

- 改：`DEl2D/{del2dData.h,del2dData.cpp,main.cpp}`、`El2D/main.cpp`、`Hel2D/main.cpp`、`ElT3/main.cpp`、`truss1D/main.cpp`、`truss2D/main.cpp`

**pyTool**（`pyTool/`）：

- 改：`template/mainGid.cpp.j2`、`DataProject.py`、`test/testElT3.py`

**回归**（`test/`）：

- 改：`framework/parser.py`（加 vtu/pvd 解析）、`framework/case.py`（pvd format 分支 + 排除规则）、`framework/update.py`（多文件基线）、`config.toml`（加 del2d1_vtk）
- 新增：`models/del2d1_vtk.gid/del2d.dat`

**文档**：上述 3.7 列出各 md。

## 5. 验证方法

1. **全量 clean 重建**（mingw64）：`test/run_tests.py --rebuild`，确认核心库 DLL + 全部示例编译通过（ResItem 重命名后无遗留旧符号）。
2. **e2e 回归全绿**：`python test/run_tests.py`，原有 case（del2d1/hel2d1/el2d1/elt3_*/truss*/el2d_bf1/el2_mfel*）max|Δ| 不回归。
3. **新增 vtk 回归**：首次 `python test/run_tests.py --suite e2e --update del2d1_vtk` 冻结基线；之后 `python test/run_tests.py --suite e2e --case del2d1_vtk` 通过，max|Δ| ≤ 1e-12。
4. **手动核查 DEl2D 产物**：`test/models/del2d1_vtk.gid/` 运行后得到 `del2d.post.res`（多步）+ `del2d_0000.vtu`…`del2d_0010.vtu` + `del2d.pvd`；用 ParaView 打开 `del2d.pvd` 可见 11 步时间动画。
5. **生成器测试**：`python test/run_tests.py --suite generator` 通过，生成产物与手写示例一致。

## 6. 风险与对策

| 风险 | 对策 |
|---|---|
| ResItem 重命名遗留旧符号导致编译/链接失败 | 全量 `--clean-first` 重建；grep 确认无残留 `GidResItem/GidResultType/GidLocation` |
| vtkPost 输出精度不足致回归超差 | vtu 数值用 `setprecision(15) scientific` |
| `_processors` 悬垂指针（生命周期） | `_processors` 为**观察指针**：`DomainData::~DomainData()` 仅 `delete _phyDatas`，不 delete `_processors`（已核实 `DomainData.cpp:21-26`）。main 中 processor 为栈对象，按声明逆序先于 data 析构；`post(it)` 仅在 `caculate()` 内调用（此时 processor 存活），data 析构不访问 `_processors`，安全。新增 vtkPost 遵循同约定（栈对象、main 管理生命周期） |
| VTU 解析分量名缺失 | 统一 `comp_N` 命名，actual/baseline 同规则对齐，comparator 不依赖真实分量名 |
| pvd 中 timestep 浮点无法映射整数 step | parse_pvd 用 DataSet 顺序索引作 step，不依赖 timestep 数值 |
