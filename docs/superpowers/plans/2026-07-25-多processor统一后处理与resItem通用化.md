# 多 Processor 统一后处理 + ResItem 通用化 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把后处理统一到 `DomainData::post(it)` 遍历 `_processors`，将 `GidResItem` 通用化为 `Processor` 基类的 `ResItem`，并让 `vtkPost` 支持多时间步 PVD 输出且纳入回归。

**Architecture:** `Processor` 构造时已自动登记进 `DomainData::_processors`（观察指针，data 析构不 delete）。新增 `DomainData::post(it)` 统一遍历调用。`ResItem`（原 `GidResItem`）+ `ResLocation`/`ResType`（原 GiD 专有枚举）提升为 `Processor` 基类通用结果项，`_resItems` 移到基类。`vtkPost` 每步写一个 `.vtu` 并全量重写 `.pvd`，按 `_resItems` 输出。

**Tech Stack:** C++14 / CMake / Eigen 3.4（核心库 DLL）；Python 3 + Jinja2（pyTool）；Python + pytest（test 回归框架，GiD res + 文本解析器）。

## Global Constraints

- C++14；MSVC `/utf-8`（根 CMake 已配）；类成员变量下划线前缀 `_`；头文件 `#ifndef/#define/#endif` 防御。
- 命名空间 `CDFEG::`；DLL 导出宏 `CDFEG_API`。
- 编码：源文件 UTF-8（含中文注释）。
- 构建：mingw64 工具链（`C:/dev/mingw64`），回归经 `python test/run_tests.py`（config.toml 已配 `dll_dirs=["C:/dev/mingw64/bin"]`，自动处理 PATH）。
- 改 C++ 后必须 clean 重建验证（避免陈旧 obj 掩盖符号改名问题）。
- `GidPrePost2` 保持现状（不动）。
- git 提交注释中文；提交时默认含人工与 AI 全部更改。
- 文档（md）中文命名/中文内容。

**权威 spec：** `docs/superpowers/specs/2026-07-25-多processor统一后处理与resItem通用化-design.md`

---

## 文件结构（改动总览）

**核心库** `FEMproject/CDFEG/`：
- 重命名：`GidResItem.h/.cpp` → `ResItem.h/.cpp`（CMakeLists 同步）
- 改 `ResItem.h/.cpp`：类 `ResItem`、`ResLocation`、`ResType{Scalar,Vector,Matrix}`、辅助函数 `resLocationToStr/resTypeToStr/strToResLocation/strToResType`（删 `gidResultTypeComponents`）
- 改 `Processor.h`：加 `#include "ResItem.h"` 与 `public: std::vector<ResItem> _resItems;`
- 改 `DomainData.h/.cpp`：加 `void post(int it = 0);`
- 改 `gidPrePost.h/.cpp`：删 `_resItems` 成员与 `#include "GidResItem.h"`（改 include `ResItem.h`）；内部枚举名/函数名替换
- 改 `vtkPost.h/.cpp`：`setFilePath`、多步 PVD、按 `_resItems` 输出、高精度
- 改 `CMakeLists.txt`：`GidResItem.cpp/.h` → `ResItem.cpp/.h`

**示例** `FEMproject/sample/`：
- `DEl2D/{del2dData.h,del2dData.cpp,main.cpp}`、`El2D/main.cpp`、`Hel2D/main.cpp`、`ElT3/main.cpp`、`truss1D/main.cpp`、`truss2D/main.cpp`

**pyTool** `pyTool/`：`template/mainGid.cpp.j2`、`DataProject.py`、`test/testElT3.py`

**回归** `test/`：`framework/parser.py`、`framework/case.py`、`framework/update.py`、`config.toml`、新增 `framework/tests/test_vtu_parser.py`、新增 `models/del2d1_vtk.gid/del2d.dat`

**文档**：`FEMproject/CDFEG/代码解释.md`、`升级说明.md`、各 sample 说明、`.claude/rules/pyTool能力边界.md`、`.claude/skills/*/SKILL.md`

---

## Task 1: ResItem 通用化（改名 + 枚举精简 + _resItems 提升到 Processor）

> 纯重构：类/枚举/函数改名 + `_resItems` 归属上移。不改行为。所有引用旧名的源码必须本任务内一次性同步（C++ 改名原子，中间态编译不过）。

**Files:**
- Create: `FEMproject/CDFEG/ResItem.h`、`FEMproject/CDFEG/ResItem.cpp`
- Delete: `FEMproject/CDFEG/GidResItem.h`、`FEMproject/CDFEG/GidResItem.cpp`
- Modify: `FEMproject/CDFEG/CMakeLists.txt:11,29`、`FEMproject/CDFEG/Processor.h`、`FEMproject/CDFEG/gidPrePost.h`、`FEMproject/CDFEG/gidPrePost.cpp`、`FEMproject/sample/{El2D,Hel2D,ElT3,DEl2D}/main.cpp`、`pyTool/template/mainGid.cpp.j2`、`pyTool/DataProject.py`、`pyTool/test/testElT3.py`

**Interfaces:**
- Produces: `CDFEG::ResItem`（构造 `ResItem(const std::string& name, ResType type, ResLocation loc = ResLocation::OnNodes)`、`void addVal(int iField, const std::string& valName)`、字段 `_name/_type/_loc/_iFields/_ValNames`）、`enum class ResLocation{OnNodes,OnGaussPoints}`、`enum class ResType{Scalar,Vector,Matrix}`、`resLocationToStr/resTypeToStr/strToResLocation/strToResType`、`Processor::_resItems`。

- [ ] **Step 1: 新建 `ResItem.h`（从 GidResItem.h 改名 + 枚举精简）**

创建 `FEMproject/CDFEG/ResItem.h`：

