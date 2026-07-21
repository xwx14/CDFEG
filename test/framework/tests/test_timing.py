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
