# macs → CDFEG 迁移 skill 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 建立通用 pre/ges/gcn 解析器 + pyTool 生成入口 + SKILL 指南，把 macs 项目迁移到 CDFEG 框架，先以 el 项目跑通（含 getCoef 跨场改造支持 elb 取 ela 位移）。

**Architecture:** 三层产物——① 核心库 `PhyFieldData::getCoef` 改 virtual 逐场收集 `_nodeRes`（key=场名::变量名）；② pyTool 通用解析器（preParser 解析 pre/ges/gcn → DataProject）+ 参数化生成入口 testMacs.py（mode=new，落地 macs/<Proj>）；③ SKILL.md 三步流程指南。el 项目作为首次端到端验证：生成框架后参考旧 C 源码（a1eq4g2.c/beq4g2.c/eela.c/eelb.c）填充单元 run/uEle 与每场 eProgram，最终与 el.exe 结果对比。

**Tech Stack:** Python 3 + Jinja2（pyTool）；C++14 + CMake + Eigen（CDFEG 核心库）；GiD 前后处理；MSVC `/utf-8` 或 MinGW。

## Global Constraints

- 命名空间 `CDFEG::`；DLL 导出宏 `CDFEG_API`；C++ 源文件 UTF-8（含中文注释），MSVC 须 `/utf-8`。
- 变量/函数驼峰命名；git 提交注释中文；文档中文命名。
- `getCoef` 返回 `map<string, vector<double>>`，key 格式 `"场名::变量名"`（如 `ela::u`）；`_nodeRes` 长度不足时填 0.0 兜底。
- `PhyFieldData::eProgram_el` **保持不变**（基类标准椭圆流程，coef 空）；跨场取数由各场重写的 `eProgram` 自行调 `getCoef`。
- 生成落地 `macs/<Proj>`（项目根下，独立于 FEMproject），`mode='new'`；`testMacs.py` 用 `__file__` 推算项目根，避免误产生目录。
- 单刚按行主序填 `EleSubResult.estif`（`index = i*_nVar + j`）；单元 `_name` 须与 dat elem name / `mat_<name>` 完全一致。
- 重生成前备份手填的 `run`/`uEle`/`eProgram` 体（pyTool 会覆盖源码）。
- 旧 mfel 源码路径：`E:/mfelProject/RegTest/testData/macs/el/solution/src/`（只读参考）。

## File Structure

| 文件 | 职责 | 创建/修改 |
|---|---|---|
| `FEMproject/CDFEG/PhyFieldData.h` | `getCoef` 改 virtual 声明 | 修改 |
| `FEMproject/CDFEG/PhyFieldData.cpp` | `getCoef` 逐场 `_nodeRes` 实现 | 修改 |
| `FEMproject/testGetCoef/testGetCoef.cpp` | getCoef 跨场 C++ 测试 | 创建 |
| `FEMproject/testGetCoef/CMakeLists.txt` | 测试构建 | 创建 |
| `pyTool/preParser.py` | pre/ges/gcn 解析器 | 创建 |
| `pyTool/test/test_preParser.py` | 解析器单元测试（独立 assert 脚本） | 创建 |
| `pyTool/test/testMacs.py` | 参数化生成入口 | 创建 |
| `.claude/skills/macs-to-cdfeg/SKILL.md` | 三步流程指南 | 创建 |
| `macs/El/*` | el 迁移项目（生成 + 填充） | 生成后填充 |

**约定**：Task 1–6（工具链）用 TDD + 完整代码；Task 7–12（el 填充）给「旧 C 文件:行号 → C++ 目标 + 转换规则 + 验证」，旧 C 源码作为只读输入引用，不在计划中逐行重写（避免与 spec 第 6 节重复且冗长）。

---

## Task 1: getCoef 跨场改造（核心库）

**Files:**
- Modify: `FEMproject/CDFEG/PhyFieldData.h:46-47`
- Modify: `FEMproject/CDFEG/PhyFieldData.cpp:102-106`
- Create: `FEMproject/testGetCoef/testGetCoef.cpp`
- Create: `FEMproject/testGetCoef/CMakeLists.txt`

**Interfaces:**
- Produces: `virtual std::map<std::string,std::vector<double>> PhyFieldData::getCoef(const std::vector<int>& nodeIds)`，key=`<场名>::<变量名>`，跨场收集 `_nodeRes`，越界填 0。

- [ ] **Step 1: 写失败测试 `testGetCoef.cpp`**

