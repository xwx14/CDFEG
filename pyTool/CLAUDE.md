[根 CLAUDE.md](../CLAUDE.md) > **pyTool**

# pyTool — 有限元代码生成工具

> 路径：`pyTool/`  
> 语言：Python 3（无构建系统，直接运行）  
> 核心依赖：Jinja2  
> 用途：根据数据结构定义自动生成符合 CDFEG 核心库约定的 C++ 程序、CMake 与 GiD 问题类型文件

## 变更记录 (Changelog)

| 时间 | 说明 |
| --- | --- |
| 2026-06-18 15:43:22 | init-architect 首次生成模块文档 |
| 2026-06-18 | 深挖补充 | 新增第十节「生成器能力边界」：梳理模板骨架、命令流（仅 `imp`）、与手写示例的对齐度；修正 `testHeat2D.py` 为空文件、`imp.cmd.j2` 描述 |

---

## 一、模块职责

把"有限元问题的数据结构描述"（项目→场→单元）转换为可编译的 C++ 源码 + 构建脚本 + GiD 配置文件，避免手工重复编写模板化的样板代码。生成结果直接落入 `FEMproject/sample/` 各示例目录。

## 二、入口与启动

- 无统一 CLI 入口；每个 `test/test*.py` 既是测试用例也是生成入口。
- 典型调用方式见根文档 5.2 节。
- 运行方式：在 `pyTool/` 目录下 `python test/testEl2D.py`（脚本内 `sys.path.append` 上级目录以导入 Maker）。

## 三、架构与对外接口

### 3.1 数据结构层（描述"要生成什么"）

```
DataProject（项目：name, dim, coordVars, fields[], cmds[]）
   └── DataField（场：name, dispNames, eleSubs[], pdeType, bDynamic, sch: DataSch）
          └── DataEleSub（单元：name, nNodes, type, dispNames, paramNames, runCode, uCode, ...）
                 └── DataEleSubG（高斯积分单元，继承 DataEleSub：
                                  gaussOrder, gaussPoints[], gaussWeights[], shapeFuns[]）
```

| 文件 | 类 | 关键字段 |
| --- | --- | --- |
| `DataProject.py` | `DataProject` | `name` / `dim` / `coordVars` / `fields` / `cmds`（求解命令流，供 `parseCmds` 生成 caculate 代码）/ `caculateCode` |
| `DataField.py` | `DataField` | `name` / `fieldDataClassName` / `eleSubs` / `pdeType`（1椭圆/2抛物/3双曲）/ `dispNames` / `eleResNames` / `index` / `bDynamic` / `sch: DataSch` |
| `DataEleSub.py` | `DataEleSub` | `name` / `nNodes` / `type`（0点/1线/2面/3体）/ `dispNames` / `paramNames` / `paramValues` / `runCode` / `uCode` / `initCode` / `baseClass`（默认 `ElementBase`，等参元改 `IsoEleBase`）/ `calMatrix`（默认 `['eload','estif','emass','edamp']`）/ `vtkCellType` |
| `DataEleSubG.py` | `DataEleSubG(DataEleSub)` | 增加 `gaussOrder` / `gaussPoints[]` / `gaussWeights[]` / `shapeFuns[]`（形函数字符串表达式，`x[1]/x[2]` 占位符） |
| `DataSch.py` | `DataSch` | 求解方案（当前为占位空类，`toDict/fromDict` 返回空） |

> 所有数据类提供 `toDict()` / `fromDict()` 用于 JSON 序列化（`testEl2D.py` 末尾会写 `data.json`）。

### 3.2 生成器层（MakerBase 体系）

| 文件 | 类 | 职责 |
| --- | --- | --- |
| `MakerBase.py` | `MakerBase(ABC)` | Jinja2 环境初始化（`template/` 目录、`trim_blocks/lstrip_blocks`）、`write2String` / `write2File` / 抽象 `makeAll`。含 emoji 降级工具 `safePrint` / `_stripEmoji`（GBK 控制台兼容） |
| `MakerCpp.py` | `MakerCpp(MakerBase)` | C++/CMake 生成，支持 `mode='new'`（生成完整解决方案：复制 CDFEG 库 + third + 解决方案 CMake）与 `mode='add'`（仅生成项目文件，向现有 CMake 追加 `add_subdirectory`）。`mainMode` 0=makeData 形式 / 1=GiD 数据文件形式 |
| `MakerGidFile.py` | `MakerGidFile(MakerBase)` | GiD 文件生成：`.bas`（模板）/ `.prb`（问题配置）/ `.cnd`（条件）/ `.bat`（批处理）。可从 `DataProject` 或裸参数初始化 |

### 3.3 MakerCpp 生成流程（makeAll）

