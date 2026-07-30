# 创刀有限元程序生成系统

创刀有限元程序生成系统（CDFEG = Chuang-Dao Finite Element Program Generator）是一款专业有限元程序生成系统。它由 C++ 有限元核心库（DLL）与 Python 代码生成工具链组成。有限元核心库实现了有限元程序所需的各类通用类与方法，仅依赖 Eigen 3.4 一个第三方库；程序生成工具基于 Jinja2 模板。希望此库可以像**创刀**一样随用随创，在使用中不断扩展出新的能力。

## 项目结构

```
CDFEG/
├── FEMproject/                 # C++ 有限元工程
│   ├── CDFEG/                  # 有限元核心库 (DLL)
│   │   ├── ElementBase.h       # 单元基类
│   │   ├── IsoEleBase.h        # 等参元基类
│   │   ├── PhyFieldData.h      # 物理场数据基类
│   │   ├── DomainData.h        # 有限元域数据基类
│   │   ├── EquationSystem.h    # 方程组（稀疏矩阵求解）
│   │   ├── MatrixFun.h         # 矩阵运算工具
│   │   ├── gidPrePost.h        # GiD 前后处理接口
│   │   ├── inpReader.h         # Abaqus INP 读取
│   │   └── vtkPost.h           # VTK 后处理
│   ├── sample/                 # 示例集（7 个独立 CMake 子项目）
│   │   ├── truss1D/2D/3D/      #   桁架静力（1D/2D/3D）
│   │   ├── El2D/               #   平面应变静力
│   │   ├── ElT3/               #   弹性力学（T3）
│   │   ├── DEl2D/              #   Newmark-β 动力学
│   │   └── Hel2D/              #   热弹耦合（温度场 + 位移场）
│   └── third/Eigen/            # Eigen 3.4.0（header-only）
├── pyTool/                     # Python 代码生成工具（Jinja2 模板）
├── test/                       # 回归测试框架（pytest + SQLite 计时）
├── DevTool/                    # 开发辅助工具
└── docs/                       # 文档
```

## 核心库 CDFEG

CDFEG 是有限元基础库，采用面向对象**三层架构**，支持单元类型扩展与多物理场分析。数据按三层组织：

- **DomainData**（域数据）：存储网格、材料、物理场集合与流程控制，包含一个或多个 PhyFieldData
- **PhyFieldData**（物理场）：自由度编号、边界条件、方程组装/求解与后处理，包含一个或多个单元
- **ElementBase**（单元）：`run()` 计算单刚/质量/阻尼/载荷，`uEle()` 后处理

### 类层次结构

```
ElementBase (单元基类)
    └── IsoEleBase (等参元基类)
PhyFieldData (物理场数据)
DomainData (域数据)
EquationSystem (方程组求解)
```

### 核心文件说明

| 文件 | 功能 |
|------|------|
| `ElementBase.h` | 单元基类，定义 `run()` 计算单元刚度/质量/阻尼/载荷、`uEle()` 后处理 |
| `IsoEleBase.h` | 等参元基类，形函数(`shapeFun`)、坐标变换、雅可比(`dcoor`) |
| `PhyFieldData.h` | 物理场：自由度编号、边界条件、`eProgram_el()` 装配总刚+右端、`solve()`、`uPhy()` |
| `DomainData.h` | 域数据：节点(`addNode`)、单元(`addEle`)、材料本构(`_mateConstitutive`)、多场数据传递(`getCoef`) |
| `EquationSystem.h` | 基于 Eigen 稀疏矩阵的方程组：总刚组装(`adda`)、一/二类边界、LDLT 求解 |
| `MatrixFun.h` | 矩阵工具：行列式、逆、乘法、转置、方向余弦 |
| `gidPrePost.h` | GiD 前后处理：读 `.dat`、写 `.post.res`/`.post.msh` |
| `inpReader.h` | Abaqus INP 文件读取 |

### 材料本构机制

材料参数走**本构类型表**（不再使用 `_paramNames`）：

- 单元构造里声明 `_mateTypeName`（本构类型名）与 `_types`（单元类型集合）
- DomainData 用 `_mateConstitutive[本构名] = {参数名...}` 注册本构参数
- `run()` 内 `matParams.at("参数名")` 按名取值（前处理按本构名从 dat 材料段回填）

### 多物理场数据传递（`_coefNames`）

耦合问题中，某物理场的单元需要读取其他场的结果（如热弹耦合：位移场读温度场）：

- 物理场构造里声明依赖：`_coefNames[他场序号] = {"变量名"}`（例如 DelDisp 场 `_coefNames[0]={"T"}`）
- 基类 `eProgram_el()` 按声明自动调用 `getCoef(nodeIds, _coefNames)` 取他场数据，以 `场名::变量名`（如 `Heat::T`）为键传入单元 `run()` 的 `coef`
- 派生物理场**无需重写 `eProgram`** 手动装配 coef——线性椭圆场直接复用基类即可

