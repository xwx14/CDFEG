# 回归测试时间统计 + SQLite 持久化 + 性能回归检测 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `test/` 回归框架增加每用例耗时统计、SQLite 持久化与性能回归检测（与上次 pass 对比，超 5% 阈值告警，仅 WARN 不影响退出码）。

**Architecture:** 新增 `framework/timing.py`（纯标准库 `sqlite3`）封装持久化与回归判定；`CaseResult` 增加 `secs/timing_regress/timing_detail` 字段；各 `Case.run()` 用 `time.perf_counter()` 在 `build` 完成后计时；`run_tests.py` 按"先查上次 → 检测 → 写入"顺序串联；`report.py` 增加 `TIME(s)` 列、总耗时行与回归详情区。

**Tech Stack:** Python 3.11+（`tomllib`，低版本 `tomli`）、标准库 `sqlite3`/`subprocess`/`time`/`datetime`、pytest。无新第三方依赖。仅改动 `test/`，不动 C++。

## Global Constraints

（来自 spec，每个任务的隐含前提）
- **计时口径**：`secs` = `build` 完成后的 `run + parse + compare` wall-clock（不含编译）；`build` 耗时仅在 verbose（环境变量 `CDFEG_TEST_VERBOSE=1`）时控制台打印，不入库。
- **执行顺序**：性能回归判定必须 **先查 `last_pass` → 检测 → 再 insert**，否则会查到本次自身导致 delta 恒为 0。
- **回归不影响退出码**：仅置 `timing_regress=True` 并在报告打印 `⚠`；退出码仍由精度 pass/fail/error 决定。
- **默认值**：`db_path = "test/timing.db"`（相对 `PROJ_ROOT` 解析），`regress_threshold = 0.05`，`enabled = true`。
- **命名**：驼峰命名（项目约定）；commit message 用中文。
- **回归检测前置条件**：仅当本次 `status == "pass"` 且 `secs > 0` 才检测；`last` 为 `None` 或 `<= 0` 时不判。
- **C++ 无改动**：本计划全部落在 `test/` Python 代码。

---

## File Structure

| 文件 | 责任 | 改动类型 |
| --- | --- | --- |
| `test/framework/config.py` | 解析 `[timing]` 段，提供 `Config.timing: TimingConfig` | 修改 |
| `test/config.toml` | 追加 `[timing]` 段 | 修改 |
| `test/framework/timing.py` | `detect_regress` / `git_short_commit` / `TimingDb` | 新建 |
| `test/framework/case.py` | `CaseResult` 加计时字段；各 `Case.run()` 计时 | 修改 |
| `test/framework/report.py` | 报告加时间列/总耗时/回归区 | 修改 |
| `test/run_tests.py` | `persist_timing` / `format_timing_list` / `main` 串联 / `--timing-list` | 修改 |
| `test/framework/tests/test_timing.py` | `timing.py` 单测 | 新建 |
| `test/framework/tests/test_config.py` | `[timing]` 解析单测（追加） | 修改 |
| `test/framework/tests/test_case.py` | `secs` 回填补充断言（追加） | 修改 |
| `test/framework/tests/test_report.py` | 时间列/总耗时/回归区断言（追加） | 修改 |
| `test/framework/tests/test_run_tests.py` | `persist_timing` / `format_timing_list` 单测 | 新建 |

任务依赖：1 → 2 → 3 → 4 → 5 → 6（`CaseResult` 字段在 3 定义，`TimingDb.insert` 在 4 消费它；`report` 在 5 消费字段；`run_tests` 在 6 串联全部）。

---

## Task 1: config.py 解析 `[timing]` 段

**Files:**
- Modify: `test/framework/config.py`（`Toolchain` 后新增 `TimingConfig`；`Config.__init__` 末尾追加解析）
- Modify: `test/config.toml`（文件末尾追加 `[timing]` 段）
- Test: `test/framework/tests/test_config.py`（追加 2 个用例）

**Interfaces:**
- Produces: `TimingConfig(enabled: bool, db_path: str, regress_threshold: float)`；`Config.timing` 属性。

- [ ] **Step 1: 追加失败测试到 `test/framework/tests/test_config.py`**

在文件末尾追加：

```python
def test_load_config_reads_timing(tmp_path):
    cfg_file = tmp_path / "config.toml"
    cfg_file.write_text(
        '[timing]\n'
        'enabled = true\n'
        'db_path = "test/timing.db"\n'
        'regress_threshold = 0.05\n',
        encoding="utf-8",
    )
    cfg = load_config(cfg_file)
    assert cfg.timing.enabled is True
    assert cfg.timing.db_path == "test/timing.db"
    assert cfg.timing.regress_threshold == 0.05


def test_load_config_timing_defaults_when_absent(tmp_path):
    cfg_file = tmp_path / "config.toml"
    cfg_file.write_text('[toolchain]\n', encoding="utf-8")
    cfg = load_config(cfg_file)
    assert cfg.timing.enabled is True
    assert cfg.timing.db_path == "test/timing.db"
    assert cfg.timing.regress_threshold == 0.05
```

