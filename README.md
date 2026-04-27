# 桁架有限元编程课程项目

C++ 有限元程序框架与代码自动生成工具。

## 项目结构

```
课程/
├── FEMproject/          # C++ 有限元程序
│   ├── CDFEG/         # 有限元程序核心库 (DLL)
│   │   ├── EleSubBase.h        # 单元子程序基类
│   │   ├── IsoEleBase.h      # 等参元基类
│   │   ├── EquationSystem.h   # 方程组求解
│   │   ├── MatrixFun.h       # 常用的计算函数
│   │   ├── PhyFieldData.h   # 物理场数据基类
│   │   ├── FemData.h        # 有限元空间数据基类
│   │   └── gidProPost.h    # GID前后处理接口
│   ├── truss1D/       # 一维桁架示例
│   └── sample/        # 示例程序
├── pyTool/            # Python 代码生成工具
│   ├── MakerBase.py          # 代码生成器基类
│   ├── DataEleSub.py         # 单元数据结构
│   ├── vtkCellType.py        # VTK单元类型
│   └── template/            # Jinja2 模板
└── 课程大纲.md        # 教学大纲
```

## 核心库 CDFEG

CDFEG  是一个有限元程序基础库，采用面向对象设计，支持单元类型扩展与多物理场分析。

### 类层次结构

```
EleSubBase (单元子程序基类)
    └── IsoEleBase (等参元基类)
PhyFieldData (物理场数据)
FEMData (有限元空间数据)
EquationSystem (方程组求解)
```

### 核心文件说明

| 文件 | 功能 |
|------|------|
| `EleSubBase.h` | 单元子程序基类，定义 `run()` 接口计算单元刚度矩阵、质量矩阵、阻尼矩阵和外力向量；`uEle()` 用于后处理计算 |
| `IsoEleBase.h` | 等参单元基类，提供形函数计算(`shapeFun`)、坐标转换(`coordTransFun`)、雅各比矩阵计算(`dcoor`)等核心功能 |
| `EquationSystem.h` | 基于 Eigen 库的稀疏矩阵求解器，支持总刚组装、`addFirstBC()`/`addSecondBC()` 施加边界条件 |
| `MatrixFun.h` | 矩阵运算工具库：行列式、逆矩阵、矩阵乘法、转置、向量运算、方向余弦计算等 |
| `PhyFieldData.h` | 物理场数据管理，每个物理场包含若干单元类型；管理自由度编号与边界条件 |
| `FemData.h` | 有限元空间数据：节点坐标(`addNode`)、单元连接(`addEle`)、材料属性管理 |
| `gidProPost.h` | GID 前后处理接口，支持网格导入与结果可视化 |
| `InpDataStructures.h` | Abaqus INP 文件读取器 |

### 开发示例：实现一维桁架单元

基于核心库实现一个新的单元类型，典型工作流程如下：

#### 1. 定义单元类（继承 EleSubBase）

```cpp
// Truss1D.h
#include "CDFEG/EleSubBase.h"
class Truss1D : public CDFEG::EleSubBase {
public:
    Truss1D(CDFEG::PhyFieldData* pData);
    CDFEG::EleSubResult& run(
        const std::vector<double>& r,      // 节点坐标
        const std::map<std::string, std::vector<double>>& coef,  // 形函数系数
        const std::map<std::string, double>& matParams  // 材料参数
    ) override;
    CDFEG::uResult uEle(...) override;  // 后处理计算
};
```

#### 2. 实现单元计算逻辑

```cpp
// Truss1D.cpp
Truss1D::Truss1D(CDFEG::PhyFieldData* pData) 
    : CDFEG::EleSubBase(2, pData) {
    _name = "Truss1D";
    _dispNames = {"u"};
    _paramNames = {"E", "A"};
    _types.insert("Truss1D");
}

CDFEG::EleSubResult& Truss1D::run(...) {
    double E = matParams.at("E");
    double A = matParams.at("A");
    double L = abs(r[0] - r[1]);
    double X = E * A / L;
    _result.estif = {X, -X, -X, X};  // 单元刚度矩阵
    _result.eload = {0, 0};          // 单元外力向量
    _result.nodeIds = {...};
    return _result;
}
```

#### 3. 定义物理场（继承 PhyFieldData）

```cpp
// Truss1DDispFieldData.h
class Truss1DDispFieldData : public CDFEG::PhyFieldData {
public:
    Truss1DDispFieldData(CDFEG::FEMData* femData);
};
```

```cpp
// Truss1DDispFieldData.cpp
Truss1DDispFieldData::Truss1DDispFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(1, femData) {  // 1 = 每节点1个自由度
    _name = "Truss1DDisp";
    _dispNames = {"u"};
    _eleSubs.push_back(new Truss1D(this));  // 注册单元类型
    _eleResNames = {"T"};  // 输出结果名称：温度/应力
}
```

#### 4. 定义数据类与执行计算

```cpp
// 定义数据类
class Truss1DData : public CDFEG::FEMData {
public:
    Truss1DData() {
        _dim = 1;
        _phyDatas.push_back(new Truss1DDispFieldData(this));
    }
    int caculate() {
        auto* f = static_cast<Truss1DDispFieldData*>(_phyDatas[0]);
        f->initMatrix();      // 初始化矩阵结构
        f->eProgram_el();    // 组装总刚
        f->solve();          // 求解方程
        f->uPhy();           // 后处理
        f->_equSys.calRightVals();  // 计算节点力
        return 1;
    }
};
```

#### 5. 创建模型与求解

```cpp
int main() {
    Truss1DData data;
    // 添加节点
    data.addNode(1, 0.0);
    data.addNode(2, 0.6);
    data.addNodeEnd();
    // 添加单元
    data.addEle(1, {1, 2}, "Truss1D");
    // 添加材料
    data.addMate({{"E", 2e11}, {"A", 6e-4}});
    data.setEleMateId(1, 0);
    // 设置边界条件
    auto* phydata = static_cast<Truss1DDispFieldData*>(data._phyDatas[0]);
    phydata->setFirstBoundry(1, 0.0);   // 第一类边界: u=0
    phydata->setSecondBoundry(2, 15000); // 第二类边界: q=15000
    // 求解
    data.caculate();
    return 0;
}
```

### 关键接口说明

| 接口 | 作用 |
|------|------|
| `EleSubBase::run()` | 单元计算核心，返回单元刚度矩阵、阻尼矩阵、质量矩阵、外力向量 |
| `EleSubBase::uEle()` | 后处理计算，如应力、应变 |
| `PhyFieldData::eProgram_el()` | 组装总刚矩阵与右端项 |
| `PhyFieldData::solve()` | 求解方程组 |
| `FEMData::addNode()` | 添加节点 |
| `FEMData::addEle()` | 添加单元 |


## 编译

### CMake

（待完善）

### Visual Studio

打开 `FEMproject/CDFEG.sln`，或使用 `.vcxproj` 文件。

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