```cpp
// SPDX-License-Identifier: GPL-3.0
// （与原 GidResItem.h 相同的 GPL 头，此处省略以省篇幅——实际写入完整 GPL 头）
#ifndef RES_ITEM_H
#define RES_ITEM_H
#include "CDFEG.h"
#include <string>
#include <vector>
namespace CDFEG {

    enum class ResLocation {
        OnNodes,
        OnGaussPoints
    };

    enum class ResType {
        Scalar,
        Vector,
        Matrix
    };

    CDFEG_API std::string resLocationToStr(ResLocation loc);
    CDFEG_API ResLocation strToResLocation(const std::string& str);
    CDFEG_API std::string resTypeToStr(ResType type);
    CDFEG_API ResType strToResType(const std::string& str);

    class CDFEG_API ResItem
    {
    public:
        ResItem(const std::string& name, ResType type, ResLocation loc = ResLocation::OnNodes);
        ~ResItem();
        void addVal(int iField, const std::string& valName);

        std::string _name;
        ResType _type;
        // 结果位置：OnNodes=节点结果(取_nodeRes)，OnGaussPoints=单元结果(取_elemRes)
        ResLocation _loc = ResLocation::OnNodes;
        std::vector<int> _iFields;
        std::vector<std::string> _ValNames;
    };
}
#endif
```

> GPL 头照搬原 `GidResItem.h:1-15`，仅把 `GID_RES_ITEM_H` 改 `RES_ITEM_H`。

- [ ] **Step 2: 新建 `ResItem.cpp`（从 GidResItem.cpp 改名 + 函数精简）**

创建 `FEMproject/CDFEG/ResItem.cpp`，内容基于原 `GidResItem.cpp`，做如下替换：

```cpp
// SPDX 头（照搬），#include "ResItem.h"
namespace CDFEG {
    std::string resLocationToStr(ResLocation loc) {
        switch (loc) {
            case ResLocation::OnNodes: return "OnNodes";
            case ResLocation::OnGaussPoints: return "OnGaussPoints";
        }
        return "OnNodes";
    }
    ResLocation strToResLocation(const std::string& str) {
        std::string lower; lower.reserve(str.size());
        for (char c : str) lower.push_back((char)std::tolower((unsigned char)c));
        if (lower == "ongausspoints") return ResLocation::OnGaussPoints;
        return ResLocation::OnNodes;
    }
    std::string resTypeToStr(ResType type) {
        switch (type) {
            case ResType::Scalar: return "Scalar";
            case ResType::Vector: return "Vector";
            case ResType::Matrix: return "Matrix";
        }
        return "Scalar";
    }
    ResType strToResType(const std::string& str) {
        std::string lower; lower.reserve(str.size());
        for (char c : str) lower.push_back((char)std::tolower((unsigned char)c));
        if (lower == "vector") return ResType::Vector;
        if (lower == "matrix") return ResType::Matrix;
        return ResType::Scalar;
    }

    ResItem::ResItem(const std::string& name, ResType type, ResLocation loc)
        : _name(name), _type(type), _loc(loc) {}
    ResItem::~ResItem() {}
    void ResItem::addVal(int iField, const std::string& valName) {
        _iFields.push_back(iField);
        _ValNames.push_back(valName);
    }
}
```

> 需 `#include <cctype>` 用 `std::tolower`。删除原 `gidResultTypeComponents`（不再需要，分量数用 `_ValNames.size()`）。

- [ ] **Step 3: 删除旧 `GidResItem.h` 与 `GidResItem.cpp`**

```bash
rm "FEMproject/CDFEG/GidResItem.h" "FEMproject/CDFEG/GidResItem.cpp"
```

- [ ] **Step 4: 更新 `CMakeLists.txt`**

`FEMproject/CDFEG/CMakeLists.txt`：
- 第 11 行 `GidResItem.cpp` → `ResItem.cpp`
- 第 29 行 `GidResItem.h` → `ResItem.h`

- [ ] **Step 5: `Processor.h` 增加 `_resItems` 成员**

`FEMproject/CDFEG/Processor.h`：在 `#include "CDFEG.h"` 之后加 `#include "ResItem.h"`（`<vector>` 经 ResItem.h 传递，亦可显式加 `#include <vector>`）。在 `public:` 区 `_nPts` 之前加：

```cpp
        // 本 processor 要输出的结果项（由 main 注册，post 内消费）
        std::vector<ResItem> _resItems;
```

- [ ] **Step 6: `gidPrePost.h` 去 `_resItems`、换 include**

`FEMproject/CDFEG/gidPrePost.h`：
- 第 19 行 `#include "GidResItem.h"` → `#include "ResItem.h"`
- 删除第 70 行 `std::vector<GidResItem> _resItems;`（已提升到 Processor 基类，继承可用）

- [ ] **Step 7: `gidPrePost.cpp` 替换枚举名与函数名**

`FEMproject/CDFEG/gidPrePost.cpp` 全文做以下**字面替换**（用 `replace_all` 或等价 sed，注意仅作用于标识符）：

| 旧 | 新 |
|---|---|
| `GidResItem` | `ResItem` |
| `GidLocation::OnGaussPoints` | `ResLocation::OnGaussPoints` |
| `GidLocation::OnNodes` | `ResLocation::OnNodes` |
| `GidResultType::Vector` | `ResType::Vector` |
| `GidResultType::Matrix` | `ResType::Matrix` |
| `GidResultType::Scalar` | `ResType::Scalar` |
| `gidResultTypeToStr(` | `resTypeToStr(` |

> `gidPrePost.cpp` 中 `outFile << gidResultTypeToStr(item._type)` 等调用点全部跟随替换。`OnGaussPoints` 分支（`gidPrePost.cpp:455`）与 `OnNodes` 分支均涉及。

- [ ] **Step 8: 示例 main.cpp 替换类名/枚举名（仅改名，调用方式不变）**

对 `FEMproject/sample/El2D/main.cpp`、`Hel2D/main.cpp`、`ElT3/main.cpp`、`DEl2D/main.cpp` 各自做字面替换：

| 旧 | 新 |
|---|---|
| `GidResItem` | `ResItem` |
| `GidResultType::Vector` | `ResType::Vector` |
| `GidResultType::Matrix` | `ResType::Matrix` |
| `GidResultType::Scalar` | `ResType::Scalar` |
| `GidLocation::OnGaussPoints` | `ResLocation::OnGaussPoints` |

> `gidPrePost._resItems.push_back(...)` 不变（继承自基类的成员，仍可访问）。