```cpp
#include "CDFEG/PhyFieldData.h"
#include "CDFEG/FemData.h"
#include <iostream>
#include <vector>
#include <string>
int main() {
    using namespace CDFEG;
    FEMData* fd = new FEMData();
    fd->setNPts(4);                 // 4 节点
    fd->addNodeEnd();
    PhyFieldData* ela = new PhyFieldData(2, fd);   // dof=2
    ela->_name = "ela";
    ela->_nodeRes["u"] = {1.0, 2.0, 3.0, 4.0};
    ela->_nodeRes["v"] = {10.0, 20.0, 30.0, 40.0};
    PhyFieldData* elb = new PhyFieldData(3, fd);   // dof=3
    elb->_name = "elb";
    fd->_phyDatas.push_back(ela);
    fd->_phyDatas.push_back(elb);
    // elb 调 getCoef 取所有场（含 ela）的 _nodeRes
    auto coef = elb->getCoef({0, 1, 3});
    bool ok = true;
    ok &= (coef.count("ela::u") && coef["ela::u"] == std::vector<double>{1.0, 2.0, 4.0});
    ok &= (coef.count("ela::v") && coef["ela::v"] == std::vector<double>{10.0, 20.0, 40.0});
    ok &= (coef.count("elb::dxx") && coef["elb::dxx"] == std::vector<double>{0.0, 0.0, 0.0}); // elb 未填 _nodeRes → 兜底 0
    // 越界节点（nodeId=9 超出 4 节点）→ 兜底 0
    auto coef2 = elb->getCoef({0, 9});
    ok &= (coef2["ela::u"] == std::vector<double>{1.0, 0.0});
    std::cout << (ok ? "TEST PASS" : "TEST FAIL") << std::endl;
    return ok ? 0 : 1;
}
```

- [ ] **Step 2: 写 `testGetCoef/CMakeLists.txt`**

```cmake
add_executable(testGetCoef testGetCoef.cpp)
target_link_libraries(testGetCoef PRIVATE CDFEG)
```

- [ ] **Step 3: 运行测试确认失败**

Run: `cmake -B build -S FEMproject && cmake --build build --target testGetCoef && ./build/FEMproject/testGetCoef/testGetCoef`（路径按实际生成器调整）
Expected: 编译失败或运行输出 `TEST FAIL`（getCoef 当前返回空 map，`coef.count("ela::u")==0`）。

- [ ] **Step 4: 改 `PhyFieldData.h:46-47` 为 virtual**

```cpp
		virtual std::map<std::string, std::vector<double>>  getCoef1(std::vector<int> nodeIds);
		virtual std::map<std::string, std::vector<double>>  getCoef(const std::vector<int>& nodeIds);
```

- [ ] **Step 5: 改 `PhyFieldData.cpp:102-106` 实现逐场收集**

```cpp
	std::map<std::string, std::vector<double>> PhyFieldData::getCoef(const std::vector<int>& nodeIds)
	{
		std::map<std::string, std::vector<double>> coef;
		for (PhyFieldData* fd : _femData->_phyDatas)
		{
			for (auto& kv : fd->_nodeRes)
			{
				std::vector<double> v;
				v.reserve(nodeIds.size());
				for (int nid : nodeIds)
					v.push_back(nid < (int)kv.second.size() ? kv.second[nid] : 0.0);
				coef[fd->_name + "::" + kv.first] = std::move(v);
			}
		}
		return coef;
	}
```

- [ ] **Step 6: 构建并运行测试确认通过**

Run: `cmake --build build --target testGetCoef && ./build/FEMproject/testGetCoef/testGetCoef`
Expected: `TEST PASS`

- [ ] **Step 7: 提交**

```bash
git add FEMproject/CDFEG/PhyFieldData.h FEMproject/CDFEG/PhyFieldData.cpp FEMproject/testGetCoef
git commit -m "getCoef 改 virtual 逐场收集 _nodeRes，支持跨场取数（key=场名::变量名）"
```

---

## Task 2: preParser.pre 解析

**Files:**
- Create: `pyTool/preParser.py`
- Create: `pyTool/test/test_preParser.py`

**Interfaces:**
- Produces: `parsePre(projDir, projName) -> DataProject`（填充 dim/fields/dispNames/单元名+nNode+paramNames+paramValues，不含 ges 积分信息）。

- [ ] **Step 1: 写失败测试 `test_preParser.py`（基于 el.pre 真实内容）**

```python
# SPDX-License-Identifier: GPL-3.0
import os, sys, json
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/..")
from preParser import parsePre

MACS = r"E:/mfelProject/RegTest/testData/macs"
proj = parsePre(MACS, "el")
ok = True
ok &= (proj.dim == 2)
names = [f.name for f in proj.fields]
ok &= (set(names) == {"ela", "elb"})
ela = next(f for f in proj.fields if f.name == "ela")
ok &= (ela.dispNames == ["u", "v"])
ele_names = [e.name for e in ela.eleSubs]
ok &= ("a1eq4g2" in ele_names and "a2ll2" in ele_names)
a1 = next(e for e in ela.eleSubs if e.name == "a1eq4g2")
ok &= (a1.nNodes == 4)
ok &= (a1.paramNames == ["pe", "pv", "fu", "fv", "rou", "alpha"])
ok &= (a1.paramValues == [1.0e10, 0.3, 0.0, 0.0, 3000.0, 0.6])
elb = next(f for f in proj.fields if f.name == "elb")
ok &= (elb.dispNames == ["dxx", "dyy", "dxy"])
print("parsePre:", "PASS" if ok else "FAIL")
sys.exit(0 if ok else 1)
```

- [ ] **Step 2: 运行确认失败**

Run: `cd pyTool && python test/test_preParser.py`
Expected: `FAIL`（`ModuleNotFoundError: preParser`）。

- [ ] **Step 3: 写 `preParser.py` 的 pre 解析**

