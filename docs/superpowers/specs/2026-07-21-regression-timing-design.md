# 回归测试时间统计 + SQLite 持久化 + 性能回归检测

- **日期**：2026-07-21
- **状态**：待评审
- **影响模块**：`test/`（回归测试框架，纯 Python，无 C++ 改动）

## 1. 背景与目标

现状：`test/framework/` 回归测试框架只统计**精度**指标（`max_abs_err` / `n_points` / `n_over_tol`），完全没有任何耗时记录——全目录 `grep` 仅命中 `timeout`（超时控制），不记录执行耗时，也不持久化历史。

目标：
1. 记录每个用例的求解耗时；
2. 持久化到 SQLite 数据库，便于追溯历史；
3. 自动检测**性能回归**——与同用例上次 `pass` 记录对比，超阈值时告警。

## 2. 需求决策（已与用户确认）

| 维度 | 决策 |
| --- | --- |
| 统计粒度 | 每用例总耗时 + 整体合计 |
| 计时口径 | **不含编译**：`build` 完成后（run + parse + compare）的 wall-clock |
| 持久化 | SQLite，默认 `test/timing.db`（可配） |
| 回归检测基准 | 同 `case_name` 最近一次 `pass` 记录 |
| 回归阈值 | 相对增长 **5%**（可配） |
| 退出码影响 | 仅 WARN，**不影响退出码** |
| 编译耗时 | 仅在 `-v` 模式控制台打印，不入库 |

## 3. 关键设计张力：编译缓存 vs 回归检测准确性

`E2ECase.run()` 内部先调 `builder.build([target])`，而 `Builder` 有 `_built_targets` 缓存。配置中多个 case 共用同一 target：

- `del2d1` → `del2d`
- `el2d_bf1` / `el2_mfel1` → `el2d`

首个跑该 target 的 case 会真编译（数十秒），后续命中缓存（毫秒级）。若"总耗时"含编译，则耗时因**运行顺序、是否首次**剧烈波动——用它做性能回归检测会严重失真（编译慢 ≠ 求解器退化）。

**解决**：计时从 `build` 完成后开始（`run + parse + compare`）。这样耗时稳定反映**求解器 + 后处理**的真实开销，回归检测才有意义。`build` 耗时仅作为构建系统观察值，在 `-v` 模式打印，不入库。

## 4. 数据模型

### 4.1 `CaseResult` 扩展（`framework/case.py`）

新增三个字段：

```python
@dataclass
class CaseResult:
    name: str
    suite: str
    status: str
    metric: dict = field(default_factory=dict)
    detail: str = ""
    secs: float = 0.0              # 用例耗时（build 之后，秒）；skip 为 0
    timing_regress: bool = False   # 是否触发性能回归
    timing_detail: str = ""        # 回归说明，如 "上次 2.30s → 本次 3.50s (+52%)"
```

### 4.2 SQLite 表结构（`test/timing.db`）

```sql
CREATE TABLE IF NOT EXISTS timing (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    run_id     TEXT NOT NULL,      -- 本次启动时间戳，同次运行多 case 共享
    ts         TEXT NOT NULL,       -- 记录写入时间（ISO 8601）
    case_name  TEXT NOT NULL,       -- e2e.del2d1
    suite      TEXT NOT NULL,       -- e2e / unit / generator / analytical
    status     TEXT NOT NULL,       -- pass / fail / error / skip
    secs       REAL NOT NULL,       -- 用例耗时（秒）
    git_commit TEXT                  -- HEAD short sha（取不到为 NULL）
);
CREATE INDEX IF NOT EXISTS idx_case_ts ON timing(case_name, ts);
```

> `run_id` 用于聚合"本次运行"的全部记录；`ts` 是每条记录的真实写入时间。

## 5. 模块改动

### 5.1 新增 `framework/timing.py`

纯标准库 `sqlite3`，无新依赖。提供：

