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
