# CDFEG 后处理统一改造：从分散调用到多 Processor 架构

> 摘要：CDFEG 有限元库的后处理原本调用分散、结果项定义绑定 GiD、vtkPost 仅能单步输出。本次改造把后处理统一到 `DomainData::post(it)`，将 `GidResItem` 通用化为 `Processor` 基类的 `ResItem`，并让 `vtkPost` 支持多时间步 PVD 时间序列。改造后，一个计算模型可同时挂载多个后处理器（GiD + VTK），由统一入口驱动，且 VTK 输出纳入了回归测试。

## 1. 背景：CDFEG 的后处理抽象

CDFEG（创刀有限元程序生成系统）采用三层架构：

- **DomainData**：网格 / 材料 / 物理场 / 流程控制
- **PhyFieldData**：自由度 / 边界 / 组装求解 / 后处理
- **ElementBase**：单元刚度 / 应力

前后处理由 `Processor` 基类抽象：

```cpp
class Processor {
public:
    Processor(DomainData* data);  // 构造时自动 _processors.push_back(this)
    virtual int pre();            // 前处理（读网格/材料）
    virtual int post(int it = 0); // 后处理（输出结果）
    DomainData* _femData;
};
```

两个具体处理器：

- `GidPrePost`：GiD `.dat` 前处理 + `.post.msh` / `.post.res` 后处理
- `vtkPost`：VTK/VTU 后处理

关键设计：`Processor` 构造时**自动登记**进 `DomainData::_processors`（观察指针，`DomainData` 析构不 `delete`）。这使得"一个模型挂多个处理器"在数据层面已经支持——但改造前并没有人遍历 `_processors` 去调 `post`。

## 2. 改造前的三个痛点

### 痛点一：后处理调用入口分散

动力学示例 `del2dData` 自持 `_prePost` 指针：

```cpp
class del2dData : public DomainData {
    GidPrePost* _prePost;        // 自持指针
    void setPost(GidPrePost* p);
};

int del2dData::caculate() {
    for (int it = 0; it < nStep; ++it) {
        ...
        if (_prePost) _prePost->post(it);   // 直接调
    }
}
```

静力示例则更直接——在 `main` 里调 `gidPrePost.post()`。`_processors` 列表白白维护。

### 痛点二：结果项定义绑定 GiD

`GidResItem`（结果项：位移/应力等）+ `GidLocation{OnNodes, OnGaussPoints}` + `GidResultType{Scalar, Vector2, ..., PlainDeformationMatrix, LocalAxes, Complex*, ...}`（17 个 GiD 专有值）是 `GidPrePost` 的专属成员。`vtkPost` 用不上这套，只能遍历物理场的全部 `_dispNames` / `_eleResNames` 笼统输出。

### 痛点三：vtkPost 单步 + 硬编码

```cpp
int vtkPost::post(int it) {
    if (it == 0) writeVTK("result.vtk");   // 仅首步，硬编码文件名
    return 0;
}
```

动力学多步场景下，`vtkPost` 只输出第一步；无路径配置；输出内容不可控。

## 3. 三项改造

### 改造一：`DomainData::post` 统一入口

```cpp
void DomainData::post(int it) {
    for (Processor* p : _processors) p->post(it);
}
```

`del2dData` 删除自持 `_prePost` / `setPost`，`caculate` 改调 `post(it)`。各示例 `main` 统一为 `data.post(0)`（静力）或由 `caculate` 内部 `post(it)`（动力）驱动。

这是**控制反转**：派生类不再关心有几个处理器、是什么类型，只管"该输出了"。

### 改造二：`ResItem` 通用化

重命名 + 枚举精简：

| 旧 | 新 |
|---|---|
| `GidResItem` | `ResItem` |
| `GidLocation{OnNodes, OnGaussPoints}` | `ResLocation`（同名） |
| `GidResultType{17 个 GiD 专有值}` | `ResType{Scalar, Vector, Matrix}` |

