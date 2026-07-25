# CDFEG 物理场 uPhy 后处理逻辑统一：weight 加权节点外推的复用

> 摘要：CDFEG 物理场的 `uPhy` 负责"位移回填 + 应力恢复"。4 个应力场派生全部重写 `uPhy`，其中 weight 加权节点外推逻辑几乎一字不差地重复了 4 份，而基类对应的 `nodeResult` 处理分支竟是死代码。本次重构把外推逻辑提到基类 `extrapolateNodeResults`，4 派生去重，净减 286 行。

## 1. uPhy 在做什么

`PhyFieldData::uPhy` 是求解后的后处理环节：

1. 把方程解 `_rhs[id]` 回填到节点位移 `_nodeRes[disp]`；
2. 逐单元调 `uEle(r, coef, matParams)` 反求单元结果（应力等）；
3. 对常应变/常应力单元（如 CST 三角形、Q4），单元应力需外推到节点。

步骤 3 的"节点外推"常用**加权最小二乘**：以形函数值 N 为权，节点应力 = Σ(单元应力·N) / ΣN。单元子程序在 `uResult.nodeResult["weight"]` 返回权，在 `nodeResult["sigmaXX"]` 等返回加权和（已乘 N），`uPhy` 累加后相除即得节点应力。

## 2. 4 份重复的外推逻辑

grep 发现 4 个应力场派生**全部重写** `uPhy`，且都用 weight 模式：

| 派生 | 应力外推逻辑 | 位移回填 | 后处理 |
|---|---|---|---|
| `ElDispFieldData` (El2D) | weight（σxx/yy/xy） | 标准 | vonMises |
| `Elastic2DDispFieldData` (ElT3) | weight（Sxx/Syy/Sxy） | 标准 | — |
| `DelDispFieldData` (Hel2D) | weight（σxx/yy/xy） | 标准 | vonMises |
| `DelDispFieldData` (DEl2D) | weight（σxx/yy/xy） | Newmark（`_u`/`_v`/`_w`） | vonMises |

每份外推逻辑约 50 行，几乎一字不差（仅应力分量名不同）：

```cpp
std::map<std::string, std::vector<double>> nodeStressSum;
std::vector<double> nodeWeightSum(nPts, 0.0);
for (eleSub : _eleSubs)
    for (eleID : eleSub->_eleIds) {
        ... 构造 r / coef ...
        uResult res = eleSub->uEle(r, coef, matParams);
        // eleResult → _elemRes
        if (res.nodeResult.count("weight")) {
            for (i : nodeIds) {
                nodeWeightSum[i] += weights[i];
                for (name : stressNames)
                    nodeStressSum[name][i] += res.nodeResult[name][i];
            }
        }
    }
for (name : stressNames)
    _nodeRes[name][i] = nodeWeightSum[i] > 0 ? nodeStressSum[name][i] / nodeWeightSum[i] : 0;
```

## 3. 基类的死代码

基类 `PhyFieldData::uPhy` 的 `nodeResult` 处理却是另一套逻辑：

```cpp
for (it : res.nodeResult)
    if (_elemRes 有 resName && resVals.size() == 1)
        _elemRes[resName][eleID] = resVals[0];   // 当单元结果
```

即基类把 `nodeResult` 当"单值单元结果"写 `_elemRes`，**不做节点外推**。但所有应力派生都重写了 `uPhy`（走 weight 外推），无人用基类这套——**这是死代码**。

## 4. 统一方案

### 基类新增辅助方法

```cpp
class PhyFieldData {
protected:
    // 单元结果 + 节点结果加权外推：uEle 返回 "weight" 时最小二乘外推到节点
    void extrapolateNodeResults(const std::vector<std::string>& nodeResNames);
    // 由 3 个应力分量算 von Mises 等效应力
    void computeVonMises(const std::string& sXX, const std::string& sYY,
                         const std::string& sXY, const std::string& outName = "vonMises");
public:
    std::vector<std::string> _nodeExtrapNames;   // 要外推的结果名
    bool _bVonMises = false;                      // 是否算 vonMises
};
```

`extrapolateNodeResults` 封装上面的 weight 外推循环；`computeVonMises` 封装等效应力公式。

### 基类 uPhy 重构

```cpp
int PhyFieldData::uPhy() {
    // resize + 回填位移（不变）
    ...
    extrapolateNodeResults(_nodeExtrapNames);
    if (_bVonMises && _nodeExtrapNames.size() >= 3)
        computeVonMises(_nodeExtrapNames[0], _nodeExtrapNames[1], _nodeExtrapNames[2]);
    return 1;
}
```

同时删除 `nodeResult → _elemRes[size==1]` 死代码。

## 5. 静力场 vs 动力场

### 静力场：删 uPhy 重写

`ElDisp` / `Elastic2D` / `Hel2D DelDisp` 三个静力场，位移回填是标准的（`_nodeRes[disp] = _rhs[id]`），与基类 `uPhy` 完全一致。改造后只需在构造函数声明外推名：

```cpp
ElDispFieldData::ElDispFieldData(...) {
    ...
    _eleResNames     = {"sigmaXX", "sigmaYY", "sigmaXY", "volume"};
    _nodeExtrapNames = {"sigmaXX", "sigmaYY", "sigmaXY"};   // 外推名（不含 volume）
    _bVonMises = true;
}
```

删除整个 `uPhy` 重写（每份减 66~110 行），直接用基类。

### 动力场：保留 uPhy，调辅助

DEl2D（Newmark 动力学）的位移回填是 Newmark 历史（`_u`/`_v`/`_w` + `velU`/`accU`），基类不知 Newmark，必须保留 `uPhy` 重写。但应力外推段改调辅助：

```cpp
int DelDispFieldData::uPhy() {   // DEl2D
    ensureHistorySize();
    // 1) _u 回填 + Newmark 更新 _v/_w（保留）
    // 2) _nodeRes[u/v/velU/velV/accU/accV] = _u/_v/_w（保留）
    for (str : _eleResNames) _elemRes[str].resize(nElem);
    extrapolateNodeResults(_nodeExtrapNames);               // 替代原 ~60 行外推循环
    computeVonMises("sigmaXX", "sigmaYY", "sigmaXY");
    _u1 = _u; _v1 = _v; _w1 = _w;                            // 保存历史（保留）
    return 1;
}
```

**等价性**：DEl2D 原外推用 `_u` 作 coef，但 `_nodeRes["u"/"v"]` 已在步骤 2 回填为 `_u`，基类辅助用 `_nodeRes[dispName]` 完全等价。

## 6. 小结

- **DRY**：weight 外推逻辑从 4 份 → 1 份（基类）。
- **死代码清理**：基类 `nodeResult → _elemRes[size==1]` 分支删除。
- **配置化**：外推名 + vonMises 开关由派生构造声明，静力场零 `uPhy` 代码。
- **动力场兼容**：Newmark 逻辑保留，仅外推段复用基类。

代码量：**+83 / −369，净减 286 行**。回归 14 case 全绿（`max|Δ| = 0`）。

设计哲学：**识别真正的差异，把相同的提到基类，把不同的留给派生**。4 个 `uPhy` 看似都"重写"，实则只有位移回填不同（静力 vs Newmark），应力外推完全相同。重构后，派生只表达"我有什么不同"（外推名、位移来源），公共逻辑隐于基类。
