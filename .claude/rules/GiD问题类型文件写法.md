# GiD 问题类型文件写法（.bas / .cnd / .prb / .bat）

> 本文件记录 GiD「问题类型（Problem Type）」四类自定义文件的官方写法，以及 CDFEG/pyTool 当前如何生成与使用它们。
> **核心结论**：pyTool 用 `MakerGidFile` + `template/gid*.j2` 生成这四类文件；GiD 加载 `.cnd/.prb` 供用户在前处理界面赋值，运行时按 `.bas` 模板把网格+条件渲染成文本 `.dat`，再由核心库 `GidPrePost::pre()` 按段解析。
> **关联**：pyTool [CLAUDE.md](../../pyTool/CLAUDE.md)、`MakerGidFile.py`；核心库前后处理机制见 [核心库实现细节.md](./核心库实现细节.md)；pyTool 能力边界见 [pyTool能力边界.md](./pyTool能力边界.md)。

---

## 1. GiD Problem Type 体系

GiD 通过「问题类型」定制分析流程。一个问题类型是 `.gid/` 目录下一组同名文件：

| 文件 | 角色 | GiD 何时用 | pyTool 模板 |
| --- | --- | --- | --- |
| `.cnd` | **条件（Conditions）**定义：边界条件、材料号、初值等"可赋值字段"的 schema | 前处理界面 `Data → Conditions` 列出字段供用户填 | `gidcnd.j2` |
| `.prb` | **问题数据（Problem Data）**定义：全局参数、材料参数表等 schema | 前处理界面 `Data → Problem Data` 弹窗 | `gidprb.j2` |
| `.bas` | **模板（Template）**：用 GiD 模板命令把网格+条件渲染成求解器输入文件 `.dat` | `Calculate → Calculate` 时执行 | `gidbas.j2` |
| `.bat` | **批处理**：GiD 计算时调用，负责启动外部求解器 | `Calculate → Calculate` 时执行 | `gidbat.j2` |

> CDFEG 未用独立 `.mat` 材料文件——材料参数走 `.prb` 的 `#N#` 矩阵字段（见 §3），由 `.bas` 用 `*GenData` 读回。

---

## 2. `.cnd` 条件文件

### 语法

一个条件块：

```
NUMBER: 1
CONDITION: <唯一条件名>
CONDTYPE: over <points|lines|surfaces|volumes|layers>
CONDMESHTYPE: over <nodes|body elements|face elements>
QUESTION: <字段名>
VALUE: <默认值>
  ...（可多组 QUESTION/VALUE）
END CONDITION
```

- **`CONDTYPE`**：条件可施加到的**几何实体**类别（points/lines/surfaces/volumes）。一个条件只能属于一个几何类别。
- **`CONDMESHTYPE`**：网格生成后，条件映射到的**网格实体**类别（nodes/body elements/face elements）。`body element`=单元本身；`face element`=单元的一个面（如"单元 2565 的面 2"）。
- 正常工作流：用户只在**几何**上赋条件 → 网格化时 GiD 自动把条件映射到 `CONDMESHTYPE` 指定的网格实体。手动给网格实体赋条件不推荐（重新网格化会丢失）。
- `QUESTION` 字段名后可带类型标记：`#CB#(opt1,opt2)` 复选框、`#N#` 多值矩阵（见 §3）。

### CDFEG 约定（`gidcnd.j2`）

每个物理场 × 4 个几何类别（volume/surface/line/point）各生成一个位移条件，每自由度两个字段：

```
CONDITION: volume-<proj><field>        # 例：volume-elElDisp
CONDTYPE: over volumes
CONDMESHTYPE: over nodes
QUESTION: u-I        # -I = 约束标识（-1=约束该自由度）
VALUE: -1
QUESTION: u-D        # -D = 边界值（位移或力的大小）
VALUE: 0.0
END CONDITION
```

- `-I` / `-D` 双字段：`-I` 为约束开关，`-D` 为数值。`.bas` 中 `*cond(2*i+1)` 取 `-I`、`*cond(2*i+2)` 取 `-D`（见 §4）。
- 动力学场额外生成 `D1/D2/D3`（初位移/初速度/初加速度）条件。
- 每个单元类型生成一个材料号条件 `QUESTION: mate_Num / VALUE: 1`，`CONDTYPE` 随单元几何类别（面单元 over Surfaces，边单元 over Lines）。