`_resItems` 从 `GidPrePost` 提升到 `Processor` 基类（每实例一份）。`GidPrePost` 内部把 `ResType` 经 `resTypeToStr` 映射为 GiD 字符串（`"Scalar"` / `"Vector"` / `"Matrix"`），输出逻辑不变。

GiD 专有枚举值（`PlainDeformationMatrix` / `LocalAxes` / `Complex*`）无示例使用，按 YAGNI 移除。

### 改造三：`vtkPost` 多步 PVD

```cpp
class vtkPost : public Processor {
public:
    void setFilePath(const string& parentPath, const string& baseName);
    int post(int it = 0);   // 每步写 baseName_<it>.vtu，全量重写 baseName.pvd
private:
    int writeVTU(const string& fn);
    int writePVD(const string& fn);
    vector<pair<int,double>> _steps;   // (it, time=it*_dt)
};
```

要点：

- **按 `_resItems` 输出**：与 `GidPrePost` 同口径，`main` 注册什么就输出什么。
- **PVD 时间序列**：每步一个 `.vtu` + 一个 `.pvd` 索引（`Collection`），ParaView 打开 `.pvd` 即可按真实时间 `time = it·dt` 动画播放。
- **高精度**：`setprecision(15) scientific`，支撑 `1e-12` 紧回归容差。
- **PVD 全量重写**：每次 `post(it)` 后基于 `_steps` 重写 `.pvd`，即便中途异常退出，已写步的索引仍有效。

## 4. 多 Processor 并存：DEl2D 实战

DEl2D（Newmark 动力学）同时挂 `GidPrePost` + `vtkPost`，用 lambda 对两个处理器各注册同一套结果项：

```cpp
del2dData data;
GidPrePost gidPrePost(&data);   // 自动进 _processors
gidPrePost.pre();
vtkPost vtkpost(&data);          // 自动进 _processors
vtkpost.setFilePath(path, project);

auto registerItems = [](Processor& p) {
    ResItem disp("disp", ResType::Vector);
    disp.addVal(0, "u"); disp.addVal(0, "v");
    p._resItems.push_back(disp);
    // velocity / acceleration / stress 同理
};
registerItems(gidPrePost);
registerItems(vtkpost);

data.caculate();   // 内部 post(it) 同时驱动 res + vtu 群 + pvd
```

运行一次，产出 `del2d.post.res`（11 步 GiD）+ `del2d_0000.vtu` … `del2d_0010.vtu` + `del2d.pvd`（11 步动画）。

## 5. 回归测试扩展

要让 VTK 输出纳入回归，需解析 VTU/PVD。测试框架用 `format` 字段分发解析器（`gid` / `truss_txt`），复用 comparator。新增 `pvd` format：

```python
def parse_vtu_file(path, step):
    # VTU XML → {(name, step): ResBlock}
    # PointData → OnNodes，CellData → OnCells，分量名统一 comp_N

def parse_pvd_file(path):
    # 按 <DataSet> 顺序（顺序索引即 step）逐个解析引用的 vtu，合并
```

新增 `del2d1_vtk` case（`format = "pvd"`），基线是 `del2d.pvd` + 11 个 `del2d_*.vtu`。`--update` 时整体拷贝 vtu 群（多文件基线）。

## 6. 小结

- **统一入口**：`DomainData::post(it)` 遍历 `_processors`，派生类不再自持处理器指针。
- **通用化**：`ResItem` 提升到 `Processor` 基类，GiD 专有枚举精简为通用 `ResType`。
- **多步 VTK**：`vtkPost` 支持 PVD 时间序列，与 GiD 同口径按 `_resItems` 输出。
- **多 processor**：一个模型可同时输出 GiD + VTK，互不干扰。

设计哲学：**控制反转 + 数据驱动**。派生类只管"何时输出"，输出什么（`ResItem`）、怎么输出（GiD/VTK）由 `main` 注册的数据决定，处理器各司其职。
