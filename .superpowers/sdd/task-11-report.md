# Task 11 报告：el caculate 命令流核对 + main 完整构建运行

## Status: DONE_WITH_CONCERNS

## 1. caculate 命令流核对

### 发现
- `elData::caculate()` 原为空壳（`return 1`），Task 4 的 gcn 生成命令流未实际填入。
- 填充为 ela→elb 顺序，各 4 步（initMatrix→eProgram→solve→uPhy）。
- ela 在 elb 之前，保证 elb `getCoef` 时 ela._nodeRes（位移 u/v）已回填。**顺序正确**。

### 改动
- `macs/El/el/elData.cpp`：填充 caculate() 体。
- `macs/El/el/main.cpp`：统一缩进，补末尾换行（无逻辑变更）。

## 2. 构建

- `cmake -B macs/El/build -S macs/El && cmake --build macs/El/build --config Debug` 成功。
- 产物：`macs/El/build/output/Debug/el.exe` + `CDFEG.dll`。
- 警告：C4819（UTF-8 编码，macs/El 的 CDFEG 副本未配 `/utf-8`）、C4251（DLL 导出模板）。均不影响运行。

## 3. GiD 输入数据格式

### CDFEG 标准格式（.dat）
- `*` 开头节头行，包含 `name/structure/type` 参数。
- GidPrePost::setFilePath 拼接 `parentPath\name.dat`。
- 各 section 类型：mat（材料）、coord（节点坐标）、id（边界索引）、ubf（一类边界值）、elem（单元）。
- `.bas` 模板由 GiD 展开，生成 .dat 供 CDFEG 读取。

### macs/El 的 .bas 文件（Task 5 生成）
- 格式正确，与 CDFEG 标准 dat 格式匹配。
- 单元类型名 a1eq4g2/a2ll2/beq4g2 与单元 _name 一致。
- **注意**：`readID` 在当前 CDFEG 中是空实现（解析了 vals 但未使用）。一类边界由 `readUBF` 施加，通过 `ubf<name>` 的 name 前缀匹配物理场。

### 测试数据
- 手写最小 4 节点矩形单元 `.dat`（1 个 a1eq4g2 + 1 个 a2ll2 + 1 个 beq4g2）。
- 放于 `macs/El/testData/el.dat`（不入库）。

## 4. 运行结果

### 首次运行：崩溃/卡死
- `uPhy` 中 `_elemRes[resName][eleID]` 对未预分配 key 导致内存越界。
- uEle 返回 `eleResult["sigmaXX"]`，但 `_eleResNames` 是 `{"exx","eyy","exy"}`。
- `_elemRes["sigmaXX"]` 自动创建空 vector，`[0]` 越界写入，引发未定义行为。

### Bug 修复
- `PhyFieldData::uPhy` 中写入 `_elemRes` 时加 `find` + 范围检查。
- **同步修复了主仓库 `FEMproject/CDFEG/PhyFieldData.cpp`**（同 bug）。

### 修复后运行：成功
- el.exe 正常退出（exit code 0），输出 `el.post.msh` + `el.post.res`。
- ela: neq=8（4节点 x 2自由度），elb: neq=12（4节点 x 3自由度）。
- 结果全 0：预期行为（测试数据无载荷：a2ll2::run 空壳，a1eq4g2 fv=0）。

## 5. 改动文件

| 文件 | 改动 |
|---|---|
| `macs/El/el/elData.cpp` | 填充 caculate() 命令流 |
| `macs/El/el/main.cpp` | 统一缩进 + 补末尾换行 |
| `FEMproject/CDFEG/PhyFieldData.cpp` | 修复 _elemRes 越界写入 bug |
| `macs/El/CDFEG/PhyFieldData.cpp` | 同上修复（CDFEG 独立副本） |
| `macs/El/testData/el.dat` | 测试数据（不入库） |

## 6. Commit

`c1942dc` 🐛 fix(核心库): 修复 uPhy _elemRes 越界写入 + 填充 el caculate 命令流