```python
# SPDX-License-Identifier: GPL-3.0
# 解析 macs 项目的 pre/ges/gcn，构造 pyTool DataProject
import os
from DataProject import DataProject
from DataField import DataField
from DataEleSubG import DataEleSubG

def parsePre(projDir, projName):
    """解析 <projName>.pre → DataProject（dim/场/单元骨架/参数）。
    pre 格式：
      第1行: 2dxy 2 3 8 10   ← 首字段 2dxy/3dxyz 给 dim
      <场名> 0 <dof> <?> <dispNames...>
      # 分隔
      element y
      <场名>
      <单元名> <nNode> <paramNames...>
      #
      matedata
      <场名> <单元名>
       <paramValues...>
      #
    """
    path = os.path.join(projDir, projName, projName + ".pre")
    dim = 2
    project = DataProject(projName, dim)
    fields = {}          # name -> DataField
    field_order = []
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f.readlines()]
    # 第1行取 dim
    if lines:
        head = lines[0].split()
        if head and head[0].startswith("2"):
            dim = 2
        elif head and head[0].startswith("3"):
            dim = 3
        project.dim = dim
    # 状态机解析
    mode = "field"
    cur_field = None
    cur_ele_name = None
    i = 0
    while i < len(lines):
        ln = lines[i].strip()
        i += 1
        if not ln:
            continue
        toks = ln.split()
        if ln == "#":
            mode = "field"
            cur_field = None
            cur_ele_name = None
            continue
        if toks[0] == "element":
            mode = "element"
            continue
        if toks[0] == "matedata":
            mode = "matedata"
            continue
        if mode == "field":
            # <场名> 0 <dof> <?> <dispNames...>，场名首字母非数字
            fname = toks[0]
            dispNames = toks[4:]
            fld = DataField(fname)
            fld.project = project
            fields[fname] = fld
            field_order.append(fname)
            # 暂存 dispNames（单元 addEleSub 后会从单元汇总，这里先存）
            fld._preDispNames = dispNames
            cur_field = fld
            project.addField(fld)
            continue
        if mode == "element":
            if cur_field is None or toks[0] in fields:
                # 该行是场名（切换本组场）
                if toks[0] in fields:
                    cur_field = fields[toks[0]]
                continue
            # <单元名> <nNode> <paramNames...>
            eleName = toks[0]
            nNode = int(toks[1])
            paramNames = toks[2:]
            ele = DataEleSubG(eleName, nNode)
            ele.paramNames = paramNames
            ele.dispNames = list(cur_field._preDispNames)
            cur_field.addEleSub(ele)
            continue
        if mode == "matedata":
            # <场名> <单元名>，下一非空行是数值
            if len(toks) >= 2 and toks[0] in fields and toks[1]:
                fname, eleName = toks[0], toks[1]
                # 读下一行数值
                vals = []
                while i < len(lines):
                    nl = lines[i].strip()
                    i += 1
                    if nl == "" or nl == "#":
                        break
                    try:
                        vals = [float(x) for x in nl.split()]
                        break
                    except ValueError:
                        break
                fld = fields[fname]
                for e in fld.eleSubs:
                    if e.name == eleName:
                        e.paramValues = vals
                        break
            continue
    return project
```

> 注：`DataField` 无 `_preDispNames` 字段，运行时动态挂属性（Python 允许）；ges 解析后再回填 `ele.shapeFuns` 等。

- [ ] **Step 4: 运行测试确认通过**

Run: `cd pyTool && python test/test_preParser.py`
Expected: `parsePre: PASS`

- [ ] **Step 5: 提交**

```bash
git add pyTool/preParser.py pyTool/test/test_preParser.py
git commit -m "preParser: 新增 pre 文件解析（dim/场/单元/参数）"
```

---

## Task 3: preParser.ges 解析

**Files:**
- Modify: `pyTool/preParser.py`（追加 `parseGes` + 在 `parsePre` 末尾调用）
- Modify: `pyTool/test/test_preParser.py`（追加 ges 断言）

**Interfaces:**
- Produces: `parseGes(projDir, eleName, ele) -> None`（填充 `gaussPoints/gaussWeights/shapeFuns/eleResNames/type/coordVars`，含 `coef` 段记录）。

- [ ] **Step 1: 追加 ges 断言到测试**

在 `test_preParser.py` 的 `print` 前追加：
```python
a1 = next(e for e in ela.eleSubs if e.name == "a1eq4g2")
ok &= (len(a1.gaussPoints) == 4)
ok &= (abs(a1.gaussPoints[0][0] - 0.5773502692) < 1e-9)
ok &= (a1.gaussWeights == [1.0, 1.0, 1.0, 1.0])
ok &= (len(a1.shapeFuns) == 4)
ok &= ("exx" in a1.eleResNames and "eyy" in a1.eleResNames and "exy" in a1.eleResNames)
ok &= (a1.type == 2)  # 4节点2D → 面单元
# 线单元 a2ll2
a2 = next(e for e in ela.eleSubs if e.name == "a2ll2")
ok &= (a2.type == 1 and a2.bBC == True)
```

- [ ] **Step 2: 运行确认失败**

Run: `cd pyTool && python test/test_preParser.py`
Expected: `FAIL`（`gaussPoints` 为空，DataEleSubG 默认）。

- [ ] **Step 3: 在 `preParser.py` 追加 parseGes + type 推断 + 在 parsePre 末尾遍历调用**