### 开发示例：实现一维桁架单元

#### 1. 定义单元类（继承 ElementBase）

```cpp
// Truss1D.h
#include "CDFEG/ElementBase.h"
class Truss1D : public CDFEG::ElementBase {
public:
    Truss1D(CDFEG::PhyFieldData* pData);
    CDFEG::EleSubResult& run(
        const std::vector<double>& r,                                    // 节点坐标
        const std::map<std::string, std::vector<double>>& coef,          // 他场数据（单场问题可空）
        const std::map<std::string, double>& matParams                   // 材料参数（按本构表回填）
    ) override;
    CDFEG::uResult uEle(...) override;                                   // 后处理
};
```

#### 2. 实现单元计算（材料走本构表）

```cpp
// Truss1D.cpp
Truss1D::Truss1D(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(2, pData) {        // 2 = 节点数
    _name = "Truss1D";
    _dispNames = {"u"};
    _mateTypeName = "Truss";                // 本构类型名
    _types.insert("Truss1D");
}

CDFEG::EleSubResult& Truss1D::run(...) {
    double E = matParams.at("E");           // 由本构表 "Truss" → {E, A} 回填
    double A = matParams.at("A");
    double L = std::abs(r[0] - r[1]);
    double X = E * A / L;
    _result.estif = {X, -X, -X, X};         // 单元刚度矩阵
    _result.eload = {0, 0};                 // 单元载荷向量
    return _result;
}
```

#### 3. 组装域数据并求解

```cpp
class Truss1DData : public CDFEG::DomainData {
public:
    Truss1DData() {
        _dim = 1;
        auto* f = new CDFEG::PhyFieldData(1, this);   // 1 = 每节点 1 自由度
        f->_name = "Truss1DDisp";
        f->_dispNames = {"u"};
        f->addEleSub(new Truss1D(f));                 // 注册单元
        _phyDatas.push_back(f);
        _mateConstitutive["Truss"] = {"E", "A"};      // 注册本构参数名
    }
    int caculate() {
        auto* f = _phyDatas[0];
        f->initMatrix();      // 方程编号 + 稀疏骨架
        f->eProgram_el();     // 装配总刚 + 右端项（含边界）
        f->solve();           // LDLT 求解
        f->uPhy();            // 后处理回填
        return 1;
    }
};
```

### 关键接口说明

| 接口 | 作用 |
|------|------|
| `ElementBase::run()` | 单元计算，返回单刚/质量/阻尼/载荷 |
| `ElementBase::uEle()` | 后处理（应力、应变等） |
| `PhyFieldData::eProgram_el()` | 线性椭圆问题装配总刚+右端（含 eload、边界、`_coefNames` 多场数据） |
| `PhyFieldData::solve()` | LDLT 求解 |
| `PhyFieldData::uPhy()` | 回填节点结果 + 单元后处理 |
| `DomainData::getCoef()` | 多场数据传递（按 `_coefNames` 取他场结果） |
| `DomainData::addNode()/addEle()` | 建立网格 |

## 回归测试

`test/` 是基于 pytest 的回归测试框架，端到端验证各示例（构建 → 运行 → 对比基准，判据 `max|Δ|`），并含 Catch2 单元测试：

```bash
python test/run_tests.py                            # 跑 e2e + unit（默认）
python test/run_tests.py --suite e2e --case hel2d1  # 单个用例
python test/run_tests.py --timing-list e2e.hel2d1   # 查某用例历史耗时
```

框架自动把每个用例的耗时（不含编译）写入 SQLite（`test/timing.db`），并与同用例上次 `pass` 对比做**性能回归检测**（阈值 5%，仅告警 `⚠`、不影响退出码）。

## 编译

### CMake（MinGW）

```bash
cmake -B build -G "MinGW Makefiles" \
      -DCMAKE_MAKE_PROGRAM=C:/dev/mingw64/bin/mingw32-make.exe
cmake --build build --target <示例名> -j
```

可执行程序与全部 DLL（含第三方运行时）统一输出到 `build/out/Release` 或 `build/out/Debug`。源文件 UTF-8 编码，MSVC 需 `/utf-8`。

## Python 代码生成工具

```python
from pyTool.MakerBase import MakerBase
from pyTool.DataEleSub import DataEleSub

# 创建单元数据
ele = DataEleSub("Truss2D", nNode=2)
ele.runCode = "..."
ele.paramNames = ["E", "A"]

# 使用模板生成代码
maker = MakerBase()
maker.write2File("element.cpp.j2", "Truss2D.cpp", ele.toDict())
```

---

想获得商业授权或更多帮助，请联系作者。

# 联系方式

<img src="picture/weixin.png" style="zoom: 33%;" />