```python
class TimingDb:
    def __init__(self, db_path: Path): ...
    def init(self) -> None:
        """建表 + 索引（IF NOT EXISTS，幂等）。"""
    def insert(self, result: CaseResult, run_id: str, ts: str, git_commit: str | None) -> None:
        """写入一条记录。"""
    def last_pass_secs(self, case_name: str) -> float | None:
        """查同 case_name 最近一条 status='pass' 的 secs；无则 None。"""

def detect_regress(now: float, last: float | None, threshold: float) -> tuple[bool, str]:
    """判定性能回归。返回 (是否回归, 说明文本)。
    last 为 None 或 0 时不判（返回 False）；否则当 now > last*(1+threshold) 时判为回归。"""

def git_short_commit() -> str | None:
    """subprocess 调 git rev-parse --short HEAD；失败返回 None。"""
```

### 5.2 `framework/config.py`

新增 `TimingConfig` dataclass，并在 `Config.__init__` 解析 `[timing]` 段：

```python
@dataclass
class TimingConfig:
    enabled: bool = True
    db_path: str = "test/timing.db"   # 相对 PROJ_ROOT 解析
    regress_threshold: float = 0.05

# Config.__init__ 内：
tm = raw.get("timing", {})
self.timing = TimingConfig(
    enabled=tm.get("enabled", True),
    db_path=tm.get("db_path", "test/timing.db"),
    regress_threshold=float(tm.get("regress_threshold", 0.05)),
)
```

### 5.3 `framework/case.py` —— 各 `Case.run()` 包计时

口径统一：**`build` 完成后开始计时，到返回前结束**。`skip` 用例 `secs=0`。

- **`E2ECase`**：拆出 `_run_after_build(self, exes) -> CaseResult`，承载现有 `build` 之后的全部逻辑（prepare → run → parse → compare）。`run()` 改为：

  ```python
  def run(self, ctx) -> CaseResult:
      try:
          exes = self.builder.build([self.target])
      except Exception as e:
          return CaseResult(self.name, self.suite, "error", detail=f"构建失败: {e}")
      t0 = time.perf_counter()
      result = self._run_after_build(exes)
      result.secs = time.perf_counter() - t0
      return result
  ```

- **`UnitCase`**：同理，`build` 后用 `perf_counter` 包住 `runner.run` 调用，回填 `secs`。
- **`GeneratorCase`**：`build` 不适用，直接用 `perf_counter` 包住 `pytest` 子进程调用，回填 `secs`。
- **`AnalyticalCase` / `SkipCase`**：`secs=0`，不动计时。

### 5.4 `framework/report.py`

- 表格表头加一列：`CASE | STATUS | METRIC | TIME(s)`。
- 每行打印 `secs`（`secs > 0` 时显示 `%.2f`，否则空白）。
- 表格末尾打印**整体总耗时**：`总耗时: {sum(secs):.2f}s`。
- 失败/错误详情区之后，新增**回归详情区**：对每个 `timing_regress` 的 case 打印 `⚠ {name} {timing_detail}`。

### 5.5 `run_tests.py`

- `main()` 顶部生成 `run_id = time.strftime("%Y%m%d_%H%M%S")`。
- 现有 `results = [c.run(None) for c in cases]` 不变（计时已在各 case 内完成）。
- 结果收集后，若 `cfg.timing.enabled`：
  1. 初始化 `TimingDb(PROJ_ROOT / cfg.timing.db_path)`；
  2. 取 `git_short_commit()`；
  3. 逐个 `insert(result, run_id, ts_iso, git_commit)`；
  4. **先判定后写入**：对每个 `status == "pass"` 且 `secs > 0` 的 result，先查 `last_pass_secs`，调 `detect_regress` 回填 `result.timing_regress` / `result.timing_detail`；判定全部完成后，再批量 `insert` 本次全部 result。顺序见下方说明。
  5. 若有回归，控制台打印汇总告警（不影响退出码）。
- 新增子命令 `--timing-list CASE`：打印指定 case 的全部历史记录（ts / status / secs / git_commit），提前 `return 0`。

> **执行顺序**：固定为 **查 last_pass → 检测 → insert 本次**。若先 insert 再查，`last_pass_secs`（`SELECT secs FROM timing WHERE case_name=? AND status='pass' ORDER BY ts DESC LIMIT 1`）会命中刚写入的本次记录，导致 delta 恒为 0、永远不报回归。故必须先查后插。

## 6. 回归检测逻辑