```python
def _inferType(nNode, dim, coordVars):
    """由节点数+维度推断单元几何类型：0点/1线/2面/3体。"""
    if dim == 1:
        return 1
    if dim == 2:
        return 2 if nNode >= 3 else 1     # ≥3节点为面，否则线
    if dim == 3:
        return 3 if nNode >= 5 else 2
    return 1

def parseGes(projDir, eleName, ele):
    """解析 <eleName>.ges → 填充 gaussPoints/Weights/shapeFuns/eleResNames/type。
    ges 关键段：
      node N          → 校验 nNodes
      refc rx,ry,     → coordVars（参考坐标名）
      coor x,y,       → 维度
      gaus = N        + 下 N 行: rx ry weight（2D）/ rx weight（1D）
      shap 段         → 形函数表达式（取 disp 的第一个分量的 N 个表达式）
      func = a,b,c    → eleResNames
      coef u,v        → 记录到 ele._gesCoefVars（run 填充时用 coef["场名::变量"]）
    """
    path = os.path.join(projDir, ele.project.name, eleName + ".ges")
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f.readlines()]
    gaus_n = 0
    gaus_lines = []
    shap_exprs = []
    in_shap = False
    ges_coef_vars = []
    for idx, raw in enumerate(lines):
        s = raw.strip()
        if s.startswith("gaus"):
            toks = s.replace("=", " ").split()
            gaus_n = int(toks[1])
            # 收集后续 gaus_n 个数值行
            cnt = 0
            j = idx + 1
            while j < len(lines) and cnt < gaus_n:
                lj = lines[j].strip()
                j += 1
                if lj == "" :
                    continue
                try:
                    vals = [float(x) for x in lj.split()]
                    gaus_lines.append(vals)
                    cnt += 1
                except ValueError:
                    break
            continue
        if s.startswith("shap"):
            in_shap = True
            continue
        if in_shap:
            # 形如 u1 = (...) 或 u=（分量标题）或空行或 tran/func 等结束
            if s == "" :
                continue
            if "=" in s and not s.endswith("="):
                # 取等号右侧表达式
                expr = s.split("=", 1)[1].strip()
                if expr:
                    shap_exprs.append(expr)
            # 遇到 tran/func/gaus/dist/mass/load/end 等关键字结束 shap 收集
            if any(s.startswith(k) for k in ("tran", "func", "dist", "mass", "load", "end", "coef", "refc", "coor", "node", "mate")):
                if not s.startswith(("u", "v", "d")):  # 形函数变量行以 u1/v1/dxx1 等开头，不视为结束
                    in_shap = False
            continue
        if s.startswith("func") and "=" in s:
            # func = exx,eyy,exy
            rhs = s.split("=", 1)[1]
            ele.eleResNames = [x.strip() for x in rhs.split(",") if x.strip()]
            continue
        if s.startswith("coef") and "=" in s:
            rhs = s.split("=", 1)[1]
            ges_coef_vars = [x.strip() for x in rhs.split(",") if x.strip()]
            continue
    # 填充积分点（每行前 dim 个为坐标，末个为权重）
    ele.gaussPoints = []
    ele.gaussWeights = []
    nrefc = len(gaus_lines[0]) - 1 if gaus_lines else 0
    for gl in gaus_lines:
        ele.gaussPoints.append(gl[:nrefc])
        ele.gaussWeights.append(gl[-1])
    # 形函数：rx→x[1], ry→x[2] 占位符（pyTool _replaceCoordVars 会替换）
    # 只取与节点数相等的前 nNode 个表达式
    def _conv(expr):
        return expr.replace("rx", "x[1]").replace("ry", "x[2]")
    ele.shapeFuns = [_conv(e) for e in shap_exprs[:ele.nNodes]]
    # type 推断
    if ges_coef_vars:
        ele.type = 1            # 含 coef 的边界/依赖单元多为线单元（beq4g2 是面，但暂按 ges 显式覆盖）
        ele._gesCoefVars = ges_coef_vars
    inferred = _inferType(ele.nNodes, ele.project.dim, ele.coordVars)
    if ele.type == 1 and ele.nNodes >= 3 and ele.project.dim == 2:
        ele.type = 2            # beq4g2（4节点面单元含 coef）修正回面
    elif not ges_coef_vars:
        ele.type = inferred
    if ele.type == 1:
        ele.bBC = True
    # coordVars
    if nrefc == 1:
        ele.coordVars = ["x"]
    elif nrefc == 2:
        ele.coordVars = ["x", "y"]
```

并在 `parsePre` 的 `return project` 前追加：
```python
    # 用 ges 填充积分/形函数/结果名
    for fld in project.fields:
        for e in fld.eleSubs:
            e.project = fld
            parseGes(projDir, e.name, e)
            e.inferVTKCellType()
    return project
```

- [ ] **Step 4: 运行测试确认通过**

Run: `cd pyTool && python test/test_preParser.py`
Expected: `parsePre: PASS`

- [ ] **Step 5: 提交**

```bash
git add pyTool/preParser.py pyTool/test/test_preParser.py
git commit -m "preParser: 新增 ges 解析（积分点/形函数/结果名/类型推断）"
```

---

## Task 4: preParser.gcn 解析

