# CDFEG 项目 AI 上下文（根级）

> 创刀有限元程序生成系统（CDFEG = Chuang-Dao Finite Element Program Generator）：自研有限元基础库（DLL）+ pyTool 代码生成工具链 + 6 个示例。

## 模块索引

| 模块 | 路径 | 职责 | 文档 |
| --- | --- | --- | --- |
| 核心库（DLL） | `FEMproject/CDFEG/` | 三层架构基础库（单元→物理场→总体数据）+ GiD/VTK/INP 前后处理 | [CLAUDE.md](./FEMproject/CDFEG/CLAUDE.md) |
| 示例集 | `FEMproject/sample/` | 6 示例：truss1D/2D/3D、El2D、ElT3、DEl2D | [CLAUDE.md](./FEMproject/sample/CLAUDE.md) |
| 代码生成工具 | `pyTool/` | Jinja2 模板驱动的 C++/CMake/GiD 生成器 | [CLAUDE.md](./pyTool/CLAUDE.md) |
| 开发工具 | `DevTool/` | `add_license_header.py` 批量加 GPL 头 | （单文件） |
| 第三方 | `FEMproject/third/Eigen/` | Eigen 3.4.0（header-only） | （不写文档） |

## 构建与编码约定

- **C++14**，CMake；生成器 MinGW Makefiles 或 MSVC。
- **编码**：源文件 UTF-8（含中文注释），MSVC 下必须 `/utf-8`（根 CMake 已配），否则中文注释被按 GBK 解码报错。
- **命名空间** `CDFEG::`；DLL 导出宏 `CDFEG_API`（编译库时定义 `CDFEG_EXPORTS` 走 `dllexport`）。
- `FEMproject/build.bat` 当前仅 `cmake -B`，**未指定 `-G`、也未 `cmake --build`**，完整构建需补全。

## 跨模块陷阱（必读）

- **pyTool test 脚本会覆盖 sample 手写 C++ 源码**：`test/test*.py` 是代码生成器，运行即覆盖对应示例。**DEl2D 是手写动力学示例，勿运行 `testDEL2D.py`**（会用空壳覆盖 `NewmarkQ4g`）。
- **dat 材料名约定**：dat elem 段 `name` 与材料段 `mat_<name>` 必须与单元 `_name` 完全一致，否则材料读取失败、单刚全 0。
- **dat 行尾与 TextReader**：`TextReader` 必须以 binary 模式打开（`std::ios::binary`）——`preLine` 依赖 `seekg` 字节精确回退，文本模式对 CRLF 文件 `tellg/seekg` 错位会使 `pre()` 跳过 coord/elem 段导致崩溃（曾致 DEl2D `writeNodes` 越界 SIGSEGV）。仓库以 `.gitattributes` 约定 `*.dat eol=lf`，库已兼容 CRLF/LF。

## AI 协作指引

- 改核心库 / 示例 / 生成器前，先看对应模块 CLAUDE.md。
- 操作流程（派生单元、生成示例）见 `.claude/skills/`；深度机制参考见 `.claude/rules/`。
- 结构性问题（调用关系、影响面）用 codegraph；字面查询（字符串、注释）用 grep/read。
- 遵循 `.gitignore`，跳过 `build/`、`__pycache__/`、`.codegraph/`、二进制与 Eigen 源码。