> **命名一致性**：`.cnd` 中的条件名（如 `Surface-ElQ4g`、`volume-elElDisp`）必须与 `.bas` 中 `*Set Cond` / `*set cond` 引用的名字完全一致（含大小写、含 `volume-`/`Surface-` 前缀）。

---

## 3. `.prb` 问题数据文件

### 语法

```
PROBLEM DATA
QUESTION: <字段名>['#CB#'(...)]      # #CB# = 复选框多选
VALUE: <默认值>
  ...
END PROBLEM DATA

INTERVAL DATA                         # 随时间区间变化的字段（CDFEG 未用）
QUESTION: ...
VALUE: ...
END INTERVAL DATA
```

### `#N#` 矩阵字段（CDFEG 材料参数表用此）

一个字段可声明为"矩阵"（多行多列）：

```
QUESTION: <字段名>(<列参数名1>,<列参数名2>,...)
VALUE: #N# <值总数> <v1> <v2> ... <vN>
```

- `#N#` 后第一个数是**值的总数** = 列数 × 行数。
- 列数 = `QUESTION` 括号里参数个数；行数 = 用户添加的组数。
- 例：`QUESTION: ElQ4g(pe,pv,fu,fv,rou,alpha)`（6 列），用户添 1 种材料 → `VALUE: #N# 6 1.0e10 0.3 0.0 0.0 0.0 0.0`；添 3 种 → `#N# 18 ...`（18=6×3）。

### CDFEG 约定（`gidprb.j2`）

- 动力学：`QUESTION: TimeStep/VALUE:0.1`、`QUESTION: TotalTime/VALUE:1.0`。
- 每个材料组：`QUESTION: <gidName>(<p1>,...,<pn>) / VALUE: #N# <n> <默认值...>`。
- `*GenData(<gidName>,int)` 在 `.bas` 中取回值总数（即 `#N#` 后那个数）。

---

## 4. `.bas` 模板文件（重点）

### 4.1 两条铁律：`*` 命令 vs `**` 注释

| 前缀 | 含义 | 渲染时行为 |
| --- | --- | --- |
| `*xxx`（单星号） | **GiD 模板命令** | 执行/替换，不出现在输出 `.dat` 里（除非命令本身打印值） |
| `**xxx`（双星号） | **注释** | **原样输出**到 `.dat`（作为注释行） |

> **CDFEG 关键设计**：借 `**` 注释原样输出的特性，注入自定义段头 `** name=...,structure=...,type=...,index=...` 到 `.dat`，供 `GidPrePost::pre()` 按段解析（见 §4.4）。

### 4.2 常用命令清单（pyTool 实际用到的）

