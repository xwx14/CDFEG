from pathlib import Path
import sys
from unittest.mock import patch, call
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.builder import Builder


def test_build_caches_configured(tmp_path):
    b = Builder(source_dir="FEMproject", build_dir=str(tmp_path / "build"))
    with patch("framework.builder.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 0
        b.build(["del2d"])
        b.build(["del2d"])  # 第二次应只 build 不重复 configure
        cmds = [c.args[0] for c in mock_run.call_args_list]
        # configure 一次 + build 一次（第二次被 target 缓存跳过）
        assert sum("cmake" in c and "-B" in c for c in cmds) >= 1
        assert sum("--build" in c for c in cmds) == 1


def test_build_returns_exe_path(tmp_path):
    b = Builder(source_dir="FEMproject", build_dir=str(tmp_path / "build"))
    with patch("framework.builder.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 0
        exes = b.build(["del2d"])
        assert "del2d" in exes
        assert exes["del2d"].name.startswith("del2d")


def test_build_new_target_only_builds_missing(tmp_path):
    b = Builder(source_dir="FEMproject", build_dir=str(tmp_path / "build"))
    with patch("framework.builder.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 0
        b.build(["del2d"])
        b.build(["hel2d"])
        # 第二次只 build hel2d（del2d 已缓存）
        second = mock_run.call_args_list[-1].args[0]
        assert "hel2d" in second
        assert "del2d" not in second


def test_build_failure_raises(tmp_path):
    import pytest
    b = Builder(source_dir="FEMproject", build_dir=str(tmp_path / "build"))
    with patch("framework.builder.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 1
        with pytest.raises(RuntimeError):
            b.build(["del2d"])
