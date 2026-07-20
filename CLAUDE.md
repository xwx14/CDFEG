# CDFEG 项目 AI 上下文（根级）

> 创刀有限元程序生成系统（CDFEG = Chuang-Dao Finite Element Program Generator）：自研有限元基础库（DLL）+ pyTool 代码生成工具链 +示例。

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
- **编译调试环境**：`C:\dev\mingw64`（含 `g++`/`gcc`/`mingw32-make.exe`）。`make` **不在 PATH**，Unix Makefiles 构建需显式 `-DCMAKE_MAKE_PROGRAM=C:/dev/mingw64/bin/mingw32-make.exe`；cmake 位于 `D:\greensoft\cmake-4.1.1`。**本机无 cygwin**（`.claude/rules/` 中关于 cygwin 的假设已过时，实际统一使用 mingw64）。
- **编码**：源文件 UTF-8（含中文注释），MSVC 下必须 `/utf-8`（根 CMake 已配），否则中文注释被按 GBK 解码报错。
- **模型数据文件**：本项目所有模型 `.dat` 文件（GiD 输入，由核心库 `GidPrePost::pre()` 按段解析）**均为文本文件**（UTF-8/ASCII，多为 CRLF 行尾）。若 Read 工具因 CRLF 或特殊字节将其误判为"二进制"而拒绝读取，改用 Bash（`file`/`cat`/`head`）确认与读取即可，切勿当作二进制处理。
- **命名空间** `CDFEG::`；DLL 导出宏 `CDFEG_API`（编译库时定义 `CDFEG_EXPORTS` 走 `dllexport`）。
- `FEMproject/build.bat` 当前仅 `cmake -B`，**未指定 `-G`、也未 `cmake --build`**，完整构建需补全。

## AI 协作指引

- 改核心库 / 示例 / 生成器前，先看对应模块 CLAUDE.md。
- 操作流程（派生单元、生成示例）见 `.claude/skills/`；深度机制参考见 `.claude/rules/`。