```
makeAll()
├── mode='new': _copy_cdfeg_lib() + _copy_third()   # 复制依赖
├── for project, iProgramType in projects:
│     ├── _build_render_data(project)                # 场补齐 dof2/headerGuard/eleResNames
│     ├── makeProject(project, outPath, iProgramType)
│     │     ├── _makeMain(...)        [仅 iProgramType==0]   # main.cpp.j2 或 mainGid.cpp.j2
│     │     ├── _makeFEMData(...)     # parseCmds(cmds) → caculateCode；femdata.h.j2 / .cpp.j2
│     │     ├── for field: _makePhyFieldData(...)            # phyfielddata.h.j2 / .cpp.j2
│     │     ├── for ele: _makeEleSub(...)                    # elesub.h.j2 / .cpp.j2
│     │     └── _makeCMakeLists(...)                         # cmake.j2
│     └── mode='add': _add_to_existing_sln()         # 追加 add_subdirectory
└── mode='new': makeSlnCMake()                       # cmakeSln.j2
```

`parseCmds(project)` 把 `project.cmds`（命令列表，每条 `[cmdName, phyIndex, ...]`）渲染为 `caculate` 方法体内的 C++ 代码字符串，模板为 `<cmdName>.cmd.j2`。**目前 template 下仅 `imp.cmd.j2` 一种命令**（渲染完整 `initMatrix→eProgram→solve→uPhy→calRightVals` 流程）；`template/cppCode/` 目录为空（历史遗留）。

### 3.4 辅助

- `vtkCellType.py`：`VTKCellType` 枚举 + `get_vtk_cell_type(type, nNodes)` + `get_vtk_cell_type_name`，是 C++ 端 `vtkCellType.h` 的 Python 镜像，供 `DataEleSub.inferVTKCellType()` 推断用。

## 四、关键依赖与配置

- **Jinja2**（`from jinja2 import Environment, FileSystemLoader`）：模板引擎。
- 模板目录：`pyTool/template/`（相对 MakerBase.py 所在目录）。
- 路径硬编码：`MakerCpp` 中 `CDFEG_LIB_DIR = ../FEMproject/CDFEG`、`THIRD_DIR = ../FEMproject/third`（基于 `__file__` 推算，要求 pyTool 与 FEMproject 同级）。
- 无 `requirements.txt` / `pyproject.toml`；运行前需 `pip install jinja2`。

## 五、模板清单（pyTool/template/）

| 模板 | 生成目标 |
| --- | --- |
| `main.cpp.j2` / `mainGid.cpp.j2` | 示例 main.cpp（makeData / GiD 文件两种入口） |
| `femdata.h.j2` / `femdata.cpp.j2` | `<Project>Data.h/.cpp`（派生 FEMData） |
| `phyfielddata.h.j2` / `phyfielddata.cpp.j2` | `<Field>FieldData.h/.cpp`（派生 PhyFieldData） |
| `elesub.h.j2` / `elesub.cpp.j2` | `<Ele>.h/.cpp`（派生 ElementBase/IsoEleBase） |
| `cmake.j2` / `cmakeSln.j2` | 项目级 / 解决方案级 CMakeLists.txt |
| `gidbas.j2` / `gidprb.j2` / `gidcnd.j2` / `gidbat.j2` | GiD `.bas/.prb/.cnd/.bat` |
| `vcxproj.j2` / `vcxproj.filters.j2` | VS 工程文件（备用） |
| `imp.cmd.j2` | caculate 命令流模板：渲染 `initMatrix→eProgram→solve→uPhy→calRightVals` 完整流程（命令名 `imp`，目前唯一命令模板） |
| `pack_templates.py` | 打包模板到 `templates.db` |
| `templates.db` | 打包后的模板库（二进制） |

> `template/cppCode/` 子目录当前为**空**（历史遗留，命令模板现已合并到 template 根，仅剩 `imp.cmd.j2`）。

## 六、数据模型

见 3.1 节。命令流 `cmds` 示例（驱动 `caculate` 内的 C++ 代码生成）：每条命令是 `[模板名, 场索引, ...]`，例如 `["eProgram", 0]` 渲染 `eProgram.cmd.j2` 生成 `field0->eProgram();`。

## 七、测试与质量

- `test/test*.py`（8 个文件，其中 `testHeat2D.py` 为 **0 字节空文件**——热传导生成器未实现）：`test1DTruss` / `test2DTruss` / `test1DEulerBeam` / `Test1DTimoshenkobeam` / `testEl2D` / `testElT3` / `testDEL2D` / `testHeat2D`(空)。
- **重要**：这些"测试"是**代码生成器**，运行会**覆盖** `FEMproject/sample/` 下对应示例的手写 C++ 源码。修改手写示例前务必确认是否由 pyTool 生成（见 `sample/DEl2D/升级说明.md` 5.11 节：DEl2D 是手写的，运行 testDEL2D.py 会用 `ElQ4g` 名覆盖）。
- 无 pytest 断言式单测。