- [ ] **Step 9: pyTool 模板与脚本同步**

`pyTool/template/mainGid.cpp.j2` 第 22、26 行：

```jinja
    CDFEG::ResItem resItem{{ itemIdx }}("{{ item.name }}", CDFEG::ResType::{{ item.type }}{% if item.location == "OnGaussPoints" %}, CDFEG::ResLocation::OnGaussPoints{% endif %});
```

`pyTool/DataProject.py:47,51`：注释中 `GidResItem` → `ResItem`、`GidResultType` → `ResType`（docstring 文字）。

`pyTool/test/testElT3.py:37`：注释 `GidResItem` → `ResItem`。

- [ ] **Step 10: 全量 clean 重建 + e2e 回归验证（行为不变）**

因涉及源文件改名，必须清空 build 目录避免陈旧 obj：

```bash
rm -rf test/build
python test/run_tests.py --suite e2e --rebuild
```

Expected: 全部 e2e case（del2d1/hel2d1/el2d1/elt3_1/elt3_4x4/truss1D1/truss2D1/truss3D1/el2d_bf1/el2_mfel1/el2_mfel_noedge1/del2d_mini1）PASS，max|Δ| 不回归（≤1e-12）。

- [ ] **Step 11: 生成器测试验证**

```bash
python test/run_tests.py --suite generator
```

Expected: PASS（生成产物含 `ResItem`，与手写示例一致）。

- [ ] **Step 12: 提交**

```bash
git add -A
git commit -m "refactor: GidResItem→ResItem 通用化并提升到 Processor 基类

- GidResItem.h/.cpp 重命名为 ResItem.h/.cpp
- GidLocation→ResLocation, GidResultType(17值)→ResType{Scalar,Vector,Matrix}
- _resItems 从 GidPrePost 提升到 Processor 基类
- 同步全部示例 main、pyTool 模板与脚本"
```

---

## Task 2: DomainData::post 统一入口 + 示例迁移到 data.post(0)

> 新增统一入口，把分散的 `processor.post()` 调用改为 `data.post(0)`；del2dData 删除自持 `_prePost`。仍不改 vtkPost 输出依据（truss 仍由旧 writeVTK 路径输出，下任务再改）。

**Files:**
- Modify: `FEMproject/CDFEG/DomainData.h`、`FEMproject/CDFEG/DomainData.cpp`、`FEMproject/sample/DEl2D/del2dData.h`、`FEMproject/sample/DEl2D/del2dData.cpp`、`FEMproject/sample/DEl2D/main.cpp`、`FEMproject/sample/{El2D,Hel2D,ElT3}/main.cpp`、`FEMproject/sample/truss1D/main.cpp`、`FEMproject/sample/truss2D/main.cpp`

**Interfaces:**
- Consumes: `Processor::_resItems`（Task 1）、`Processor::post(int)`
- Produces: `DomainData::post(int it = 0)`（void，遍历 `_processors` 调 `post(it)`）

- [ ] **Step 1: `DomainData.h` 声明 `post`**

在 `DomainData.h` 的 `virtual int main() { return -1; };`（第 132 行）之后加：

```cpp
		// 统一后处理：遍历 _processors 调各自 post(it)
		void post(int it = 0);
```

- [ ] **Step 2: `DomainData.cpp` 实现 `post`**

在 `DomainData.cpp` 顶部 `#include` 区确认含 `#include "Processor.h"`（若无需补；`Processor.h` 提供 `post` 虚函数定义）。在 `DomainData::~DomainData()` 之后加：

```cpp
void CDFEG::DomainData::post(int it) {
	for (Processor* p : _processors) {
		p->post(it);
	}
}
```

- [ ] **Step 3: `del2dData.h` 删 `_prePost` 与 `setPost`**

`FEMproject/sample/DEl2D/del2dData.h`：
- 删除第 22-24 行 `namespace CDFEG { class GidPrePost; }` 前向声明块；
- 删除第 35-36 行 `void setPost(CDFEG::GidPrePost* prePost);` 与其上注释；
- 删除第 37-38 行 `private:` 与 `CDFEG::GidPrePost* _prePost = nullptr;`。

改后类体只剩：

```cpp
class del2dData : public CDFEG::DomainData {
public:
    del2dData();
    ~del2dData();
    virtual int caculate() override;
    virtual int main() override;
};
```

- [ ] **Step 4: `del2dData.cpp` 删 `setPost`，`caculate` 改调 `post(it)`**

- 删除第 19 行 `#include "CDFEG/gidPrePost.h"`（不再需要 GidPrePost 完整类型；del2dData.h 已去前向声明）；
- 删除第 35-38 行 `void del2dData::setPost(...) { _prePost = prePost; }`；
- 第 71-75 行 `if (_prePost) { ... _prePost->post(it); }` 替换为：

```cpp
        post(it);   // 遍历 _processors（GidPrePost 等）输出本步结果
```

- [ ] **Step 5: `DEl2D/main.cpp` 删 `setPost`**

删除第 62-63 行注释与 `data.setPost(&gidPrePost);`（保留 `gidPrePost` 构造、`pre()`、ResItem 注册、`data.caculate()`）。此时 `data.caculate()` 内部已通过 `post(it)` 驱动 gidPrePost。

- [ ] **Step 6: 静力示例 main 改 `data.post(0)`**

`FEMproject/sample/El2D/main.cpp` 第 27 行 `gidPrePost.post();` → `data.post(0);`。
`FEMproject/sample/Hel2D/main.cpp` 第 36 行 `gidPrePost.post();` → `data.post(0);`。
`FEMproject/sample/ElT3/main.cpp`：定位 `gidPrePost.post();` → `data.post(0);`（行号以实际为准）。

- [ ] **Step 7: truss main 改 `data.post(0)`**

`FEMproject/sample/truss1D/main.cpp`：删除第 61-62 行 `CDFEG::vtkPost vtkpost(&data); vtkpost.post();`，在 `data.caculate();`（第 60 行）之后加 `data.post(0);`。

`FEMproject/sample/truss2D/main.cpp`：删除第 67-68 行 `CDFEG::vtkPost vtkpost(&data); vtkpost.post();`，在 `data.caculate();`（第 60 行）之后加 `data.post(0);`。