```
对每个 result：
  若 result.status != "pass" 或 result.secs <= 0：跳过（求解未成功或无意义）
  last = db.last_pass_secs(result.name)   # 本次 insert 之前查
  regress, detail = detect_regress(result.secs, last, cfg.timing.regress_threshold)
  若 regress：result.timing_regress = True; result.timing_detail = detail
然后批量 insert 全部 result
```

`detect_regress` 判定：`last is None` 或 `last == 0` → 不判（首跑/无历史）；否则 `now > last * (1 + threshold)` → 回归，说明文本 `"上次 {last:.2f}s → 本次 {now:.2f}s (+{pct:.0f}%)"`。

## 7. 配置扩展（`test/config.toml` 追加）

```toml
[timing]
enabled = true
db_path = "test/timing.db"
regress_threshold = 0.05
```

> `db_path` 相对 `PROJ_ROOT` 解析。默认放在 `test/` 下（不在 `build/` 内，清理构建目录不会丢历史）。建议将 `*.db` 加入 `.gitignore`。

## 8. 子命令与入口

| 命令 | 行为 |
| --- | --- |
| `python test/run_tests.py`（不变） | 正常跑 + 自动计时/入库/回归检测 |
| `python test/run_tests.py --timing-list e2e.del2d1` | 打印该 case 历史，`return 0` |
| `--update`（不变） | 刷新基准，不触发 timing 流程 |

`--timing-list` 在 `argparse` 注册；命中时读 config、开 db、查询打印后直接 return，不构建、不跑 case。

## 9. 报告输出示例

```
======================================================================
CASE                          STATUS   METRIC                  TIME(s)
----------------------------------------------------------------------
e2e.del2d1                     pass     max|Δ|=1.0e-10          0.42
e2e.hel2d1                     pass     max|Δ|=8.3e-11          0.35
unit.core                      pass                             0.18
======================================================================
合计: 3 pass / 0 fail / 0 error / 0 skip
总耗时: 0.95s

⚠ 性能回归:
      e2e.del2d1  上次 0.28s → 本次 0.42s (+50%)
```

## 10. 测试计划

复用 `framework/tests/` pytest 套件，新增 `framework/tests/test_timing.py`：

- `test_init_creates_table`：临时 db，`init()` 后查 `sqlite_master` 确认表与索引存在。
- `test_insert_and_last_pass`：insert 两条（pass / fail），`last_pass_secs` 返回 pass 那条。
- `test_detect_regress_threshold`：`(1.0, 0.9, 0.2)` → 不回归（1.0 < 0.9×1.2=1.08）；`(1.3, 1.0, 0.2)` → 回归（1.3 > 1.0×1.2=1.2）；`(1.0, None, 0.2)` → 不判。
- `test_detect_regress_zero_last`：`last=0` → 不判（避免除零/误报）。
- `test_query_excludes_current_run`（或顺序约束）：验证"先查后插"顺序下，`last_pass_secs` 返回的是上一次而非本次。

`case.py` 计时改动通过现有 `test_case.py` 的 mock runner 回归（确认 `secs` 被正确回填、`_run_after_build` 逻辑等价于原内联逻辑）。

## 11. 非目标（YAGNI）

明确**不做**：
- 拆分编译/运行/解析多段耗时入库（用户已选最简粒度）。
- 性能基线冻结（`--timing-pin` 类）——首版用"上次 pass"。
- 历史趋势图 / 中位数 / 均值统计。
- 回归影响退出码（仅 WARN）。
- 跨机器对比（`git_commit` 仅作记录，不做机器维度区分）。

## 12. 验收标准

1. `python test/run_tests.py` 正常跑完，控制台报告含 `TIME(s)` 列与 `总耗时` 行。
2. `test/timing.db` 存在，`timing` 表有对应记录，字段完整。
3. 第二次运行，若某 pass case 耗时增长 >5%，报告中 `⚠ 性能回归` 区列出该 case；退出码仍为 0。
4. `python test/run_tests.py --timing-list e2e.del2d1` 打印历史记录且退出码 0。
5. `framework/tests/test_timing.py` 全部通过。
6. 现有 `framework/tests/` 全部用例不受影响（计时改动不破坏既有行为）。