## 7. Self-Review

- [x] caculate 顺序正确（ela 先 elb 后）
- [x] 构建通过
- [x] 运行不崩溃，输出结果文件
- [x] GiD 数据格式搞清
- [x] 发现并修复真实 bug（_elemRes 越界）
- [x] 中文提交注释
- [x] 不改核心库逻辑（仅修复 bug）
- [x] 运行产物不入库

## 8. 留给 Task 12 的项

1. **结果正确性验证**：当前测试数据无载荷，结果全 0。需要准备有载荷的真实网格数据（可能需要 GiD 生成或参照旧 macs/el 测试数据的几何+边界），对比旧系统结果。
2. **a2ll2 单元 run 空壳**：a2ll2::run 返回空 estif 和空 eload，面力载荷未实际施加。需填充 a2ll2 的 run 计算逻辑。
3. **readID 空实现**：当前一类边界仅通过 readUBF 施加。readID 是空循环体，二类边界也未实现。需确认旧系统的 el.dis 格式是否需要 readID 支持。
4. **a1eq4g2::uEle 结果键名不匹配**：uEle 返回 "sigmaXX/sigmaYY/sigmaXY"，但 _eleResNames 是 "exx/eyy/exy"。修复后虽不越界但结果不写入 _elemRes。需对齐键名或在 Task 12 中用 post2() + GidResItem 输出。
5. **macs/El 的 CDFEG 副本与主仓库同步**：macs/El 有独立的 CDFEG 副本，未来应统一或定期同步。

## 9. Task 11 Review 修复：a1eq4g2::uEle 键名不匹配（单元结果丢失）

### 问题
`a1eq4g2::uEle` 返回的 eleResult 键名为 `sigmaXX/sigmaYY/sigmaXY/volume`（Task 7 参考 ElQ4g2 写成应力输出），但 `elaFieldData::_eleResNames` 是 `{"exx","eyy","exy"}`（来自 a1eq4g2.ges 的 `func = exx,eyy,exy` 段）。`PhyFieldData::uPhy` 中通过 `_elemRes.find(resName)` 查找键名，sigmaXX 找不到则静默跳过，导致 ela 单元应变结果全部丢失。

### 选项选择：A（输出应变，对齐 ges 语义）
- **ges func 段**：`exx=+[u/x]`, `eyy=+[v/y]`, `exy=+[u/y]+[v/x]` 明确是工程应变。
- **旧 C 代码 a1eq4g2.c**：无 uEle/后处理代码，ela 在旧系统中不输出单元结果（只位移）。
- **el 项目分工**：ela 场输出位移+应变（exx/eyy/exy），elb 场（最小二乘平滑）输出应力（dxx/dyy/dxy）。
- 排除 B（改 _eleResNames 为应力名）和 C（清空 _eleResNames），因为 ges func 段和 el/elb 分工明确支持选项 A。

### 改动
- `macs/El/el/a1eq4g2.cpp::uEle`：
  - 移除应力本构计算（sigma = D * epsilon）。
  - 高斯点直接计算应变 exx=du/dx, eyy=dv/dy, exy=du/dy+dv/dx（与 ges func 段公式一致）。
  - 形函数加权外推到节点（框架不变，变量名从 sigmaXX→exx 等）。
  - eleResult 键名改为 exx/eyy/exy，nodeResult 键名同步修改。
  - 移除 volume/weight 辅助输出（不在 _eleResNames 中）。

### 验证
- 编译通过（MSVC Debug），运行 el.exe 正常退出。
- uPhy 中 `_elemRes.find("exx")` 现在能匹配到已预分配的 vector，单元应变结果正确写入 `_elemRes`。
- 注意：当前 `post()` 只输出 `_nodeRes`（位移），不输出 `_elemRes`。单元结果需后续 Task 12 中使用 post2() + GidResItem 才能写入 res 文件。

### Commit
`9ebeb6a` fix: A1eq4g2 uEle 输出应变 exx/eyy/exy 对齐 _eleResNames（修复单元结果丢失）