- [ ] **Step 2: 运行测试确认失败**

Run: `python -m pytest test/framework/tests/test_config.py::test_load_config_reads_timing test/framework/tests/test_config.py::test_load_config_timing_defaults_when_absent -v`
Expected: FAIL（`AttributeError: 'Config' object has no attribute 'timing'`）

- [ ] **Step 3: 在 `test/framework/config.py` 实现 `TimingConfig`**

在 `Toolchain` dataclass 之后、`class Config` 之前插入：

```python
@dataclass
class TimingConfig:
    enabled: bool = True
    db_path: str = "test/timing.db"
    regress_threshold: float = 0.05
```

在 `Config.__init__` 末尾（`self.toolchain = ...` 赋值之后）追加：

```python
        tm = raw.get("timing", {})
        self.timing = TimingConfig(
            enabled=tm.get("enabled", True),
            db_path=tm.get("db_path", "test/timing.db"),
            regress_threshold=float(tm.get("regress_threshold", 0.05)),
        )
```

- [ ] **Step 4: 在 `test/config.toml` 末尾追加配置段**

```toml

[timing]
enabled = true
db_path = "test/timing.db"
regress_threshold = 0.05
```

- [ ] **Step 5: 运行测试确认通过**

Run: `python -m pytest test/framework/tests/test_config.py -v`
Expected: PASS（全部用例，含原有 3 个）

- [ ] **Step 6: 提交**

```bash
git add test/framework/config.py test/config.toml test/framework/tests/test_config.py
git commit -m "feat(test): config 解析 [timing] 段（TimingConfig + 默认值）"
```

---

## Task 2: timing.py 纯函数（detect_regress + git_short_commit）

**Files:**
- Create: `test/framework/timing.py`
- Test: `test/framework/tests/test_timing.py`（新建，本任务先写纯函数部分）

**Interfaces:**
- Produces: `detect_regress(now: float, last: float | None, threshold: float) -> tuple[bool, str]`；`git_short_commit() -> str | None`。

- [ ] **Step 1: 新建 `test/framework/tests/test_timing.py` 写失败测试**

```python
"""timing.py 单元测试。"""
from pathlib import Path
import sys
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.timing import detect_regress, git_short_commit


def test_detect_regress_no_history():
    assert detect_regress(1.0, None, 0.05) == (False, "")


def test_detect_regress_zero_last():
    assert detect_regress(1.0, 0.0, 0.05) == (False, "")


def test_detect_regress_within_threshold():
    # boundary = 1.0 * 1.05 = 1.05；1.04 < 1.05 不回归
    reg, _ = detect_regress(1.04, 1.0, 0.05)
    assert reg is False


def test_detect_regress_over_threshold():
    # 1.1 > 1.05 回归
    reg, detail = detect_regress(1.1, 1.0, 0.05)
    assert reg is True
    assert "1.00s" in detail
    assert "1.10s" in detail
    assert "+" in detail


def test_detect_regress_faster_not_regress():
    reg, _ = detect_regress(0.5, 1.0, 0.05)
    assert reg is False


def test_git_short_commit_returns_sha():
    with patch("framework.timing.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 0
        mock_run.return_value.stdout = "abc1234\n"
        assert git_short_commit() == "abc1234"


def test_git_short_commit_none_on_failure():
    with patch("framework.timing.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 128
        mock_run.return_value.stdout = ""
        assert git_short_commit() is None
```

- [ ] **Step 2: 运行测试确认失败**

Run: `python -m pytest test/framework/tests/test_timing.py -v`
Expected: FAIL（`ModuleNotFoundError: No module named 'framework.timing'`）

- [ ] **Step 3: 新建 `test/framework/timing.py` 实现纯函数**

```python
"""回归测试耗时统计：SQLite 持久化 + 性能回归检测。纯标准库 sqlite3。"""
import subprocess


def detect_regress(now, last, threshold):
    """判定性能回归。

    last 为 None 或 <=0 时不判（首跑/无历史），返回 (False, "")。
    否则当 now > last*(1+threshold) 时判为回归，返回 (True, 说明文本)。
    """
    if last is None or last <= 0:
        return (False, "")
    boundary = last * (1.0 + threshold)
    if now > boundary:
        pct = (now - last) / last * 100.0
        return (True, f"上次 {last:.2f}s → 本次 {now:.2f}s (+{pct:.0f}%)")
    return (False, "")


def git_short_commit():
    """返回 git HEAD short sha；取不到（非 git 仓库/无 git/失败）返回 None。"""
    try:
        r = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            capture_output=True, text=True, timeout=5,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return None
    if r.returncode != 0:
        return None
    sha = r.stdout.strip()
    return sha or None
```

