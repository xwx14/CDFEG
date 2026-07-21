"""timing.py 单元测试。"""
from pathlib import Path
import sqlite3
import sys
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.timing import detect_regress, git_short_commit, TimingDb
from framework.case import CaseResult


def _result(name, status, secs):
    return CaseResult(name=name, suite="e2e", status=status, secs=secs)


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