> 同时删除两个 truss main 顶部 `#include "CDFEG/vtkPost.h"`（不再直接用 vtkPost 类型；`data.post(0)` 经 DomainData 接口，无需该头）。truss 的 `*.txt` 手工输出保留不动。

- [ ] **Step 8: 重建 + e2e 回归验证**

```bash
python test/run_tests.py --suite e2e --rebuild
```

Expected: 全部 e2e PASS。truss 的 vtk 输出此时仍由 vtkPost 旧 `writeVTK` 路径产生（truss 未注册 `_resItems`，但旧路径遍历 `_dispNames`，故 vtk 仍有内容；truss 回归比的是 `.txt`，不受影响）。

- [ ] **Step 9: 提交**

```bash
git add -A
git commit -m "refactor: 新增 DomainData::post 统一入口，示例迁移到 data.post(0)

- DomainData::post(it) 遍历 _processors 调各自 post
- del2dData 删除自持 _prePost/setPost，caculate 改调 post(it)
- El2D/Hel2D/ElT3/truss main 统一改用 data.post(0)"
```

---

## Task 3: vtkPost 多步 PVD 输出 + 按 _resItems 输出 + truss 补注册

> vtkPost 改为多步 PCD 时间序列，输出依据从「遍历 _dispNames/_eleResNames」改为「遍历 _resItems」。truss main 补注册 ResItem，否则 vtk 输出为空。

**Files:**
- Modify: `FEMproject/CDFEG/vtkPost.h`、`FEMproject/CDFEG/vtkPost.cpp`、`FEMproject/sample/truss1D/main.cpp`、`FEMproject/sample/truss2D/main.cpp`

**Interfaces:**
- Consumes: `Processor::_resItems`（Task 1）、`ResItem{_name,_type,_loc,_iFields,_ValNames}`、`ResLocation::OnNodes/OnGaussPoints`、`PhyFieldData::_nodeRes/_elemRes`、`DomainData::_dt`
- Produces: `vtkPost::setFilePath(parentPath, baseName)`、`vtkPost::post(it)` 写 `<base>_<it:04d>.vtu` + 重写 `<base>.pvd`

- [ ] **Step 1: 重写 `vtkPost.h`**

```cpp
// SPDX 头（照搬原 vtkPost.h:1-15）
#ifndef VTKPOST_H
#define VTKPOST_H
#include "Processor.h"
#include <string>
#include <vector>
#include <utility>

namespace CDFEG {
    class CDFEG_API vtkPost :
        public Processor
    {
        public:
        vtkPost(DomainData* data);
        ~vtkPost();

        // 与 GidPrePost 一致：输出 parentPath/baseName_<it>.vtu + baseName.pvd
        void setFilePath(const std::string& parentPath, const std::string& baseName);
        virtual int post(int it = 0);

    private:
        int writeVTU(const std::string& fn);
        int writePVD(const std::string& fn);
        std::string _outPath = ".";
        std::string _baseName = "result";
        // 已写步：(it, time=it*_dt)，供 pvd 汇总；每次 post 全量重写 pvd
        std::vector<std::pair<int, double>> _steps;
    };
}
#endif
```

- [ ] **Step 2: 重写 `vtkPost.cpp`**