- [ ] **Step 4: 运行测试确认通过**

Run: `python -m pytest test/framework/tests/test_timing.py -v`
Expected: PASS（7 个用例）

- [ ] **Step 5: 提交**

```bash
git add test/framework/timing.py test/framework/tests/test_timing.py
git commit -m "feat(test): timing 新增 detect_regress 与 git_short_commit"
```

---

## Task 3: case.py 给 CaseResult 加计时字段并在各 run 计时

**Files:**
- Modify: `test/framework/case.py`（`CaseResult` 加 3 字段；`E2ECase.run` 拆 `_run_after_build`；`UnitCase.run` / `GeneratorCase.run` 计时）
- Test: `test/framework/tests/test_case.py`（追加 secs 断言用例）

**Interfaces:**
- Produces: `CaseResult.secs: float`、`CaseResult.timing_regress: bool`、`CaseResult.timing_detail: str`；各 `run()` 返回的 `CaseResult` 已回填 `secs`。
- Consumes: 无（本任务独立）。

- [ ] **Step 1: 追加失败测试到 `test/framework/tests/test_case.py`**

文件顶部 `import` 区追加 `import time`（若未有）。在文件末尾追加：

```python
def test_e2e_records_secs_on_pass(tmp_path):
    case_dir = _setup_case_dir(tmp_path)
    _, case = _make_case(case_dir, tmp_path)

    def fake_run(exe, args, cwd, expect, timeout=600, **kwargs):
        out = Path(cwd) / "del2d.post.res"
        shutil.copy(FIXTURE, out)
        return RunResult(0, "", "", {"del2d.post.res": out})

    case._runner_run = fake_run
    r = case.run(None)
    assert r.status == "pass"
    assert r.secs >= 0.0  # 计时已回填


def test_e2e_secs_zero_when_build_fails(tmp_path):
    """build 失败时不应计时（secs 保持 0）。"""
    case_dir = _setup_case_dir(tmp_path)
    builder, case = _make_case(case_dir, tmp_path)
    builder.build.side_effect = RuntimeError("cmake 失败")
    r = case.run(None)
    assert r.status == "error"
    assert r.secs == 0.0
```

并在原有 `test_e2e_pass_when_identical` 末尾的断言后追加一行：

```python
    assert r.secs >= 0.0
```

- [ ] **Step 2: 运行测试确认失败**

Run: `python -m pytest test/framework/tests/test_case.py -v`
Expected: FAIL（`AttributeError: 'CaseResult' object has no attribute 'secs'`）

- [ ] **Step 3: 修改 `test/framework/case.py` 的 `CaseResult` 加字段**

```python
@dataclass
class CaseResult:
    name: str          # "e2e.del2d1"
    suite: str         # "e2e" | "unit" | "generator" | "analytical"
    status: str        # "pass" | "fail" | "error" | "skip"
    metric: dict = field(default_factory=dict)
    detail: str = ""
    secs: float = 0.0              # 用例耗时（build 之后，秒）；skip/error-on-build 为 0
    timing_regress: bool = False   # 是否触发性能回归（由 run_tests 检测后回填）
    timing_detail: str = ""        # 回归说明文本
```

在 `case.py` 顶部 import 区追加（若未有）：

```python
import time
```

- [ ] **Step 4: 改 `E2ECase`——拆出 `_run_after_build`，`run` 包计时**

将 `E2ECase.run` 替换为下面两个方法（`_run_after_build` 承载原 `run` 中 build 之后的全部逻辑，逐字保留）：

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

    def _run_after_build(self, exes) -> CaseResult:
        work_dir = self._prepare_work_dir()
        rr: RunResult = self._runner_run(exes[self.target], [self.project, "."], work_dir,
                                         [self.output], timeout=self.timeout,
                                         extra_dll_dirs=self.dll_dirs)
        if rr.timed_out:
            return CaseResult(self.name, self.suite, "error", detail=f"运行超时({self.timeout}s)")
        if rr.returncode != 0:
            return CaseResult(self.name, self.suite, "error",
                              detail=f"退出码 {rr.returncode}: {rr.stderr[:200]}")
        if self.output not in rr.outputs:
            return CaseResult(self.name, self.suite, "error",
                              detail=f"未产出 {self.output}")
        actual = self._parse(rr.outputs[self.output])
        baseline_path = self.case_dir / self.baseline
        if not baseline_path.exists():
            return CaseResult(self.name, self.suite, "error",
                              detail=f"基准缺失: {baseline_path}")
        baseline = self._parse(baseline_path)
        cr = compare(actual, baseline, self.tol)
        if not cr.structural_ok:
            return CaseResult(self.name, self.suite, "fail",
                              detail="结构漂移: " + "; ".join(cr.structural_errors))
        metric = {"max_abs_err": cr.max_abs_err, "n_points": cr.n_points, "n_over_tol": cr.n_over_tol}
        if cr.n_over_tol > 0:
            detail = (f"max|Δ|={cr.worst_delta:.2e} 超差 {cr.n_over_tol}/{cr.n_points}; "
                      f"首超 {cr.first_over}")
            return CaseResult(self.name, self.suite, "fail", metric=metric, detail=detail)
        return CaseResult(self.name, self.suite, "pass", metric=metric)
