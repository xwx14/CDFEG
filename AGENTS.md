# CDFEG 项目开发指南

## 项目概述

CDFEG（创道有限元程序生成系统）是一个有限元分析（FEA）框架，由两个核心模块组成：

- **FEMproject/** — C++ 有限元核心库（DLL）及示例程序
- **pyTool/** — Python 代码生成工具（基于 Jinja2 模板引擎，自动生成 C++ FEM 项目代码）

## 项目结构

```
CDFEG/
├── FEMproject/                    # C++ 有限元核心库
│   ├── CMakeLists.txt             # 顶层 CMake（解决方案级）
│   ├── build.bat                  # Windows 构建脚本（仅 configure）
│   ├── CDFEG/                     # 核心库源码（编译为 SHARED DLL）
│   │   ├── CDFEG.h                # DLL 导出宏定义
│   │   ├── ElementBase.h/.cpp     # 单元基类（所有单元的基类）
│   │   ├── IsoEleBase.h/.cpp      # 等参单元基类（形函数、雅各比矩阵）
│   │   ├── PhyFieldData.h/.cpp    # 物理场数据（总刚组装、求解、后处理）
│   │   ├── FemData.h/.cpp         # 有限元空间数据（节点、单元、材料管理）
│   │   ├── EquationSystem.h/.cpp  # 稀疏线性方程组求解器（Eigen）
│   │   ├── MatrixFun.h/.cpp       # 矩阵向量运算工具库
│   │   ├── Processor.h/.cpp       # 前后处理器基类
│   │   ├── vtkPost.h/.cpp         # VTK 后处理器（VTU 格式输出）
│   │   ├── gidProPost.h           # GiD 前后处理器（新版）
│   │   ├── GidPrePost2.h/.cpp     # GiD 前后处理器（旧版，支持旧格式文件）
│   │   ├── InpDataStructtures.h   # Abaqus INP 数据结构定义
│   │   ├── inpReader.h/.cpp       # Abaqus INP 文件解析器
│   │   ├── TextReader.h/.cpp      # 通用文本文件读取器
│   │   └── vtkCellType.h          # VTK 单元类型枚举
│   ├── include/CDFEG/             # 公共头文件（安装用）
│   ├── sample/                    # 示例程序
│   │   ├── truss1D/               # 一维桁架
│   │   ├── truss2D/               # 二维桁架
│   │   ├── truss3D/               # 三维桁架
│   │   ├── El2D/                  # 二维弹性（T3+Q4）
│   │   └── ElT3/                  # 二维弹性（T3三角形单元）
│   └── third/                     # 第三方依赖（Eigen 3.4.0）
├── pyTool/                        # Python 代码生成工具
│   ├── DataProject.py             # 项目数据模型
│   ├── DataField.py               # 物理场数据模型
│   ├── DataEleSub.py              # 单元数据模型（直接继承 ElementBase）
│   ├── DataEleSubG.py             # 等参单元数据模型（继承 IsoEleBase，含高斯积分）
│   ├── DataSch.py                 # 求解方案数据模型
│   ├── vtkCellType.py             # VTK 单元类型枚举
│   ├── MakerBase.py               # 生成器基类（Jinja2 渲染）
│   ├── MakerCpp.py                # C++ 项目代码生成器
│   ├── MakerGidFile.py            # GiD 前处理文件生成器
│   ├── template/                  # Jinja2 模板目录（.j2 文件）
│   └── test/                      # 测试/示例脚本
└── AGENTS.md                      # 本文件
```

## 构建命令

### FEMproject（C++ 核心库 + 示例）

```bash
# Configure（Windows）
cd FEMproject
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# 或使用 build.bat（仅 configure）
.\build.bat
```

构建输出目录：`FEMproject/build/output/`

### pyTool（Python 工具）

```bash
# 运行代码生成测试
cd pyTool
python test/test2DTruss.py
python test/test1DTruss.py
python test/testEl2D.py
```

pyTool 无额外依赖安装步骤，仅依赖 Python 标准库 + Jinja2。

## 核心架构

### C++ 三层继承体系

```
FEMData（有限元空间数据：节点/单元/材料）
  └── PhyFieldData（物理场：自由度/边界条件/总刚组装/求解）
        └── ElementBase（单元基类：run()/uEle()）
              └── IsoEleBase（等参单元：形函数/雅各比矩阵）
```

### 每个示例的标准代码模式

1. **单元类**（继承 `ElementBase` 或 `IsoEleBase`）→ 实现 `run()` 计算单刚、`uEle()` 后处理
2. **物理场类**（继承 `PhyFieldData`）→ 注册单元类型，可覆盖 `eProgram()` 自定义组装
3. **数据类**（继承 `FemData`）→ 设置 `_dim`，创建物理场，实现 `caculate()`
4. **main 函数** → 创建数据对象 → 添加节点/单元/材料/边界条件 → `caculate()`

### pyTool 代码生成流程

```
Python 测试脚本 → DataProject + DataField + DataEleSub
  → MakerCpp.makeAll() → Jinja2 模板渲染
    → 完整 C++ 项目（main.cpp、单元类、物理场类、数据类、CMakeLists.txt）
```

## 编码约定

### C++

- **类名**：PascalCase（`FEMData`, `PhyFieldData`, `ElementBase`）
- **成员变量**：下划线前缀 + camelCase（`_nPts`, `_femData`, `_eleNodes`）
- **函数**：camelCase（`addNode`, `setFirstBoundry`）
- **宏/常量**：全大写（`DOF_ID`, `H`, `H2`）
- **缩进**：Tab
- **C++ 标准**：C++14
- **所有文件**头部有 GPL-3.0 SPDX 许可证声明
- **虚函数**：基类用 `virtual`，派生类用 `override`
- **矩阵存储**：一维 `vector<double>` 行优先
- **命名空间**：`CDFEG`

### Python

- **类名**：PascalCase
- **方法名**：camelCase
- **序列化**：`toDict()` / `fromDict()` 模式
- **所有文件**头部有 GPL-3.0 许可证声明

## 第三方依赖

| 依赖 | 版本 | 位置 | 用途 |
|------|------|------|------|
| Eigen | 3.4.0 | `FEMproject/third/Eigen/` | 稀疏矩阵、线性代数、SimplicialLDLT 求解器 |
| Jinja2 | — | pyTool 运行时依赖 | C++ 代码模板渲染 |

## 关键技术要点

- **雅各比矩阵**：`IsoEleBase` 使用有限差分法（步长 H=0.02）计算形函数导数，支持 1 阶和 2 阶（Hessian）
- **稀疏矩阵**：自定义 CSR 格式存储 + Eigen `RowMajor` 稀疏矩阵
- **边界条件**：第一类用划行划列法（保存 `_data0`/`_f0` 原始数据），第二类叠加右端项
- **单元注册**：通过 `_types` set 字符串匹配，`addEle()` 时自动分配到对应单元类
- **DLL 导出**：Windows 下通过 `CDFEG_EXPORTS` 宏控制 `__declspec(dllexport/dllimport)`

## 注意事项

- 修改核心库（`FEMproject/CDFEG/` 下的 `.cpp/.h`）后需重新构建 DLL
- 新增示例程序需在顶层 `FEMproject/CMakeLists.txt` 添加 `add_subdirectory()`
- pyTool 生成的代码中，`runCode`、`uCode`、`initCode` 是用户嵌入的 C++ 代码片段

- `gidProPost.h`（小写开头）是新版 GiD 处理器，`GidPrePost2` 是旧版，支持旧格式文件