```cpp
// SPDX 头（照搬原 vtkPost.cpp:1-15）
#include "vtkPost.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include "PhyFieldData.h"
#include "DomainData.h"
#include "ResItem.h"
namespace CDFEG {
    vtkPost::vtkPost(DomainData* data):Processor(data) {}
    vtkPost::~vtkPost() {}

    void vtkPost::setFilePath(const std::string& parentPath, const std::string& baseName) {
        _outPath = parentPath;
        _baseName = baseName;
    }

    int vtkPost::post(int it) {
        std::ostringstream oss;
        oss << _outPath << "/" << _baseName << "_" << std::setw(4) << std::setfill('0') << it << ".vtu";
        std::string vtuFn = oss.str();
        if (writeVTU(vtuFn) != 0) return -1;
        double time = it * _femData->_dt;
        _steps.push_back({it, time});
        std::string pvdFn = _outPath + "/" + _baseName + ".pvd";
        return writePVD(pvdFn);
    }

    int vtkPost::writeVTU(const std::string& fn) {
        std::ofstream ofs(fn);
        if (!ofs.is_open()) return -1;
        ofs << std::setprecision(15) << std::scientific;   // 高精度，支撑紧回归容差
        int dim = _femData->_dim;
        int nPt = _femData->_nPts;
        int nEle = _femData->_nElem;

        ofs << "<?xml version=\"1.0\"?>" << std::endl;
        ofs << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        ofs << "  <UnstructuredGrid>" << std::endl;
        ofs << "    <Piece NumberOfPoints=\"" << nPt << "\" NumberOfCells=\"" << nEle << "\">" << std::endl;

        // 节点坐标
        ofs << "      <Points>" << std::endl;
        ofs << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">" << std::endl;
        ofs << "          ";
        for (int iPt = 0; iPt < nPt; iPt++) {
            for (int iDim = 0; iDim < dim; iDim++) ofs << _femData->_nodes[dim * iPt + iDim] << " ";
            for (int iDim = dim; iDim < 3; iDim++) ofs << 0.0 << " ";
        }
        ofs << std::endl << "        </DataArray>" << std::endl << "      </Points>" << std::endl;

        // 单元连接
        ofs << "      <Cells>" << std::endl;
        ofs << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << std::endl << "          ";
        for (size_t i = 0; i < _femData->_eleNodes.size(); i++) ofs << _femData->_eleNodes[i] << " ";
        ofs << std::endl << "        </DataArray>" << std::endl;
        ofs << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << std::endl << "          ";
        for (int iEle = 0; iEle < nEle; iEle++) ofs << _femData->_elePt[iEle + 1] << " ";
        ofs << std::endl << "        </DataArray>" << std::endl;
        ofs << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">" << std::endl << "          ";
        for (int iEle = 0; iEle < nEle; iEle++) ofs << static_cast<int>(_femData->_eleTypes[iEle]) << " ";
        ofs << std::endl << "        </DataArray>" << std::endl << "      </Cells>" << std::endl;

        // 节点结果（OnNodes）与单元结果（OnGaussPoints），均按 _resItems 输出
        ofs << "      <PointData>" << std::endl;
        for (ResItem& item : _resItems) {
            if (item._loc != ResLocation::OnNodes || item._iFields.empty()) continue;
            PhyFieldData* phy = _femData->_phyDatas[item._iFields[0]];
            ofs << "        <DataArray type=\"Float64\" Name=\"" << item._name
                << "\" NumberOfComponents=\"" << item._ValNames.size() << "\" format=\"ascii\">" << std::endl;
            ofs << "          ";
            for (int iPt = 0; iPt < nPt; iPt++) {
                for (const std::string& vn : item._ValNames) {
                    auto& col = phy->_nodeRes[vn];
                    ofs << (iPt < (int)col.size() ? col[iPt] : 0.0) << " ";
                }
            }
            ofs << std::endl << "        </DataArray>" << std::endl;
        }
        ofs << "      </PointData>" << std::endl;
        ofs << "      <CellData>" << std::endl;
        for (ResItem& item : _resItems) {
            if (item._loc != ResLocation::OnGaussPoints || item._iFields.empty()) continue;
            PhyFieldData* phy = _femData->_phyDatas[item._iFields[0]];
            ofs << "        <DataArray type=\"Float64\" Name=\"" << item._name
                << "\" NumberOfComponents=\"" << item._ValNames.size() << "\" format=\"ascii\">" << std::endl;
            ofs << "          ";
            for (int iEle = 0; iEle < nEle; iEle++) {
                for (const std::string& vn : item._ValNames) {
                    auto& col = phy->_elemRes[vn];
                    ofs << (iEle < (int)col.size() ? col[iEle] : 0.0) << " ";
                }
            }
            ofs << std::endl << "        </DataArray>" << std::endl;
        }
        ofs << "      </CellData>" << std::endl;

        ofs << "    </Piece>" << std::endl << "  </UnstructuredGrid>" << std::endl << "</VTKFile>" << std::endl;
        return 0;
    }

    int vtkPost::writePVD(const std::string& fn) {
        std::ofstream ofs(fn);
        if (!ofs.is_open()) return -1;
        ofs << "<?xml version=\"1.0\"?>" << std::endl;
        ofs << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        ofs << "  <Collection>" << std::endl;
        for (auto& s : _steps) {
            std::ostringstream vf;
            vf << _baseName << "_" << std::setw(4) << std::setfill('0') << s.first << ".vtu";
            ofs << "    <DataSet timestep=\"" << s.second << "\" part=\"0\" file=\"" << vf.str() << "\"/>" << std::endl;
        }
        ofs << "  </Collection>" << std::endl << "</VTKFile>" << std::endl;
        return 0;
    }
}
```

> `_nodeRes`/`_elemRes` 为 `std::map<std::string, std::vector<double>>`（vtkPost 旧代码已用，确认存在）。`NumberOfComponents` 用 `_ValNames.size()`，与 GiD 侧语义一致。

- [ ] **Step 3: truss2D/main.cpp 补注册 ResItem**

在 `data.post(0);`（Step 7 of Task 2 已加）之前，`data.caculate();` 之后，插入（在 `Truss2DDispFieldData* phy = ...` 之前或之后均可，只要在 `post(0)` 前）：

```cpp
    // 为 vtkPost 注册输出项（按 _resItems 输出；GidPrePost 未挂载，仅 vtkPost 在 _processors 中）
    CDFEG::Processor* proc = data._processors.empty() ? nullptr : data._processors.front();
    if (proc) {
        CDFEG::ResItem disp("disp", CDFEG::ResType::Vector);
        disp.addVal(0, "u"); disp.addVal(0, "v");
        proc->_resItems.push_back(disp);
        CDFEG::ResItem force("force", CDFEG::ResType::Scalar, CDFEG::ResLocation::OnGaussPoints);
        force.addVal(0, "T");
        proc->_resItems.push_back(force);
        CDFEG::ResItem stress("stress", CDFEG::ResType::Scalar, CDFEG::ResLocation::OnGaussPoints);
        stress.addVal(0, "sigma");
        proc->_resItems.push_back(stress);
    }
```

> truss2D 仅 vtkPost 一个 processor（truss 不构造 GidPrePost），`_processors.front()` 即 vtkPost。

- [ ] **Step 4: truss1D/main.cpp 补注册 ResItem**

同 Step 3，但 disp 为 1D（仅 u，用 `Scalar`）：

```cpp
    CDFEG::Processor* proc = data._processors.empty() ? nullptr : data._processors.front();
    if (proc) {
        CDFEG::ResItem disp("disp", CDFEG::ResType::Scalar);
        disp.addVal(0, "u");
        proc->_resItems.push_back(disp);
        CDFEG::ResItem force("force", CDFEG::ResType::Scalar, CDFEG::ResLocation::OnGaussPoints);
        force.addVal(0, "T");
        proc->_resItems.push_back(force);
        CDFEG::ResItem stress("stress", CDFEG::ResType::Scalar, CDFEG::ResLocation::OnGaussPoints);
        stress.addVal(0, "sigma");
        proc->_resItems.push_back(stress);
    }
```

> truss1D 物理场 `_eleResNames` 含 `T`/`sigma`（见 truss1D main 现有 `*.txt` 输出），`_nodeRes` 含 `u`。

- [ ] **Step 5: 重建 + e2e 回归验证**

```bash
python test/run_tests.py --suite e2e --rebuild
```

Expected: 全部 e2e PASS（truss 回归比 `.txt`，vtk 改动不影响）。

- [ ] **Step 6: 手动核查 truss vtk 产物（可选，确认按 _resItems 输出非空）**

```bash
cd test/build/run/truss2D1 && cat Truss2D.txt  # 确认 txt 仍在
# vtkPost 默认 setFilePath 未调用，输出到当前目录 result_0000.vtu + result.pvd
ls result*.vtu result.pvd 2>/dev/null
```