```

- [ ] **Step 5: 改 `UnitCase.run` 包计时**

```python
    def run(self, ctx) -> CaseResult:
        try:
            exes = self.builder.build([self.binary])
        except Exception as e:
            return CaseResult(self.name, self.suite, "error", detail=f"构建失败: {e}")
        t0 = time.perf_counter()
        rr = _runner_run_default(exes[self.binary], [], Path("."), [], timeout=self.timeout,
                                 extra_dll_dirs=self.dll_dirs)
        if rr.returncode == 0:
            result = CaseResult(self.name, self.suite, "pass",
                                detail=rr.stdout[-200:] if rr.stdout else "")
        else:
            result = CaseResult(self.name, self.suite, "fail",
                                detail=f"Catch2 断言失败:\n{(rr.stdout or '')[:500]}")
        result.secs = time.perf_counter() - t0
        return result
```

- [ ] **Step 6: 改 `GeneratorCase.run` 包计时**

```python
    def run(self, ctx) -> CaseResult:
        import subprocess
        t0 = time.perf_counter()
        r = subprocess.run(["python", "-m", "pytest", str(self.pytest_dir), "-v"],
                           capture_output=True, text=True)
        if r.returncode == 0:
            result = CaseResult(self.name, self.suite, "pass")
        else:
            result = CaseResult(self.name, self.suite, "fail", detail=r.stdout[:500])
        result.secs = time.perf_counter() - t0
        return result
```

- [ ] **Step 7: 运行测试确认通过**

Run: `python -m pytest test/framework/tests/test_case.py -v`
Expected: PASS（含原有 4 个 + 新增 2 个）

- [ ] **Step 8: 回归确认 report/config 测试不受影响**

Run: `python -m pytest test/framework/tests/ -v`
Expected: PASS（全部已有用例；`secs` 有默认值，`test_report.py` 的 `_r` helper 仍可用）

- [ ] **Step 9: 提交**

```bash
git add test/framework/case.py test/framework/tests/test_case.py
git commit -m "feat(test): CaseResult 记录用例耗时（build 之后），E2ECase 拆 _run_after_build"
```

---

## Task 4: timing.py 的 TimingDb（init/insert/last_pass_secs/list_history）

**Files:**
- Modify: `test/framework/timing.py`（追加 `TimingDb` 类）
- Test: `test/framework/tests/test_timing.py`（追加 TimingDb 用例）

**Interfaces:**
- Consumes: `CaseResult`（`Task 3` 产出，`insert` 读取 `.name/.suite/.status/.secs`）。
- Produces: `TimingDb(db_path)`，方法 `init()`、`insert(result, run_id, ts, git_commit)`、`last_pass_secs(case_name) -> float | None`、`list_history(case_name) -> list[tuple]`。

- [ ] **Step 1: 追加失败测试到 `test/framework/tests/test_timing.py`**

文件顶部 import 区追加：

```python
import sqlite3
from framework.timing import TimingDb
from framework.case import CaseResult


def _result(name, status, secs):
    return CaseResult(name=name, suite="e2e", status=status, secs=secs)
```

文件末尾追加：

```python
def test_timingdb_init_creates_table_and_index(tmp_path):
    db = TimingDb(tmp_path / "t.db")
    db.init()
    conn = sqlite3.connect(tmp_path / "t.db")
    try:
        tables = conn.execute(
            "SELECT name FROM sqlite_master WHERE type='table'").fetchall()
        idxs = conn.execute(
            "SELECT name FROM sqlite_master WHERE type='index'").fetchall()
    finally:
        conn.close()
    assert ("timing",) in tables
    assert ("idx_case_ts",) in idxs


def test_timingdb_init_is_idempotent(tmp_path):
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.init()  # 二次调用不应报错


def test_timingdb_insert_and_last_pass(tmp_path):
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.insert(_result("e2e.a", "pass", 1.0), "r1", "2026-01-01T00:00:00", "abc")
    db.insert(_result("e2e.a", "fail", 2.0), "r1", "2026-01-01T00:00:01", "abc")
    db.insert(_result("e2e.a", "pass", 1.5), "r2", "2026-01-02T00:00:00", "def")
    assert db.last_pass_secs("e2e.a") == 1.5  # 最近一次 pass