**Files:**
- Modify: `pyTool/preParser.py`（追加 `parseGcn`）
- Modify: `pyTool/test/test_preParser.py`（追加 gcn 断言）

**Interfaces:**
- Produces: `parseGcn(projDir, projName, project) -> None`（填 `project.caculateCode`：按场依赖顺序对每场渲染 `initMatrix/eProgram/solve/uPhy` 调用）。

- [ ] **Step 1: 追加 gcn 断言**

在 `test_preParser.py` 追加（gcn 解析在 parsePre 之后单独调用）：
```python
from preParser import parseGcn
parseGcn(MACS, "el", proj)
# caculateCode 应先出现 ela 场调用，再出现 elb（依赖 ela）
ok &= ("ela" in proj.caculateCode and "elb" in proj.caculateCode)
ok &= (proj.caculateCode.index("ela") < proj.caculateCode.index("elb"))
```

- [ ] **Step 2: 运行确认失败**

Run: `cd pyTool && python test/test_preParser.py`
Expected: `FAIL`（`parseGcn` 未定义或 caculateCode 为空）。

- [ ] **Step 3: 追加 parseGcn**

```python
def parseGcn(projDir, projName, project):
    """解析 <projName>.gcn → project.caculateCode。
    gcn 格式（el）：
      defi
      a ell              ← 场名 a（对应 _name），求解器 ell
      b str a            ← 场名 b，求解器 str，依赖 a
      START a
      SOLVc a            ← 标准椭圆
      SOLVstr b a        ← 最小二乘（b 依赖 a）
      gidres(coor0);
    渲染为按依赖顺序的各场调用（场名取 gcn 的 a/b，需映射到 project.fields 的 _name）。
    """
    path = os.path.join(projDir, projName, projName + ".gcn")
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n").strip() for ln in f.readlines()]
    # defi 段：场依赖（b str a → b 依赖 a）
    deps = {}            # field_key -> [dep_keys]
    keys_order = []
    in_defi = False
    for ln in lines:
        if ln == "defi":
            in_defi = True
            continue
        if in_defi:
            if ln == "" or ln.startswith("START") or ln.startswith("SOLV") or ln.startswith("gidres"):
                break
            toks = ln.split()
            if len(toks) >= 2:
                key = toks[0]
                keys_order.append(key)
                deps[key] = toks[2:]    # 依赖的场 key
    # 拓扑排序（简单：按声明顺序，依赖已在前的天然满足）
    ordered = [k for k in keys_order]
    # gcn 的 a/b 映射到 project.fields：按声明顺序对应
    field_list = project.fields
    key_to_field = {}
    for idx, k in enumerate(ordered):
        if idx < len(field_list):
            key_to_field[k] = field_list[idx]
    # 渲染 caculateCode：每场 initMatrix→eProgram→solve→uPhy
    lines_out = []
    for k in ordered:
        fld = key_to_field.get(k)
        if not fld:
            continue
        var = fld.fieldDataClassName[0].lower() + fld.fieldDataClassName[1:]  # 小驼峰实例名
        lines_out.append(f"        {var}->initMatrix();")
        lines_out.append(f"        {var}->eProgram();")
        lines_out.append(f"        {var}->solve();")
        lines_out.append(f"        {var}->uPhy();")
    project.caculateCode = "\n".join(lines_out) + "\n"
```

- [ ] **Step 4: 运行测试确认通过**

Run: `cd pyTool && python test/test_preParser.py`
Expected: `parsePre: PASS`

- [ ] **Step 5: 提交**

```bash
git add pyTool/preParser.py pyTool/test/test_preParser.py
git commit -m "preParser: 新增 gcn 解析（场依赖顺序 → caculateCode）"
```

---

## Task 5: testMacs.py 参数化生成入口

**Files:**
- Create: `pyTool/test/testMacs.py`

**Interfaces:**
- Consumes: `parsePre`, `parseGcn`（Task 2/4）；`MakerCpp`, `MakerGidFile`。
- Produces: 运行 `python test/testMacs.py el` 在项目根 `macs/El/` 生成完整解决方案。

- [ ] **Step 1: 写 `testMacs.py`**

```python
# SPDX-License-Identifier: GPL-3.0
# macs 项目通用生成入口：python test/testMacs.py <项目名> [macs根目录]
import sys, os, json
HERE = os.path.dirname(os.path.abspath(__file__))
PYTOOL = os.path.dirname(HERE)                 # pyTool/
PROJ_ROOT = os.path.dirname(PYTOOL)            # 项目根 CDFEG/
sys.path.append(PYTOOL)
from preParser import parsePre, parseGcn
from MakerCpp import MakerCpp
from MakerGidFile import MakerGidFile

def main():
    if len(sys.argv) < 2:
        print("用法: python test/testMacs.py <项目名> [macs根目录]")
        sys.exit(1)
    projName = sys.argv[1]
    macsRoot = sys.argv[2] if len(sys.argv) > 2 else r"E:/mfelProject/RegTest/testData/macs"
    project = parsePre(macsRoot, projName)
    parseGcn(macsRoot, projName, project)
    # 落地项目根 macs/<Proj>，new 模式（自带库副本）
    outPath = os.path.join(PROJ_ROOT, "macs", projName.capitalize())
    os.makedirs(outPath, exist_ok=True)
    maker = MakerCpp(project, outPath, mode="new", sln_cmake_path=os.path.join(outPath, "CMakeLists.txt"))
    maker.mainMode = 1     # GiD 入口
    maker.makeAll()
    MakerGidFile(project, outPath).makeAll()
    with open(os.path.join(outPath, "data.json"), "w", encoding="utf-8") as f:
        json.dump(project.toDict(), f, indent=4, ensure_ascii=False)
    print("生成完成:", outPath)

if __name__ == "__main__":
    main()
```