> 注意：truss main 未调 `setFilePath`，vtkPost 用默认 `_outPath="."` `_baseName="result"`，输出 `result_0000.vtu` + `result.pvd` 到工作目录。打开 `result_0000.vtu` 应含 `disp`/`force`/`stress` 三个 DataArray。

- [ ] **Step 7: 提交**

```bash
git add -A
git commit -m "feat(vtkPost): 多时间步 PVD 输出并按 ResItem 输出

- vtkPost 新增 setFilePath，每步写 base_<it>.vtu + 全量重写 base.pvd
- 输出依据由 _dispNames/_eleResNames 改为遍历 _resItems
- 数值精度 setprecision(15) scientific，支撑紧回归容差
- truss1D/2D main 补注册 disp/force/stress ResItem"
```

---

## Task 4: DEl2D 新增 vtkPost（多 processor 演示 + 动力学 PVD）

**Files:**
- Modify: `FEMproject/sample/DEl2D/main.cpp`

**Interfaces:**
- Consumes: `vtkPost::setFilePath`、`Processor::_resItems`、`ResItem`（Task 1/3）
- Produces: DEl2D 运行同时产出 `del2d.post.res` + `del2d_<it>.vtu` 群 + `del2d.pvd`

- [ ] **Step 1: `DEl2D/main.cpp` 新增 vtkPost + lambda 双注册**

第 20 行 `#include "CDFEG/gidPrePost.h"` 之后加：

```cpp
#include "CDFEG/vtkPost.h"
#include "CDFEG/Processor.h"
```

在 `gidPrePost.pre();`（第 33 行）之后、ResItem 注册之前，加 vtkPost 构造：

```cpp
    CDFEG::vtkPost vtkpost(&data);
    vtkpost.setFilePath(path, project);
```

把第 41-60 行的四段 `gidPrePost._resItems.push_back(...)` 重构为 lambda，对 gidPrePost 与 vtkpost 各注册一次：

```cpp
    // 注册结果项：位移、速度、加速度、应力（gid 与 vtk 两个 processor 各一份）
    auto registerItems = [](CDFEG::Processor& p) {
        CDFEG::ResItem dispItem("disp", CDFEG::ResType::Vector);
        dispItem.addVal(0, "u"); dispItem.addVal(0, "v");
        p._resItems.push_back(dispItem);

        CDFEG::ResItem velItem("velocity", CDFEG::ResType::Vector);
        velItem.addVal(0, "velU"); velItem.addVal(0, "velV");
        p._resItems.push_back(velItem);

        CDFEG::ResItem accItem("acceleration", CDFEG::ResType::Vector);
        accItem.addVal(0, "accU"); accItem.addVal(0, "accV");
        p._resItems.push_back(accItem);

        CDFEG::ResItem stressItem("stress", CDFEG::ResType::Matrix);  // 节点应力（默认 OnNodes，与原 main 一致）
        stressItem.addVal(0, "sigmaXX");
        stressItem.addVal(0, "sigmaYY");
        stressItem.addVal(0, "sigmaXY");
        p._resItems.push_back(stressItem);
    };
    registerItems(gidPrePost);
    registerItems(vtkpost);

    data.caculate();   // 内部 post(it) 同时驱动 GidPrePost(res) + vtkPost(vtu/pvd)
```

> 说明：DEl2D 应力为最小二乘外推到节点的节点应力（存于 `DelDispFieldData::_nodeRes["sigmaXX/YY/XY"]`），故 `stressItem` 用默认 `OnNodes`——与原 `del2d main.cpp` 的 `GidResItem("stress", Matrix)`（两参数，默认 OnNodes）一致，GidPrePost 与 vtkPost 均走节点分支。gid 与 vtk 两份 ResItem 由同一 lambda 注册，`_loc` 自然一致。

- [ ] **Step 2: 重建 + 跑 del2d1，核查多 processor 产出**

```bash
python test/run_tests.py --suite e2e --case del2d1 --rebuild
ls test/build/run/del2d1/
```

Expected: `del2d.post.res`（11 步）+ `del2d_0000.vtu` … `del2d_0010.vtu` + `del2d.pvd`。e2e PASS（res 回归不破）。

- [ ] **Step 3: 手动核查 PVD 时间序列（可选，ParaView 打开 del2d.pvd 看 11 步动画）**

由用户在 ParaView 中打开 `test/build/run/del2d1/del2d.pvd` 确认时间轴动画。

- [ ] **Step 4: 提交**

```bash
git add -A
git commit -m "feat(DEl2D): 新增 vtkPost 多时间步输出，演示多 processor 并存

- del2d main 挂载 GidPrePost + vtkPost，lambda 双注册 ResItem
- caculate 内 post(it) 同时产出 res + vtu 群 + pvd"
```

---

## Task 5: 回归测试扩展（VTU/PVD parser + del2d1_vtk case，TDD）

> 新增 `del2d1_vtk` e2e case 对比 vtkPost 的 PVD 时间序列。parser 用 TDD：先写解析单测，再实现。

**Files:**
- Modify: `test/framework/parser.py`、`test/framework/case.py`、`test/framework/update.py`、`test/config.toml`
- Create: `test/framework/tests/test_vtu_parser.py`、`test/models/del2d1_vtk.gid/del2d.dat`
- Test: `test/framework/tests/test_vtu_parser.py`（pytest）

**Interfaces:**
- Consumes: `ResBlock`（`test/framework/parser.py`）、`compare`（comparator.py）、E2ECase（case.py）
- Produces: `parse_vtu_file(path, step)`、`parse_pvd_file(path)`、`format="pvd"` 分支、`del2d1_vtk` case

- [ ] **Step 1: 写失败测试 `test/framework/tests/test_vtu_parser.py`**