def test_timingdb_last_pass_none_when_no_pass(tmp_path):
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.insert(_result("e2e.a", "fail", 1.0), "r1", "2026-01-01T00:00:00", None)
    assert db.last_pass_secs("e2e.a") is None


def test_timingdb_last_pass_filters_other_case(tmp_path):
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.insert(_result("e2e.a", "pass", 1.0), "r1", "2026-01-01T00:00:00", None)
    db.insert(_result("e2e.b", "pass", 9.0), "r1", "2026-01-01T00:00:01", None)
    assert db.last_pass_secs("e2e.a") == 1.0


def test_timingdb_list_history_newest_first(tmp_path):
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.insert(_result("e2e.a", "pass", 1.0), "r1", "2026-01-01T00:00:00", "a")
    db.insert(_result("e2e.a", "pass", 1.5), "r2", "2026-01-02T00:00:00", "b")
    rows = db.list_history("e2e.a")
    assert len(rows) == 2
    assert rows[0][2] == 1.5  # (ts, status, secs, git_commit)；最新在前
    assert rows[1][2] == 1.0


def test_timingdb_creates_parent_dir(tmp_path):
    """db_path 在不存在的子目录时应自动创建。"""
    db = TimingDb(tmp_path / "sub" / "deep" / "t.db")
    db.init()
    assert (tmp_path / "sub" / "deep" / "t.db").exists()
```

- [ ] **Step 2: 运行测试确认失败**

Run: `python -m pytest test/framework/tests/test_timing.py -v`
Expected: FAIL（`ImportError: cannot import name 'TimingDb'`）

- [ ] **Step 3: 在 `test/framework/timing.py` 追加 `TimingDb`**

文件顶部 import 区追加：

```python
import sqlite3
from pathlib import Path
```

文件末尾追加：

```python
class TimingDb:
    """timing 记录的 SQLite 持久化。纯标准库 sqlite3。"""

    def __init__(self, db_path):
        self.db_path = str(db_path)

    def init(self):
        Path(self.db_path).parent.mkdir(parents=True, exist_ok=True)
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                "CREATE TABLE IF NOT EXISTS timing ("
                " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                " run_id TEXT NOT NULL,"
                " ts TEXT NOT NULL,"
                " case_name TEXT NOT NULL,"
                " suite TEXT NOT NULL,"
                " status TEXT NOT NULL,"
                " secs REAL NOT NULL,"
                " git_commit TEXT)"
            )
            conn.execute(
                "CREATE INDEX IF NOT EXISTS idx_case_ts ON timing(case_name, ts)"
            )

    def insert(self, result, run_id, ts, git_commit):
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                "INSERT INTO timing (run_id, ts, case_name, suite, status, secs, git_commit)"
                " VALUES (?, ?, ?, ?, ?, ?, ?)",
                (run_id, ts, result.name, result.suite, result.status,
                 result.secs, git_commit),
            )

    def last_pass_secs(self, case_name):
        """同 case 最近一条 status='pass' 的 secs；无则 None。按 id DESC 取最新。"""
        with sqlite3.connect(self.db_path) as conn:
            row = conn.execute(
                "SELECT secs FROM timing WHERE case_name=? AND status='pass'"
                " ORDER BY id DESC LIMIT 1",
                (case_name,),
            ).fetchone()
        return row[0] if row else None

    def list_history(self, case_name):
        """返回 [(ts, status, secs, git_commit), ...]，按 id DESC（最新在前）。"""
        with sqlite3.connect(self.db_path) as conn:
            return conn.execute(
                "SELECT ts, status, secs, git_commit FROM timing"
                " WHERE case_name=? ORDER BY id DESC",
                (case_name,),
            ).fetchall()
```

- [ ] **Step 4: 运行测试确认通过**

Run: `python -m pytest test/framework/tests/test_timing.py -v`
Expected: PASS（7 个纯函数 + 7 个 TimingDb = 14 个）

- [ ] **Step 5: 提交**

```bash
git add test/framework/timing.py test/framework/tests/test_timing.py
git commit -m "feat(test): TimingDb SQLite 持久化（init/insert/last_pass_secs/list_history）"
```

---

## Task 5: report.py 报告加 TIME 列 / 总耗时 / 性能回归区

**Files:**
- Modify: `test/framework/report.py`（`print_report`）
- Test: `test/framework/tests/test_report.py`（追加 capsys 用例）

**Interfaces:**
- Consumes: `CaseResult.secs/.timing_regress/.timing_detail`（`Task 3` 产出）。
- Produces: 无新接口（仅控制台输出变化）。

- [ ] **Step 1: 追加失败测试到 `test/framework/tests/test_report.py`**

文件顶部 import 已有 `CaseResult`、`aggregate`。末尾追加：

```python
def test_report_shows_time_column_and_total(capsys):
    r1 = CaseResult(name="e2e.a", suite="e2e", status="pass",
                    metric={"max_abs_err": 1e-10}, secs=0.42)
    r2 = CaseResult(name="unit.b", suite="unit", status="pass", secs=0.18)
    aggregate([r1, r2])
    out = capsys.readouterr().out
    assert "TIME(s)" in out
    assert "0.42" in out
    assert "0.18" in out
    assert "总耗时" in out
    assert "0.60s" in out


