# bas 变长节点单元列举改造（支持同 gidName 不同节点数单元）

> 日期：2026-07-27
> 范围：pyTool 生成器 + GiD bas 模板
> 关联：使 `ElQ4g`(4 节点) 与 `ElT3g`(3 节点) 等不同节点数单元能共用同一 `gidName`，在同一 GiD condition / dat 段内混排。

---

## 1. 背景与目标

**目标**：让 `testEl2D.py` 中 `ElQ4g` 与 `ElT3g`（以及一般地，任意同 `gidName`、不同节点数的单元）能在同一 `gidName` 下正确生成、读取、分流。

**前置改动（已完成）**：
- `DomainData.cpp:99` `addEle` 已加入「节点数 + 类型」双重判断（与 `addEdge:129` 对称），匹配端可按节点数把单元分流到对应 eleSub，且不破坏 Hel2D 跨场同 gidName 共享（`break` 只跳内层 `_eleSubs`，两场同节点数门槛恒成立）。

**剩余障碍**：生成端 bas 模板用单元 `nNodes` 焊死输出列数与段头 structure，导致同 gidName 合并段无法表达不同节点数单元。

## 2. 现状与关键发现

### 2.1 单元列举走 GiD Condition（非 element type 几何卡）
- `gidbas.j2:81-86`：elem 段用 `*set cond {type}-{gidName} *elems` 按 **Condition** 列举单元。
- `gidcnd.j2:54-59`：condition 的 `CONDTYPE: over {type}s`（如 `over surfaces`）是几何**大类**，**不限制节点数/拓扑** → 同一 `surface` condition 下可同时含 Q4(4) 与 T3(3)。
- 段头 `** name={{elem.name}}`，其中 `elem.name = gidName`（`MakerGidFile.py:93` `addElem(gidName,...)`）→ **dat 单元段名 = gidName**。
- 实测案例 `hel2d.bas:45-50`：HeatQ4g + DelQ4g 同 gidName=HelQ4g 合并成单段 `** name=HelQ4g`、单 condition `Surface-HelQ4g`。

### 2.2 前处理天然支持变长节点（决定性发现）
`gidPrePost.cpp:311-333` `readElement` **不读段头 `structure="I32**N"` 的 N**，节点数由每行实际整数个数决定：

```cpp
std::vector<int> vals = TextReader::splitInts(line, " ,");  // 按空格/逗号切
id = vals[0];              // 首个 = 单元号
vals.erase(vals.begin());
mateId = vals.back();      // 末个 = 材料号
vals.pop_back();
addEle(id, vals, name);    // 中间 = 节点号，个数 = 实际节点数
```

- Q4 行 `eid n1 n2 n3 n4 mate`（6 整数）→ 节点 4 个
- T3 行 `eid n1 n2 n3 mate`（5 整数）→ 节点 3 个

→ **前处理零改动**，且 `readElement` 只用 `params["name"]`，完全不依赖 `structure` 字段。

### 2.3 障碍精确定位
`gidbas.j2` elem 段两处被 `nNodes` 焊死：
- 第 81 行段头 `structure="I32**{{elem.nNodes+2}}"` —— 固定列数元信息。
- 第 85 行 `*format "%10i {{ '%10i ' * elem.nNodes }}%10i "` —— 固定 `nNodes+2` 输出列。

`MakerGidFile.py:93` 合并同 gidName 时 `addElem(gidName, group['ele'].nNodes, ...)` 只取组内**首个** ele 的 `nNodes` → 混排时 T3 被按首 ele 节点数输出/错位。

### 2.4 bas 可行性已实测
经实测确认：去掉 `*format` 行后，`*ElemsNum *elemsConec *cond(1)` 由 GiD 按**当前单元实际节点数**输出；`readElement` 的 `splitInts` 可正确解析。

## 3. 方案（改 gidbas.j2：删 format + 删 elem 段 structure）

### 3.1 改动清单