```python
"""VTU/PVD 解析单测。"""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))  # 使 import framework 可用

from framework.parser import parse_vtu_file, parse_pvd_file


def _write_vtu(path, arrays):
    """arrays: list of (name, ncomp, location, values_flat)。location: 'PointData'/'CellData'。"""
    with open(path, "w", encoding="utf-8") as f:
        f.write('<?xml version="1.0"?>\n<VTKFile type="UnstructuredGrid" version="0.1">'
                '<UnstructuredGrid><Piece NumberOfPoints="2" NumberOfCells="1">\n')
        f.write('<Points><DataArray type="Float64" NumberOfComponents="3" format="ascii">\n0 0 0 1 0 0\n'
                '</DataArray></Points>\n')
        f.write('<Cells><DataArray type="Int32" Name="connectivity" format="ascii">0 1</DataArray>'
                '<DataArray type="Int32" Name="offsets" format="ascii">2</DataArray>'
                '<DataArray type="UInt8" Name="types" format="ascii">3</DataArray></Cells>\n')
        for name, ncomp, loc, vals in arrays:
            f.write(f'<{loc}><DataArray type="Float64" Name="{name}" '
                    f'NumberOfComponents="{ncomp}" format="ascii">\n')
            f.write(" ".join(str(v) for v in vals) + "\n")
            f.write(f'</DataArray></{loc}>\n')
        f.write('</Piece></UnstructuredGrid></VTKFile>\n')


def test_parse_vtu_point_data(tmp_path):
    p = tmp_path / "a.vtu"
    _write_vtu(p, [("disp", 2, "PointData", [1.0, 2.0, 3.0, 4.0])])  # 2 节点 × 2 分量
    blocks = parse_vtu_file(p, step=0)
    assert ("disp", 0) in blocks
    blk = blocks[("disp", 0)]
    assert blk.components == ["comp_0", "comp_1"]
    assert blk.values[1] == [3.0, 4.0]
    assert blk.location == "OnNodes"


def test_parse_vtu_cell_data(tmp_path):
    p = tmp_path / "a.vtu"
    _write_vtu(p, [("stress", 3, "CellData", [10.0, 20.0, 30.0])])  # 1 单元 × 3 分量
    blocks = parse_vtu_file(p, step=5)
    assert ("stress", 5) in blocks
    assert blocks[("stress", 5)].values[1] == [10.0, 20.0, 30.0]
    assert blocks[("stress", 5)].location == "OnCells"


def test_parse_pvd_collects_steps(tmp_path):
    for it in range(3):
        _write_vtu(tmp_path / f"s_{it:04d}.vtu",
                   [("disp", 1, "PointData", [float(it), float(it)])])
    pvd = tmp_path / "s.pvd"
    pvd.write_text(
        '<?xml version="1.0"?>\n<VTKFile type="Collection"><Collection>\n'
        '<DataSet timestep="0.0" part="0" file="s_0000.vtu"/>\n'
        '<DataSet timestep="0.1" part="0" file="s_0001.vtu"/>\n'
        '<DataSet timestep="0.2" part="0" file="s_0002.vtu"/>\n'
        '</Collection></VTKFile>\n', encoding="utf-8")
    blocks = parse_pvd_file(pvd)
    assert sorted(k[1] for k in blocks) == [0, 1, 2]   # 顺序索引作 step
    assert blocks[("disp", 2)].values[1] == [2.0]
```

- [ ] **Step 2: 跑测试确认失败**

```bash
python -m pytest test/framework/tests/test_vtu_parser.py -v
```

Expected: FAIL（`ImportError: cannot import name 'parse_vtu_file'`）。

- [ ] **Step 3: 实现 `parse_vtu_file` 与 `parse_pvd_file`（追加到 `test/framework/parser.py` 末尾）**

```python
import xml.etree.ElementTree as ET


def parse_vtu_file(path, step) -> dict[tuple[str, int], ResBlock]:
    """解析 .vtu -> {(name, step): ResBlock}。

    PointData -> location='OnNodes'（按节点序，entity_id 从 1 起）；
    CellData  -> location='OnCells'（按单元序）。
    分量名统一 comp_N（VTU 不存分量名，actual/baseline 同规则即可对齐）。
    """
    blocks: dict[tuple[str, int], ResBlock] = {}
    tree = ET.parse(path)
    root = tree.getroot()
    piece = root.find(".//Piece")
    npt = int(piece.get("NumberOfPoints", "0"))
    for loc_tag, loc_name in (("PointData", "OnNodes"), ("CellData", "OnCells")):
        container = piece.find(loc_tag)
        if container is None:
            continue
        for da in container.findall("DataArray"):
            name = da.get("Name", "")
            ncomp = max(1, int(da.get("NumberOfComponents", "1")))
            vals = [float(x) for x in (da.text or "").split()]
            blk = ResBlock(result_name=name, analysis="", step=step,
                           result_type="", location=loc_name)
            blk.components = [f"comp_{i}" for i in range(ncomp)]
            for eid in range(len(vals) // ncomp):
                blk.values[eid + 1] = vals[eid * ncomp:(eid + 1) * ncomp]
            blocks[(name, step)] = blk
    return blocks


def parse_pvd_file(path) -> dict[tuple[str, int], ResBlock]:
    """解析 .pvd，按 <DataSet> 顺序（索引即 step）逐个解析引用的 vtu，合并。"""
    blocks: dict[tuple[str, int], ResBlock] = {}
    base = Path(path).parent
    tree = ET.parse(path)
    for step, ds in enumerate(tree.findall(".//DataSet")):
        fn = ds.get("file", "")
        if not fn:
            continue
        vtu = base / fn
        if not vtu.exists():
            continue
        for k, blk in parse_vtu_file(vtu, step).items():
            blocks[k] = blk
    return blocks
```

> `ResBlock` 已在 `parser.py:18` 定义（`result_name/analysis/step/result_type/location/components/values`）。`Path` 已在 parser.py 导入。

- [ ] **Step 4: 跑测试确认通过**

```bash
python -m pytest test/framework/tests/test_vtu_parser.py -v
```

Expected: 3 PASS。

- [ ] **Step 5: `case.py` 的 `_parse` 加 `pvd` 分支 + `_prepare_work_dir` 排除 vtk 基线**

`test/framework/case.py`：
- 顶部加 `from framework.parser import parse_res_file, parse_pvd_file`（在原 `parse_res_file` 导入后追加 `parse_pvd_file`）；
- `_parse` 方法（第 65-70 行）改为：