def test_report_skips_time_for_zero_secs(capsys):
    r = CaseResult(name="e2e.c", suite="e2e", status="skip", secs=0.0)
    aggregate([r])
    out = capsys.readouterr().out
    assert "TIME(s)" in out
    # secs=0 时 time_str 留空，数据行不应出现 "0.00"
    data_lines = [ln for ln in out.splitlines()
                  if "e2e.c" in ln and "TIME" not in ln]
    assert data_lines
    assert "0.00" not in data_lines[0]


def test_report_shows_regress_section(capsys):
    r = CaseResult(name="e2e.a", suite="e2e", status="pass", secs=1.5,
                   timing_regress=True,
                   timing_detail="上次 1.00s → 本次 1.50s (+50%)")
    aggregate([r])
    out = capsys.readouterr().out
    assert "性能回归" in out
    assert "上次 1.00s" in out
    # 回归不影响退出码
    assert aggregate([r]) == EXIT_OK
```

- [ ] **Step 2: 运行测试确认失败**

Run: `python -m pytest test/framework/tests/test_report.py -v`
Expected: FAIL（`AssertionError`：输出中无 `TIME(s)` / `总耗时` / `性能回归`）

- [ ] **Step 3: 改 `test/framework/report.py` 的 `print_report`**

将整个 `print_report` 函数替换为：

```python
def print_report(results: list[CaseResult]):
    print("\n" + "=" * 70)
    print(f"{'CASE':<28} {'STATUS':<8} {'METRIC':<22} {'TIME(s)'}")
    print("-" * 70)
    for r in results:
        metric_str = ""
        if r.status == "pass" and r.metric.get("max_abs_err") is not None:
            metric_str = f"max|Δ|={r.metric['max_abs_err']:.2e}"
        elif r.status in ("fail", "error"):
            metric_str = r.detail[:20]
        time_str = f"{r.secs:.2f}" if r.secs > 0 else ""
        print(f"{r.name:<28} {r.status:<8} {metric_str:<22} {time_str}")
    print("=" * 70)
    n_pass = sum(1 for r in results if r.status == "pass")
    n_fail = sum(1 for r in results if r.status == "fail")
    n_err = sum(1 for r in results if r.status == "error")
    n_skip = sum(1 for r in results if r.status == "skip")
    total_secs = sum(r.secs for r in results)
    print(f"合计: {n_pass} pass / {n_fail} fail / {n_err} error / {n_skip} skip")
    print(f"总耗时: {total_secs:.2f}s")
    # 失败详情
    for r in results:
        if r.status in ("fail", "error") and r.detail:
            print(f"\n[{r.status.upper()}] {r.name}\n      {r.detail}")
    # 性能回归（仅告警，不影响退出码）
    regress = [r for r in results if r.timing_regress]
    if regress:
        print("\n⚠ 性能回归:")
        for r in regress:
            print(f"      {r.name}  {r.timing_detail}")
```

- [ ] **Step 4: 运行测试确认通过**

Run: `python -m pytest test/framework/tests/test_report.py -v`
Expected: PASS（含原有 5 个退出码用例 + 新增 3 个）

- [ ] **Step 5: 提交**

```bash
git add test/framework/report.py test/framework/tests/test_report.py
git commit -m "feat(test): 报告加 TIME 列/总耗时/性能回归区"
```

---

## Task 6: run_tests.py 串联入库 + 回归检测 + --timing-list

**Files:**
- Modify: `test/run_tests.py`（加 `persist_timing` / `format_timing_list`；改 `main`；注册 `--timing-list`）
- Test: `test/framework/tests/test_run_tests.py`（新建）

**Interfaces:**
- Consumes: `TimingDb`（Task 4）、`detect_regress` / `git_short_commit`（Task 2）、`Config.timing`（Task 1）、`CaseResult` 字段（Task 3）。
- Produces: `persist_timing(db, results, threshold, run_id, ts, git_commit)`、`format_timing_list(case_name, rows)`、CLI `--timing-list CASE`。

- [ ] **Step 1: 新建 `test/framework/tests/test_run_tests.py` 写失败测试**

```python
"""run_tests.py 中可单测的纯逻辑（persist_timing / format_timing_list）。"""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.timing import TimingDb
from framework.case import CaseResult
from run_tests import persist_timing, format_timing_list