| 文件 | 改动 | 理由 |
|---|---|---|
| **`pyTool/template/gidbas.j2`** | ① 删 elem 段内 `*format` 行（第 85 行）；② 段头去掉 `structure="I32**(nNodes+2)"` 字段（第 81 行） | 去掉 format 后 GiD 按实际节点数输出；structure 不被 `readElement` 读取，删除后 `nNodes` 在模板中彻底无引用 |
| `pyTool/MakerGidFile.py` | **不改** | `elem.nNodes` 仍由 `addElem` 生成存入 `data['elems']`，但模板不再引用，成为未使用字段（无害，YAGNI） |
| `FEMproject/CDFEG/gidPrePost.cpp` | **不改** | `readElement` 已用 `splitInts` 变长读，且只用 `params["name"]`，不依赖 structure |
| `FEMproject/CDFEG/DomainData.cpp` | **不改** | `addEle:99` 已有节点数判断（前置改动） |

改后 elem 段（gidbas.j2）：
```
{%- for elem in elems %}** name={{elem.name}},type="elem",index={{elem.index}}
*set cond {{elem.type}}-{{elem.name}} *elems
*loop elems *OnlyIncond
*ElemsNum *{{elem.idType}} *cond(1)
*end
{%endfor%}
```

### 3.2 structure 字段处理：删除
- elem 段头的 `structure="I32**(nNodes+2)"` **删除**。`readElement` 只取 `params["name"]`，`structure` 对功能无任何作用；删除后段头仍保留 `name/type/index`，`gidPrePost.cpp:151` 的 `parseInfoLine` 仍能按 `type="elem"` 识别段并派发到 `readElement`。
- **其他段**（coord / id / ubf / mat 等）的 `structure` 字段**保留不动**——它们的 reader 可能依赖 structure（如 coord 段按维度读坐标），不在本次范围。

### 3.3 端到端链路（Q4/T3 同 gidName="X"）

```
testEl2D.py 设 ElQ4g/ElT3g 同 gidName
  → MakerGidFile 按 gidName 合并：单 condition Surface-X、单段 name=X（段头无 structure）
  → bas 无 format：Q4 行输出 4 节点、T3 行输出 3 节点（GiD 按实际拓扑）
  → readElement 逐行 splitInts 变长读 → addEle(id, 实际节点, "X")
  → addEle:99 节点数判断：4 节点→ElQ4g、3 节点→ElT3g，正确分流
```

建模前提：用户在 GiD 中给 Q4 与 T3 单元都打同一个 condition（如 `Surface-X`）。Condition 不限拓扑，混合网格可归同一 condition。

## 4. 向后兼容

- 现有 sample 的 `.bas`（`el.bas`/`hel2d.bas` 等）是**已生成**文件，模板改动不影响它们；只有重跑 `test*.py` 才生成新版（无 `*format`、elem 段无 structure）。
- 重跑后 `readElement` 用 `splitInts` 仍能解析（不依赖列宽与 structure）→ **El2D / Hel2D / DEl2D 功能不变**。
- 唯一差异：dat 单元行从等宽对齐（`%10i`）变为 GiD 默认空格分隔输出（纯 cosmetic，不影响解析）；elem 段头少了 structure 字段（不影响解析）。
- Hel2D（跨场同 gidName、同构 Q4）完全不受影响。

## 5. 验证计划

1. 改 `gidbas.j2`：删 elem 段 `*format` 行 + 段头 `structure` 字段。
2. 构造 **Q4+T3 混合网格** GiD 模型，所有单元打同一 condition（如 `Surface-Mix`）。
3. `testEl2D.py` 设 `ElQ4g`/`ElT3g` 同 `gidName="Mix"`，重跑生成。
4. 检查生成 bas：elem 段头无 structure、段内无 `*format`；运行生成的程序读 dat，确认：
   - 段被正确识别为 elem（`parseInfoLine` 按 type 派发）；
   - `ElQ4g._eleIds` 只含 4 节点单元、`ElT3g._eleIds` 只含 3 节点单元；
   - 单刚装配 / 求解 / 后处理无报错。
5. 回归：重跑 `testHel2d.py` 生成 Hel2D，跑 Hel2D 确认跨场共享仍正常（位移/温度结果与基线一致）。

## 6. 范围边界（不做）

- **不改** `MakerGidFile.py`、`gidPrePost.cpp`、`DomainData.cpp`（均已就绪）。
- **不删** 其他段（coord/id/ubf/mat）的 structure 字段（可能被各自 reader 依赖）。
- **不清理** `MakerGidFile` 中 `elem.nNodes` 未使用字段（YAGNI，无害）。
- **不引入** 固定列填充占位方案（方案C，侵入前处理，已排除）。
- **不做** 同 gidName 拆多段（方案B，与「段名=gidName→addEle eleType」矛盾，已排除）。