| 命令 | 作用 | 示例 |
| --- | --- | --- |
| `*loop nodes` / `*loop elems` / `*loop materials` | 遍历节点/单元/材料 | `*loop nodes` ... `*end nodes` |
| `*end` / `*end nodes` / `*end elems` | 结束循环 | `*end` |
| `*all` / `*OnlyInCond` / `*OnlyInLayer` | 循环修饰符（跟在 `*loop nodes\|elems` 后） | `*loop nodes *OnlyInCond`（只遍历有当前条件的实体） |
| `*Set Cond <名> *nodes\|*elems` | 选定一个条件命中的实体集 | `*Set Cond volume-elElDisp *nodes` |
| `*Add Cond <名> *nodes\|*elems` | 向已选集合**追加**另一条件命中集 | `*Add Cond surface-elElDisp *nodes` |
| `*Remove Cond <名>` | 从已选集合移除 | — |
| `*cond(n[,type])` | 取当前实体第 n 个条件字段值；`type` 可 `int`/`real` | `*cond(1)`、`*cond(2,int)` |
| `*GenData(字段名\|序号[,type])` | 取 `.prb` PROBLEM DATA 字段值 | `*GenData(ElQ4g,int)`、`*GenData(TimeStep)` |
| `*IntvData(...)` | 取 INTERVAL DATA 字段（CDFEG 未用） | — |
| `*NodesNum` | 当前节点号 | `*NodesNum` |
| `*NodesCoord([1..3][,real])` | 当前节点坐标（可指定第几维） | `*NodesCoord`、`*NodesCoord(1,real)` |
| `*ElemsNum` | 当前单元号 | `*ElemsNum` |
| `*ElemsMat` | 当前单元材料号 | `*elemsmat` |
| `*elemsConec[(n)\|(swap)]` | 当前单元连接（节点号列表）；`(n)` 取第 n 个；`(swap)` 角点/中点重排 | `*elemsConec` |
| `*globalnodes` | 面条件的全局节点号（面单元/边单元用） | `*globalnodes` |
| `*format "<printf 串>"` | 设置输出格式（C printf） | `*format "%6i %12.6e %12.6e"` |
| `*operation(<expr>)` | 算术运算（支持 `+ - * /`、`abs`/`pow`/`sqrt`/`strcmp`）；内部命令**不带** `*` | `*operation(i+5)`、`*operation(4*elemsnum+1)` |
| `*set var <名>=<值\|operation(...)\|clock>` | 赋值变量 | `*set var N=GenData(ElQ4g,int)` |
| `*for(init;cond;step)` ... `*end for` | C 风格循环 | `*for(i=1;i<=N;i=i+6)` |
| `*if(<cond>)` / `*else` / `*endif` | 条件 | `*if(GenData(Degrees_Freedom_Nodes,int)>=3)` |
| `*\\`（行末反斜杠） | 抑制换行（行续） | `-1*\\` |
| `*tcl(<proc> <args>)` | 调 Tcl 过程，打印其返回文本（复杂逻辑逃生口） | `*tcl(MyGetKey *layernum)` |
| `*npoin` / `*nelem` / `*ndime` / `*nmats` | 节点数/单元数/维度/材料数 | `*npoin  *nelem` |

> **注意**：命令与括号间**不能有空格**（`*cond(1)` 正确，`*cond (1)` 错误）。

### 4.3 核心范式：收集"任意几何实体上的条件"到节点

CDFEG 允许同一条件施加在体/面/线/点任一几何类别上，统一收集到节点集再遍历：

```
*Set Cond volume-<名> *nodes          # 先设一个条件集
*Add Cond surface-<名> *nodes         # 追加其余 3 个类别
*Add Cond line-<名> *nodes
*Add Cond point-<名> *nodes
*loop nodes *OnlyInCond               # 只遍历命中的节点
*NodesNum *cond(1) *cond(3)           # 取节点号 + 条件字段
*end
```

### 4.4 CDFEG `.dat` 段头协议（借 `**` 注释）

每个数据段以一行 `**` 注释作为段头，`GidPrePost::pre()` 按段名分发解析：

```
** name=<段名>,structure="<结构>",type="<类型>",index=<场号>
```

- **`structure`**：描述该段每行数据的字段序列（文本 `.dat` 下仅作解析指引，非二进制编码）：
  - `I32`=一个 32 位整数；`F64`=一个 64 位浮点。
  - `I32**N` = N 个 I32；`F64**N` = N 个 F64。
  - 混合：`I32**1 F64**2` = 1 整数 + 2 浮点（如 ubf 段：节点号 + dof 个边界值）。
- **`type`**（段类型枚举，`pre()` 据此分发）：
  - `mat`（材料）、`coord`（坐标）、`id`（约束号）、`ubf`（边界力/位移值）、`elem`（单元连接）、`dbc`、`initVal`（动力学初值）。
  - 全局段（`baseData` 的 npoin/nelem、`time`）不带 `type`。
- **`index`**：所属物理场号。
- **铁律**：`structure` 声明的字段数/类型，必须与该段循环体实际每行打印的字段**完全一致**，否则 `pre()` 解析错位。

### 4.5 产物示例（`El2D/el.gid/el.bas` 节选）

