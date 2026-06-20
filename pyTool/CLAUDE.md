[根 CLAUDE.md](../CLAUDE.md) > **pyTool**

# pyTool — 有限元代码生成工具

> 路径 `pyTool/`；Python 3 + Jinja2，根据数据结构描述自动生成符合 CDFEG 约定的 C++ 程序 + CMake + GiD 文件，落入 `FEMproject/sample/`。

## 入口

- 无统一 CLI；每个 `test/test*.py` 既是测试也是生成入口。
- 运行：在 `pyTool/` 下 `python test/testXxx.py`（脚本 `sys.path.append` 上级目录以导入 Maker）。

## 数据结构（三层）

```
DataProject（项目）→ DataField（场）→ DataEleSub / DataEleSubG（单元，G 为高斯积分）
```

字段与签名查 codegraph；完整操作流程见 skill `generate-sample`。

## 生成器与模式

| 生成器 | 职责 |
| --- | --- |
| `MakerCpp` | C++/CMake 生成。`mode='new'`（复制 CDFEG 库 + third，生成解决方案 CMake）/ `mode='add'`（仅项目文件，向现有 CMake 追加 `add_subdirectory`）。`mainMode` 0=makeData / 1=GiD |
| `MakerGidFile` | GiD `.bas/.prb/.cnd/.bat` |

## 模板清单（`pyTool/template/`）

`main.cpp.j2`/`mainGid.cpp.j2`、`femdata.*.j2`、`phyfielddata.*.j2`、`elesub.*.j2`、`cmake.j2`/`cmakeSln.j2`、`gidbas`/`gidprb`/`gidcnd`/`gidbat.j2`、`imp.cmd.j2`（唯一命令模板：渲染 `initMatrix→eProgram→solve→uPhy→calRightVals`）。

## 关键陷阱

- **`test/test*.py` 是代码生成器，运行会覆盖 `FEMproject/sample/` 下对应示例的手写 C++ 源码**。**DEl2D 手写，勿运行 `testDEL2D.py`**（空壳 `ElQ4g` 覆盖 `NewmarkQ4g`）。

## 深度参考

生成器能力边界（能 / 不能生成什么）：[`.claude/rules/pyTool能力边界.md`](../.claude/rules/pyTool能力边界.md)。
