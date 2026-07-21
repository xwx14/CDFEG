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