```
** name=baseData,structure="I32"                  # 全局：npoin nelem
*npoin  *nelem
** name=mat_ElQ4g,structure="F64**6",type="mat",index=0     # 材料段，每行6个浮点
*set var N=GenData(ElQ4g,int)                     # 取材料组数×6
*for(i=1;i<=N;i=i+6)
*GenData(ElQ4g,*operation(i+0)) *GenData(ElQ4g,*operation(i+1)) ...   # 6列
*end for
** name=coord,structure="I32**1 F64**2",type="coord",index=1  # 节点号+x+y
*loop nodes
*format "%6i %12.6e %12.6e"
  *NodesNum *NodesCoord
*end
** name=idElDisp,structure="I32**3",type="id",index=0         # 约束号：节点号+2个-I
*Set Cond volume-elElDisp *nodes                # 收集4类几何上的条件
*Add Cond surface-elElDisp *nodes
*Add Cond line-elElDisp *nodes
*Add Cond point-elElDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1) *cond(3)                      # cond(1)=u-I, cond(3)=v-I
*end
** name=ElQ4g,structure="I32**6",type="elem",index=1          # 单元连接
*set cond Surface-ElQ4g *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)                   # 单元号+连接+材料号
*format "%10i %10i %10i %10i %10i %10i "
*end
```

---

## 5. `.bat` 批处理

GiD 计算时调用 `<project>.bat`，传入三个位置参数：

| 参数 | 含义 |
| --- | --- |
| `%1` | 项目名（project name） |
| `%2` | 模型目录（model directory，`.dat` 所在） |
| `%3` | 问题类型目录（problem type directory，`.bat`/`.exe` 所在） |

CDFEG 约定（`gidbat.j2`）：

```bat
@echo off
echo project: %1
echo project directory: %2
echo problem directory: %3
%3\<name>.exe %1 %2
```

即调用问题类型目录下的 `<name>.exe`，传 `<project> <path>`；`mainGid.cpp` 的 `argv[1]=project`、`argv[2]=path`，读 `path/<project>.dat`。

> `.bat` 必须全英文（项目全局约定）；建立后要测试修正。

---

## 6. 关键陷阱与约定

| 陷阱/约定 | 说明 |
| --- | --- |
| `*` vs `**` | 单星号=命令（执行），双星号=注释（原样输出）。CDFEG 段头借 `**` 注入。 |
| 条件名大小写 | `.cnd` 的 `CONDITION:` 名与 `.bas` 的 `*Set Cond` 名必须逐字符一致（含 `Surface-`/`volume-` 前缀、大小写）。 |
| `CONDTYPE` 限几何类别 | `over surfaces` 的条件只能施加到几何面，不能给点/线/体。用户只对几何赋值，网格化自动映射到 `CONDMESHTYPE`。 |
| `*cond(n)` 作用域 | 必须在 `*Set Cond` 之后的 `*loop ... *OnlyInCond` 内使用；条件值是每实体局部值，非全局。 |
| 段头 `structure` 一致性 | `** name=,structure=` 声明的字段数/类型必须与循环体每行实际打印一致，否则 `pre()` 解析错位。 |
| `.bas` 不是图灵完备 | 无循环 over 字段名、无真正函数；复杂逻辑用 `*tcl(...)` 调 Tcl 过程。 |
| 材料数据位置 | CDFEG 走 `.prb` 的 `#N#` 矩阵字段 + `.bas` `*GenData` 读取，**未用独立 `.mat`**。 |
| 边单元连接 | 面力/边条件用 `*globalnodes`（面条件的全局节点号），体单元用 `*elemsConec`。 |
| `MateTypeName` 与 `gidName` | `.cnd`/`.prb` 中的单元条件名/材料字段名用 `gidName`（GiD Condition 名，非 C++ 类名）。 |

---

## 7. 权威来源

- **GiD 16 Customization Manual (PDF)** — 官方最权威：<https://downloads.gidsimulation.com/GiD_Documentation/Docs/GiD16/GiD_16_Customization_Manual.pdf>
- **GiD Reference Manual – CUSTOMIZATION (HTML)** — 在线可查命令：<http://www-opale.inrialpes.fr/Aerochina/info/en/html-version/gid_16.html>
- **GiD 10 Customization Manual (PDF)** — 老版 `.cnd/.prb` 传统格式讲得最细：<https://mmech.com/images/stories/Standard_Products/GiD/GiD_10/GiD_Customization_Manual.pdf>
- **GiD 官方论坛**（GiD 开发者 escolano 解答）：<https://forum.gidsimulation.com/t/how-to-create-a-gid-problem-type/2660>

> 本地：GiD 安装目录 `help/` 下附当前版本对应的 customization manual 副本（与线上开发者版可能有差异，以本地版本为准）。
