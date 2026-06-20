# pyTool 生成器能力边界

> 本文件是 pyTool 代码生成器**能自动生成什么 / 不能生成什么**的深度参考，基于 `template/*.j2` 与 `test/testDEL2D.py` 源码梳理。
> **核心结论**：pyTool 是"项目脚手架"，生成类结构与构建配置；**计算逻辑与后处理仍需人工填充**。
> **关联**：pyTool [CLAUDE.md](../../pyTool/CLAUDE.md)；操作流程见 skill `generate-sample`。

---

## 能自动生成

| 产物 | 来源模板 | 内容 |
| --- | --- | --- |
| FEMData 派生类 | `femdata.*.j2` | `_dim` + 注册物理场 + `caculate`（命令流渲染） |
| PhyFieldData 派生类 | `phyfielddata.*.j2` | `_name/_dispNames/_dof2` + 注册单元 + `_eleResNames` |
| 单元派生类 | `elesub.*.j2` | 构造（元信息 + 积分参数 + `caculateShapeCoef` + resize 结果）+ `shapeFun`（从字符串表达式）+ **空的 `run`/`uEle` 体** |
| main 入口 | `main.cpp.j2` / `mainGid.cpp.j2` | GiD 文件入口：`setFilePath→pre→caculate→post` |
| CMake | `cmake.j2` / `cmakeSln.j2` | 项目级（`iProgramType`: 0=exe / 1=dll / 2=static）+ 解决方案级（`add_subdirectory`） |
| GiD 配置 | `gidbas/gidprb/gidcnd/gidbat.j2` | `.bas/.prb/.cnd/.bat` |
| caculate 命令流 | `imp.cmd.j2` | 完整 `initMatrix→eProgram→solve→uPhy→calRightVals`（目前唯一命令） |

## 不能生成（需人工填充）

1. **单元计算逻辑**：`run()` 体（`ele.runCode`）、`uEle()` 体（`ele.uCode`）——高斯积分、B 矩阵、单刚组装、应力计算全部手写。`testDEL2D.py` 未设 `runCode`，故生成的 `ElQ4g` 是**空壳**。
2. **物理场后处理**：模板**不生成 `uPhy` 重写**；应力外推、von Mises 等需人工补（对比手写 `ElDispFieldData::uPhy`）。
3. **GiD 结果项**：生成 main 用 `post()`（非 `post2()`），**不注册 `GidResItem`**——结果输出可能不全（对比手写 El2D main）。
4. **动力学**：数据结构有 `bDynamic`/`pdeType`/`sch` 字段，但**无代码生成**——Newmark 有效矩阵、时间步循环、`_bSavedData0` 管理全手写。
5. **热传导**：`testHeat2D.py` 为空文件，**未实现**。
6. **细粒度命令流**：仅 `imp` 一种命令模板，无法组合自定义求解流程。

## 对齐度（生成 vs 手写示例）

| 对比项 | 手写 El2D/DEl2D | pyTool 生成 |
| --- | --- | --- |
| 单元构造 | 完整 | **对齐**（元信息 + 积分参数一致） |
| `shapeFun` | 完整 | **对齐**（从 `shapeFuns` 字符串生成） |
| `run`/`uEle` 体 | 完整计算逻辑 | **空壳**（仅 `{{runCode}}`/`{{uCode}}` 占位） |
| `uPhy` 重写 | 应力外推 + von Mises | **不生成**（用基类，仅回填位移） |
| main 结果输出 | `post2()` + 注册 `GidResItem` | `post()`，无注册 |

> ⚠️ 勿运行 `testDEL2D.py`：它会用空壳 `ElQ4g` 覆盖手写的 `NewmarkQ4g` 动力学逻辑。