- [ ] **Step 2: 运行生成 el**

Run: `cd pyTool && python test/testMacs.py el`
Expected: 生成 `E:/myProject/CDFEG/macs/El/`，含 CMakeLists.txt、源码、CDFEG/ 库副本、third/、GiD 文件。

- [ ] **Step 3: 检查生成产物结构**

Run: `ls macs/El/` （在项目根）
Expected: 至少含 `CMakeLists.txt`、`elData*.{h,cpp}`、`elaFieldData*.{h,cpp}`、`elbFieldData*.{h,cpp}`、单元源码、`CDFEG/`、`el.gid/` 或 GiD 文件。
若类名/结构与预期不符，回查 `DataField.fieldDataClassName` / `DataProject` 命名规则并修正 `testMacs.py`。

- [ ] **Step 4: 提交**

```bash
git add pyTool/test/testMacs.py
git commit -m "新增 testMacs 参数化生成入口（pre/ges/gcn → macs/<Proj> new 模式）"
```

---

## Task 6: SKILL.md 三步流程指南

**Files:**
- Create: `.claude/skills/macs-to-cdfeg/SKILL.md`

- [ ] **Step 1: 写 SKILL.md**

```markdown
---
name: macs-to-cdfeg
description: Use when 把 mfel 旧项目（macs 测试集：el/heat/newmark 等）迁移到 CDFEG 框架，需解析 pre/ges/gcn 生成框架、参考旧 C 源码填充计算逻辑、比较 cnd。
---

# macs → CDFEG 迁移（macs-to-cdfeg）

## Overview
把 `E:/mfelProject/RegTest/testData/macs/<proj>` 下的旧 mfel（C+GiD）项目迁移到 CDFEG（C++ 三层架构）。三步：解析生成框架 → 填充逻辑 → 比较 cnd。

## When to Use
- 新迁移一个 macs 项目到 CDFEG
- 排查迁移后编译/结果与旧程序不一致

## 步骤

### 1. 解析生成框架
\`\`\`bash
cd pyTool && python test/testMacs.py <项目名>
\`\`\`
解析 `<proj>.pre`+`*.ges`+`<proj>.gcn` → 生成 `macs/<Proj>`（new 模式，自带库副本）。产物：FEMData/PhyFieldData/EleSub 派生类骨架 + CMake + GiD。

### 2. 填充计算逻辑（参考旧 C 源码）
**每个场的 `eProgram` 必须重写**（基类 `eProgram_el` 不处理 eload→右端，也不调 getCoef）。映射：
| 旧 C（solution/src） | CDFEG 目标 |
|---|---|
| `<proj>.c` main 流程 | `<Proj>Data::caculate`（gcn 解析已生成命令流） |
| `ee<field>a.c` | `<Field>FieldData::eProgram` 重写（组装单刚 + eload→_f） |
| `<ele>.c` | `<Ele>::run`/`uEle`（高斯积分+B矩阵+行主序单刚） |
| `gidpre/gidres/gidmsh.c` | 框架已实现（mainMode=1） |

**多场 coef**：依赖场（如 elb）的 `eProgram` 调 `getCoef(nodeIds)` 取位移，单元 `run` 用 `coef["<场名>::<变量>"]`（如 `coef["ela::u"]`）。时序由 caculate 命令流保证（先 ela 后 elb）。

### 3. 比较 cnd
核对生成 cnd 与原 `<proj>.cnd`：边界条件项（各场 -I/-D × volume/surface/line/point）+ 材料赋值项（mate_Num）。数量与 QUESTION/VALUE 结构一致即合格。

## Common Mistakes
| 现象 | 原因 | 修复 |
|---|---|---|
| 单刚全 0 | dat elem name 与单元 `_name` 不一致 | 改 dat 为 `mat_<_name>` |
| elb 位移全 0 | getCoef 时 ela 未求解 | caculate 顺序按 gcn 依赖 |
| 载荷未生效 | eProgram_el 不处理 eload | 重写 eProgram，手动 `_equSys._f += eload` |
| 重生成覆盖手填 | 重跑 testMacs | 先备份 run/uEle/eProgram 体 |

> 深度机制见 `.claude/rules/核心库实现细节.md`；pyTool 能力边界见 `.claude/rules/pyTool能力边界.md`。
```

- [ ] **Step 2: 提交**

```bash
git add .claude/skills/macs-to-cdfeg/SKILL.md
git commit -m "新增 macs-to-cdfeg skill 三步迁移指南"
```

---

## Task 7: el — A1eq4g2 单元填充（位移场面单元）

**Files:**
- Modify: `macs/El/<a1eq4g2 源码>.cpp/.h`（生成产物，文件名按 testMacs 实际输出）