def test_persist_timing_detects_regress(tmp_path):
    """守护"先查后插"顺序：上次 1.0、本次 1.5 应判回归。
    若实现误为"先插后查"，last 会查到本次自身 1.5，boundary=1.575，
    1.5 不超 → timing_regress=False，本用例失败。"""
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.insert(CaseResult(name="e2e.a", suite="e2e", status="pass", secs=1.0),
              run_id="old", ts="2026-01-01T00:00:00", git_commit=None)
    results = [CaseResult(name="e2e.a", suite="e2e", status="pass", secs=1.5)]
    persist_timing(db, results, threshold=0.05,
                   run_id="new", ts="2026-01-02T00:00:00", git_commit="abc")
    assert results[0].timing_regress is True
    assert "1.50s" in results[0].timing_detail
    # 本次记录已写入
    assert db.last_pass_secs("e2e.a") == 1.5


def test_persist_timing_skips_fail(tmp_path):
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.insert(CaseResult(name="e2e.a", suite="e2e", status="pass", secs=1.0),
              run_id="old", ts="2026-01-01T00:00:00", git_commit=None)
    results = [CaseResult(name="e2e.a", suite="e2e", status="fail", secs=99.0)]
    persist_timing(db, results, threshold=0.05,
                   run_id="new", ts="2026-01-02T00:00:00", git_commit=None)
    assert results[0].timing_regress is False  # fail 不检测


def test_persist_timing_equal_secs_not_regress(tmp_path):
    """本次与上次耗时相等时不应判回归（边界 now==last 不超阈值）。"""
    db = TimingDb(tmp_path / "t.db")
    db.init()
    db.insert(CaseResult(name="e2e.a", suite="e2e", status="pass", secs=1.0),
              run_id="old", ts="2026-01-01T00:00:00", git_commit=None)
    results = [CaseResult(name="e2e.a", suite="e2e", status="pass", secs=1.0)]
    persist_timing(db, results, threshold=0.05,
                   run_id="new", ts="2026-01-02T00:00:00", git_commit=None)
    assert results[0].timing_regress is False


def test_format_timing_list_renders_rows():
    rows = [
        ("2026-01-02T00:00:00", "pass", 1.50, "abc1234"),
        ("2026-01-01T00:00:00", "fail", 2.0, None),
    ]
    out = format_timing_list("e2e.a", rows)
    assert "e2e.a" in out
    assert "2026-01-02" in out
    assert "pass" in out
    assert "1.50" in out
    assert "abc1234" in out


def test_format_timing_list_empty():
    out = format_timing_list("e2e.a", [])
    assert "e2e.a" in out
    assert "无记录" in out
```

- [ ] **Step 2: 运行测试确认失败**

Run: `python -m pytest test/framework/tests/test_run_tests.py -v`
Expected: FAIL（`ImportError: cannot import name 'persist_timing' from 'run_tests'`）

- [ ] **Step 3: 在 `test/run_tests.py` 顶部 import 区追加**

```python
import time
from datetime import datetime
from framework.timing import TimingDb, detect_regress, git_short_commit
```

- [ ] **Step 4: 在 `test/run_tests.py` 的 `main` 函数之前追加两个函数**

```python
def persist_timing(db, results, threshold, run_id, ts, git_commit):
    """先查上次 pass → 回归检测 → 批量写入。顺序固定，避免查到本次自身。"""
    for r in results:
        if r.status == "pass" and r.secs > 0:
            last = db.last_pass_secs(r.name)
            reg, detail = detect_regress(r.secs, last, threshold)
            if reg:
                r.timing_regress = True
                r.timing_detail = detail
    for r in results:
        db.insert(r, run_id=run_id, ts=ts, git_commit=git_commit)


def format_timing_list(case_name, rows):
    """rows: iterable of (ts, status, secs, git_commit)。"""
    lines = [f"=== {case_name} 历史耗时 ==="]
    rows = list(rows)
    if not rows:
        lines.append("（无记录）")
        return "\n".join(lines)
    lines.append(f"{'TS':<22} {'STATUS':<8} {'SECS':<8} GIT")
    for ts, status, secs, git_commit in rows:
        lines.append(f"{ts:<22} {status:<8} {secs:<8.2f} {git_commit or ''}")
    return "\n".join(lines)
```

- [ ] **Step 5: 改 `test/run_tests.py` 的 `main`——注册 `--timing-list` 并串联入库**

将 `main` 中 `ap.add_argument("--update", ...)` 之后追加一行：

```python
    ap.add_argument("--timing-list", default=None, help="列出指定用例的历史耗时记录")
