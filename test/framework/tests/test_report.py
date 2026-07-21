from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.case import CaseResult
from framework.report import aggregate, EXIT_OK, EXIT_FAIL, EXIT_ERROR


def _r(name, status):
    return CaseResult(name=name, suite="e2e", status=status)


def test_all_pass_returns_0():
    assert aggregate([_r("a", "pass"), _r("b", "pass")]) == EXIT_OK


def test_any_fail_returns_1():
    assert aggregate([_r("a", "pass"), _r("b", "fail")]) == EXIT_FAIL


def test_error_without_fail_returns_2():
    assert aggregate([_r("a", "error"), _r("b", "pass")]) == EXIT_ERROR


def test_fail_takes_precedence_over_error():
    assert aggregate([_r("a", "fail"), _r("b", "error")]) == EXIT_FAIL


def test_skip_ignored():
    assert aggregate([_r("a", "skip")]) == EXIT_OK


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