```python
    def _parse(self, path):
        if self.format == "truss_txt":
            return parse_truss_txt(path)
        if self.format == "pvd":
            return parse_pvd_file(path)
        return parse_res_file(path)
```

- `_prepare_work_dir`（第 59-62 行）的拷贝排除条件追加 `.vtu`/`.pvd`：

```python
            if (f.is_file() and ".post.res" not in f.name
                    and not f.name.endswith(".bak") and f.name != self.baseline
                    and not f.name.endswith(".vtu") and not f.name.endswith(".pvd")):
                shutil.copy2(f, work_dir / f.name)
```

- [ ] **Step 6: `update.py` 支持 `pvd` format 多文件基线**

`test/framework/update.py`：
- 顶部导入追加 `parse_pvd_file`（与 `parse_res_file` 同行）；
- `_run_in_work_dir`（第 27-42 行）：返回值改为返回 `(main_path, all_outputs_dir)` 或在调用处处理。最小改动：让 `update_baseline` 在 `format=="pvd"` 时，确认后拷贝工作目录中 `<project>.pvd` + `<project>_*.vtu` 群到 `case_dir`。

将 `update_baseline`（第 45-92 行）中"强制人工确认"之后的拷贝段（第 84 行 `shutil.copy2(actual_path, baseline_path)`）替换为：

```python
    if c.get("format") == "pvd":
        # 多文件基线：拷贝 <project>.pvd + <project>_*.vtu 群，先清旧基线群
        import glob
        for old in glob.glob(str(case_dir / f"{c['project']}*.vtu")):
            Path(old).unlink()
        old_pvd = case_dir / c["baseline"]
        if old_pvd.exists():
            old_pvd.unlink()
        for vtu in glob.glob(str(Path(actual_path).parent / f"{c['project']}_*.vtu")):
            shutil.copy2(vtu, case_dir / Path(vtu).name)
        shutil.copy2(actual_path, baseline_path)
    else:
        shutil.copy2(actual_path, baseline_path)
```

> `actual_path` 为 `_run_in_work_dir` 返回的 `.pvd` 路径；其 parent（work_dir）含全部 vtu。`c["project"]` = "del2d"，基线文件名前缀 `del2d`。

- [ ] **Step 7: 新增 `del2d1_vtk` case 与 case_dir**

新建 `test/models/del2d1_vtk.gid/`，拷贝输入：

```bash
mkdir -p test/models/del2d1_vtk.gid
cp test/models/del2d1.gid/del2d.dat test/models/del2d1_vtk.gid/del2d.dat
```

`test/config.toml`：在 `del2d_mini1` case 块（第 123-131 行）之后、`[suite.unit]`（第 133 行）之前插入：

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

- [ ] **Step 8: 冻结 del2d1_vtk 基线**

```bash
python test/run_tests.py --suite e2e --update del2d1_vtk
```

按提示输入 `yes`。Expected: 在 `test/models/del2d1_vtk.gid/` 生成 `del2d.pvd` + `del2d_0000.vtu` … `del2d_0010.vtu`。diff 输出 `max|Δ|=0.000e+00`（首次无旧基准，新建）。

- [ ] **Step 9: 跑 del2d1_vtk 回归确认通过**

```bash
python test/run_tests.py --suite e2e --case del2d1_vtk
```

Expected: PASS，`max|Δ|` ≤ 1e-12。

- [ ] **Step 10: 跑全量回归确认无回归**

```bash
python test/run_tests.py --suite e2e
```

Expected: 全部 e2e PASS（含新增 del2d1_vtk）。

- [ ] **Step 11: 提交**

```bash
git add -A
git commit -m "test: VTU/PVD 回归解析与 del2d1_vtk 用例

- parser 新增 parse_vtu_file/parse_pvd_file，复用 comparator
- case.py 加 pvd format 分支，排除 .vtu/.pvd 基线文件
- update.py 支持 pcd format 多文件基线群
- config.toml 新增 del2d1_vtk 用例 + models/del2d1_vtk.gid"
```

---

## Task 6: 文档同步

**Files:**
- Modify: `FEMproject/CDFEG/代码解释.md`、`FEMproject/CDFEG/升级说明.md`、`FEMproject/sample/ElT3/单轴拉伸算例说明.md`、`.claude/rules/pyTool能力边界.md`、`.claude/skills/macs-to-cdfeg/SKILL.md`、`.claude/skills/add-regression-case/SKILL.md`

- [ ] **Step 1: 文档字面替换（`GidResItem`/`post2` → `ResItem`/`post`）**

对上述文档执行字面替换：

| 旧 | 新 |
|---|---|
| `GidResItem` | `ResItem` |
| `GidResultType` | `ResType` |
| `GidLocation` | `ResLocation` |
| `post2()` | `post()` |

> `代码解释.md` 4.6 节标题与正文、4.3 节 vtkPost 描述（补"多步 PVD 输出"一句）。`升级说明.md:131` 的 `vtkPost` 描述保留但确认与现状一致。

- [ ] **Step 2: 核查无残留旧名**

```bash
cd E:/myProject/CDFEG && grep -rn "GidResItem\|GidResultType\|GidLocation" FEMproject pyTool test docs .claude --include=*.cpp --include=*.h --include=*.py --include=*.j2 --include=*.md || echo "无残留"
```

Expected: 输出"无残留"（GidPrePost2 不含这些标识符，安全）。

- [ ] **Step 3: 提交**

```bash
git add -A
git commit -m "docs: 同步 ResItem 改名与 post 统一入口

- 代码解释/升级说明/sample 说明中 GidResItem→ResItem、post2→post
- vtkPost 描述更新为多步 PVD 输出"
```

---

## 完成判据

- 全量 `python test/run_tests.py`（e2e + unit）全绿，含新增 `del2d1_vtk`；
- `del2d1_vtk` 基线已冻结，`max|Δ|` ≤ 1e-12；
- `grep` 确认无 `GidResItem/GidResultType/GidLocation` 残留；
- DEl2D 运行产出 `del2d.post.res` + `del2d_<it>.vtu` 群 + `del2d.pvd`，ParaView 可播 11 步动画；
- `GidPrePost2` 未改动。