**Interfaces:**
- Consumes: `IsoEleBase`；`EleSubResult.estif`（行主序）；`run(r, coef, matParams)`。
- Reference: `E:/mfelProject/RegTest/testData/macs/el/solution/src/a1eq4g2.c:41-159`（单刚/载荷）+ `a1eq4g2.ges`（形函数/stif/load 公式）。

- [ ] **Step 1: 填充 `run`（参考 a1eq4g2.c:68-159）**
  - 清零 `_result.estif/eload/emass`（`adda` 累加，必清零）。
  - 高斯积分循环（4 点）：`dcoor`→雅可比→`inverse`→`shapn` 得 B；`weight=_gaus[iGaus]*det`。
  - D 矩阵（平面应力）：`fact=pe/(1+pv)/(1-2pv)*vol`、`shear=(0.5-pv)`，按 `eexx/eeyy/eexy` 构建应变-应变率。
  - 单刚 `estif[(iv-1)*8+(jv-1)] += stif*weigh`（行主序，`a1eq4g2.c:140-145`）。
  - 载荷 `eload[iv] += cu[..]*fu*vol*weigh`（`a1eq4g2.c:149-158`）。

- [ ] **Step 2: 填充 `uEle`（参考 uela.c + 应力外推）**
  - 由 `coef`（ela 节点位移 u,v）求高斯点应变 `exx/eyy/exy` → 应力 `sigmaXX/YY/XY = D·e`。
  - 形函数加权外推到节点，返回 `eleResult`（单元平均）+ `nodeResult`（节点应力+weight）。

- [ ] **Step 3: 验证编译**

Run: `cmake -B macs/El/build -S macs/El && cmake --build macs/El/build`
Expected: A1eq4g2 编译通过（无错；此时 eProgram 尚未填，链接可能失败属正常，仅需本单元编译无误）。

- [ ] **Step 4: 提交**

```bash
git add macs/El
git commit -m "el: A1eq4g2 单元填充 run/uEle（参考 a1eq4g2.c）"
```

---

## Task 8: el — Beq4g2 单元填充（应力平滑场，跨场 coef）

**Files:**
- Modify: `macs/El/<beq4g2 源码>.cpp/.h`

**Interfaces:**
- Consumes: `coef["ela::u"]`、`coef["ela::v"]`（ela 位移，经 getCoef 传入）；`IsoEleBase`。
- Reference: `beq4g2.c:44-198` + `beq4g2.ges`（coef u,v / mass lump / load = 应力）。

- [ ] **Step 1: 填充 `run`（参考 beq4g2.c:101-198）**
  - 从 `coef["ela::u"]/["ela::v"]` 取节点位移 `u/v`，`ebeq4g2`（`dcoef`）求位移导数 → 应变 `exx/eyy/exy`（`beq4g2.c:126-132`）。
  - 本构（平面应变）：`fact=pe/(1+pv)/(1-2pv)`、`shear=(1-2pv)/2`，应力 `fxx/fyy/fxy`（`beq4g2.c:133-135`）。
  - **estif = lumped mass**（`stif=0`，`beq4g2.c:141`；mass `elump=vol`，`beq4g2.c:146-181` 填 `emass`）。
  - **eload = 应力载荷**（`eload[iv]+=cd*flux*vol*weigh`，`beq4g2.c:183-197`）。

- [ ] **Step 2: 验证编译**

Run: `cmake --build macs/El/build`
Expected: Beq4g2 编译通过。

- [ ] **Step 3: 提交**

```bash
git add macs/El
git commit -m "el: Beq4g2 单元填充（最小二乘应力平滑，coef 取 ela 位移）"
```

---

## Task 9: el — ElaFieldData::eProgram 重写

**Files:**
- Modify: `macs/El/elaFieldData.cpp`

**Interfaces:**
- Consumes: `eProgram_el` 流程（组装 estif）；额外需手动把 a2ll2（边界荷载）的 eload 累加到 `_equSys._f`。
- Reference: `eela.c:60-176`（遍历单元→a1eq4g2/a2gl2→组装→边界→右端）。

- [ ] **Step 1: 重写 `eProgram`（基于 eProgram_el 结构 + eload→_f 补丁）**
  - 复制 `PhyFieldData::eProgram_el` 主体（`PhyFieldData.cpp:39-93`）到 `ElaFieldData::eProgram`。
  - 在 `eleSub->run(...)` 后，把 `outData.eload` 按 `lm` 累加到 `_equSys._f[inv]`（`eela.c:134-136` 的 u/ef 对应逻辑 → `_f`）。
  - a2ll2 单刚通常为 0，仅 eload 贡献（面力→节点力）。
  - 末尾 `applyFirstBCs` + `applySecondBCs`。

- [ ] **Step 2: 验证编译**

Run: `cmake --build macs/El/build`
Expected: ElaFieldData 编译通过。

- [ ] **Step 3: 提交**

```bash
git add macs/El
git commit -m "el: ElaFieldData eProgram 重写（组装单刚 + a2ll2 eload→右端）"
```

---

## Task 10: el — ElbFieldData::eProgram 重写（跨场 + 最小二乘）

**Files:**
- Modify: `macs/El/elbFieldData.cpp`