## 八、常见问题 (FAQ)

1. **Q: 生成后示例编译报错？**  
   A: 检查 `DataEleSub.baseClass` 是否正确（等参元必须 `IsoEleBase`）、`inferVTKCellType()` 是否被调用、`shapeFuns` 的 `x[i]` 占位符是否被 `_replaceCoordVars` 正确替换为 `coordVars`。

2. **Q: emoji 报 UnicodeEncodeError？**  
   A: Windows GBK 控制台问题，`safePrint` 已自动降级。或设 `PYTHONUTF8=1`。

3. **Q: mode='add' 不追加 add_subdirectory？**  
   A: 检查 `sln_cmake_path` 是否提供且文件存在；重复运行会因已存在而跳过。

## 九、相关文件清单

数据结构：`DataProject.py` / `DataField.py` / `DataEleSub.py` / `DataEleSubG.py` / `DataSch.py` / `vtkCellType.py`  
生成器：`MakerBase.py` / `MakerCpp.py` / `MakerGidFile.py`  
模板：`template/*.j2`（17 个）+ `template/pack_templates.py` + `template/templates.db`  
测试/入口：`test/test*.py`（8 个）

## 十、生成器能力边界（深挖补充）

> 基于 `template/*.j2` 与 `test/testDEL2D.py` 源码梳理。**核心结论：pyTool 是"项目脚手架"，生成类结构与构建配置；计算逻辑与后处理仍需人工填充。**

### 10.1 能自动生成

| 产物 | 来源模板 | 内容 |
| --- | --- | --- |
| FEMData 派生类 | `femdata.*.j2` | `_dim` + 注册物理场 + `caculate`（命令流渲染） |
| PhyFieldData 派生类 | `phyfielddata.*.j2` | `_name/_dispNames/_dof2` + 注册单元 + `_eleResNames` |
| 单元派生类 | `elesub.*.j2` | 构造（元信息 + 积分参数 + `caculateShapeCoef` + resize 结果）+ `shapeFun`（从字符串表达式）+ **空的 `run`/`uEle` 体** |
| main 入口 | `main.cpp.j2` / `mainGid.cpp.j2` | GiD 文件入口：`setFilePath→pre→caculate→post` |
| CMake | `cmake.j2` / `cmakeSln.j2` | 项目级（`iProgramType`: 0=exe / 1=dll / 2=static）+ 解决方案级（`add_subdirectory`） |
| GiD 配置 | `gidbas/gidprb/gidcnd/gidbat.j2` | `.bas/.prb/.cnd/.bat` |
| caculate 命令流 | `imp.cmd.j2` | 完整 `initMatrix→eProgram→solve→uPhy→calRightVals`（目前唯一命令） |

### 10.2 不能生成（需人工填充）

1. **单元计算逻辑**：`run()` 体（`ele.runCode`）、`uEle()` 体（`ele.uCode`）——高斯积分、B 矩阵、单刚组装、应力计算全部手写。`testDEL2D.py` 未设 `runCode`，故生成的 `ElQ4g` 是**空壳**。
2. **物理场后处理**：模板**不生成 `uPhy` 重写**；应力外推、von Mises 等需人工补（对比手写 `ElDispFieldData::uPhy`）。
3. **GiD 结果项**：生成 main 用 `post()`（非 `post2()`），**不注册 `GidResItem`**——结果输出可能不全（对比手写 El2D main）。
4. **动力学**：数据结构有 `bDynamic`/`pdeType`/`sch` 字段，但**无代码生成**——Newmark 有效矩阵、时间步循环、`_bSavedData0` 管理全手写（见 `sample/DEl2D/升级说明.md`）。
5. **热传导**：`testHeat2D.py` 为空文件，**未实现**。
6. **细粒度命令流**：仅 `imp` 一种命令模板，无法组合自定义求解流程。

### 10.3 对齐度（生成 vs 手写示例）

| 对比项 | 手写 El2D/DEl2D | pyTool 生成 |
| --- | --- | --- |
| 单元构造 | 完整 | **对齐**（元信息 + 积分参数一致） |
| `shapeFun` | 完整 | **对齐**（从 `shapeFuns` 字符串生成） |
| `run`/`uEle` 体 | 完整计算逻辑 | **空壳**（仅 `{{runCode}}`/`{{uCode}}` 占位） |
| `uPhy` 重写 | 应力外推 + von Mises | **不生成**（用基类，仅回填位移） |
| main 结果输出 | `post2()` + 注册 `GidResItem` | `post()`，无注册 |

> ⚠️ 这正是 `sample/DEl2D/升级说明.md` 5.11 节警告"勿运行 `testDEL2D.py`"的根因：它会用空壳 `ElQ4g` 覆盖手写的 `NewmarkQ4g` 动力学逻辑。