```

将 `main` 中 `cfg = load_config(...)` 之后、`if args.update:` 之前插入 `--timing-list` 分支：

```python
    if args.timing_list:
        db = TimingDb(PROJ_ROOT / cfg.timing.db_path)
        db.init()
        rows = db.list_history(args.timing_list)
        print(format_timing_list(args.timing_list, rows))
        return 0
```

将 `main` 末尾的 `results = [c.run(None) for c in cases]` 与 `return aggregate(results)` 之间插入入库串联：

```python
    results = [c.run(None) for c in cases]

    if cfg.timing.enabled:
        run_id = time.strftime("%Y%m%d_%H%M%S")
        ts = datetime.now().isoformat(timespec="seconds")
        commit = git_short_commit()
        db = TimingDb(PROJ_ROOT / cfg.timing.db_path)
        db.init()
        persist_timing(db, results, cfg.timing.regress_threshold,
                       run_id=run_id, ts=ts, git_commit=commit)

    return aggregate(results)
```

- [ ] **Step 6: 运行单测确认通过**

Run: `python -m pytest test/framework/tests/test_run_tests.py -v`
Expected: PASS（5 个用例）

- [ ] **Step 7: 全量单测回归**

Run: `python -m pytest test/framework/tests/ -v`
Expected: PASS（config/timing/case/report/run_tests 全部）

- [ ] **Step 8: 端到端验证——跑单个 e2e 用例确认 db 生成与报告**

Run: `python test/run_tests.py --suite e2e --case del2d1`
Expected: 控制台报告含 `TIME(s)` 列、该行有数值、末尾 `总耗时: X.XXs`；进程退出码 0；生成 `test/timing.db`。

确认 db 有记录（Bash）：
```bash
python -c "import sqlite3; c=sqlite3.connect('test/timing.db'); print(c.execute('SELECT case_name,status,secs FROM timing').fetchall())"
```
Expected: 输出含 `('e2e.del2d1', 'pass', <正数>)`。

- [ ] **Step 9: 端到端验证——--timing-list 与回归告警**

Run: `python test/run_tests.py --timing-list e2e.del2d1`
Expected: 打印历史记录表（至少 1 行），退出码 0。

再跑一次同一用例确认正常写入（无回归时应无 `⚠`）：
Run: `python test/run_tests.py --suite e2e --case del2d1`
Expected: 报告无 `性能回归` 区（耗时波动通常 < 5%）。

- [ ] **Step 10: 提交**

```bash
git add test/run_tests.py test/framework/tests/test_run_tests.py
git commit -m "feat(test): run_tests 串联耗时入库+回归检测，新增 --timing-list"
```

---

## Self-Review 记录

**Spec coverage（逐节核对）：**
- §2 决策表 → Global Constraints 全部落地（口径/阈值/db_path/退出码/顺序）。
- §4.1 CaseResult 三字段 → Task 3 Step 3。
- §4.2 SQLite schema → Task 4 Step 3（表/索引/字段一致，`id DESC` 取代 `ts DESC` 更稳，行为等价）。
- §5.1 timing.py 三函数 + 一类 → Task 2 + Task 4。
- §5.2 config TimingConfig → Task 1。
- §5.3 各 Case 计时 → Task 3（E2ECase/UnitCase/GeneratorCase；Analytical/Skip 默认 0）。
- §5.4 report 三项 → Task 5。
- §5.5 run_tests 串联 + --timing-list → Task 6（`persist_timing` 内含"先查后插"顺序）。
- §6 回归检测逻辑 → Task 6 `persist_timing`。
- §7 config.toml [timing] → Task 1 Step 4。
- §8 子命令表 → Task 6 Step 5。
- §9 报告示例 → Task 5 输出格式一致。
- §10 测试计划 → test_timing/test_config/test_case/test_report/test_run_tests 全覆盖（含 `last=0` 不判、先查后插不查自身）。
- §12 验收标准 1-6 → Task 5/6 的端到端验证 + 全量单测覆盖。

**Placeholder scan：** 无 TBD/TODO；每个代码步骤均含完整代码。

**Type consistency：** `CaseResult.secs/timing_regress/timing_detail` 在 Task 3 定义，Task 4（insert 读取 `.name/.suite/.status/.secs`）、Task 5（读取三字段）、Task 6（回填三字段）用法一致；`TimingDb.insert(result, run_id, ts, git_commit)` 签名在 Task 4 定义、Task 6 调用一致；`detect_regress(now, last, threshold) -> (bool, str)` 在 Task 2 定义、Task 6 调用一致；`persist_timing(db, results, threshold, run_id, ts, git_commit)` 在 Task 6 定义与测试一致。