**Interfaces:**
- Consumes: `getCoef(nodeIds)`（取 ela 位移）；beq4g2 产出 estif(lumped mass)+eload(应力)。
- Reference: `eelb.c:46-192`（最小二乘：emass + ew/=emass）。

- [ ] **Step 1: 重写 `eProgram`**
  - 仿 `eProgram_el` 结构遍历 beq4g2 单元。
  - **关键**：`coef = getCoef(nodeIds);` 取 ela 位移，传入 `eleSub->run(r, coef, matParams)`（beq4g2 用 `coef["ela::u"]`）。
  - 组装 beq4g2 的 estif（lumped mass）+ eload（应力）→ `_equSys`（estif 进 adda，eload 进 `_f`）。
  - 末尾施加边界。

- [ ] **Step 2: 重写 `uPhy`（回填应力 dxx/dyy/dxy）**
  - `_rhs` → `_nodeRes["dxx"]/["dyy"]/["dxy"]`（基类 uPhy 已回填 `_dispNames` 对应 dof；确认 dxx/dyy/dxy 为 elb 的 dispNames）。

- [ ] **Step 3: 验证编译**

Run: `cmake --build macs/El/build`
Expected: ElbFieldData 编译通过。

- [ ] **Step 4: 提交**

```bash
git add macs/El
git commit -m "el: ElbFieldData eProgram/uPhy 重写（getCoef 取 ela 位移，最小二乘应力平滑）"
```

---

## Task 11: el — caculate + main 完整构建运行

**Files:**
- Modify: `macs/El/elData.cpp`（caculate，gcn 已生成命令流，核对即可）
- Verify: `macs/El/main.cpp`（GiD 入口 mainMode=1）

- [ ] **Step 1: 核对 caculate 命令流**
  - `ElData::caculate` 应含 gcn 解析生成的「ela: init→eProg→solve→uPhy；elb: 同」序列（Task 4 产出）。
  - 确认 ela 在 elb 之前（保证 elb getCoef 时 ela `_nodeRes` 已填）。

- [ ] **Step 2: 准备 GiD 输入数据**
  - 把 `macs/el/el.bas/.cnd/.dis/.mat/.pos/.pre` 或 GiD 网格数据复制到 `macs/El/` 运行目录（按 `GidPrePost::setFilePath` 约定）。
  - 确认 dat elem name 与单元 `_name` 一致（a1eq4g2/beq4g2）。

- [ ] **Step 3: 完整构建**

Run: `cmake -B macs/El/build -S macs/El && cmake --build macs/El/build --config Debug`
Expected: 生成 `macs/El/build/El.exe`（或同名可执行），无错。

- [ ] **Step 4: 运行**

Run: `cd macs/El/build && ./El.exe`
Expected: 读取 GiD 网格 → 求解 → 输出位移/应力结果文件，无崩溃。

- [ ] **Step 5: 提交**

```bash
git add macs/El
git commit -m "el: caculate 命令流核对 + main 完整构建运行通过"
```

---

## Task 12: el 结果对比 + cnd 比较

**Files:**
- Verify: `macs/El` 输出 vs `macs/el/el.exe` 输出
- Compare: 生成 `macs/El/el.cnd` vs `E:/mfelProject/RegTest/testData/macs/el/el.cnd`

- [ ] **Step 1: 对比位移（ela 场 u/v）**
  - 运行旧 `macs/el/el.exe` 得参考结果。
  - 与新 `macs/El` 输出的节点位移逐节点对比，容差 1e-6。
  - Expected: 最大误差 < 1e-6。若超差，回查 A1eq4g2 单刚（Task 7）、eProgram（Task 9）。

- [ ] **Step 2: 对比应力（elb 场 dxx/dyy/dxy）**
  - 同上对比节点应力。
  - Expected: 误差 < 1e-6。若超差，回查 Beq4g2（Task 8）、getCoef 时序（Task 10/11）。

- [ ] **Step 3: cnd 比较**
  - 逐条核对生成 `macs/El/el.cnd` 与原 `el.cnd`：边界条件项（ela u-I/D、v-I/D；elb dxx/dyy/dxy-I/D，各 4 种 CONDMETHOD）+ 材料赋值项（Surface-a1eq4g2/Line-a2ll2/Surface-beq4g2 的 mate_Num）。
  - Expected: 条件数量与 QUESTION/VALUE 结构一致（编号/顺序差异可接受）。

- [ ] **Step 4: 提交验证记录**

```bash
git add macs/El
git commit -m "el: 结果与 el.exe 一致（位移/应力误差<1e-6），cnd 比对通过"
```

---

## Self-Review 记录

- **Spec 覆盖**：spec 第 2 节产物 → Task 1/2-5/6；第 4 节字段映射 → Task 2/3/4；第 5 节 getCoef → Task 1；第 6 节 el 迁移 → Task 7-11；第 8 节 cnd → Task 12；第 10 节验证 → Task 12。全覆盖。
- **类型一致性**：`getCoef` 签名 Task 1 定义，Task 10 调用一致（`coef["ela::u"]`）；`parsePre/parseGes/parseGcn` 签名 Task 2-4 定义，Task 5 调用一致。
- **占位符**：el 填充任务（7-11）用「旧 C 行号 + 转换规则 + 验证」，旧 C 源码是 spec 引用的既有只读输入，非 placeholder。
