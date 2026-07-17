"""runner.py 单元测试（mock subprocess + 真实临时文件）。"""
from pathlib import Path
import sys
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.runner import run


def test_run_collects_existing_output(tmp_path):
    # 模拟 exe 成功 + 产出文件存在
    out_file = tmp_path / "del2d.post.res"
    out_file.write_text("dummy", encoding="utf-8")
    with patch("framework.runner.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 0
        mock_run.return_value.stdout = ""
        mock_run.return_value.stderr = ""
        r = run("del2d.exe", ["del2d", "."], tmp_path, ["del2d.post.res"])
        assert r.returncode == 0
        assert "del2d.post.res" in r.outputs
        assert r.outputs["del2d.post.res"].exists()


def test_run_missing_output_not_collected(tmp_path):
    with patch("framework.runner.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 0
        mock_run.return_value.stdout = ""
        mock_run.return_value.stderr = ""
        r = run("del2d.exe", ["del2d", "."], tmp_path, ["del2d.post.res"])
        assert "del2d.post.res" not in r.outputs


def test_run_nonzero_returncode(tmp_path):
    with patch("framework.runner.subprocess.run") as mock_run:
        mock_run.return_value.returncode = 139  # segfault
        mock_run.return_value.stdout = ""
        mock_run.return_value.stderr = "Segmentation fault"
        r = run("del2d.exe", ["del2d", "."], tmp_path, [])
        assert r.returncode == 139
        assert "Segmentation" in r.stderr


def test_run_timeout(tmp_path):
    import subprocess
    with patch("framework.runner.subprocess.run") as mock_run:
        mock_run.side_effect = subprocess.TimeoutExpired(cmd="del2d.exe", timeout=1)
        r = run("del2d.exe", ["del2d", "."], tmp_path, ["del2d.post.res"])
        assert r.timed_out is True
        assert r.returncode == -1
        assert r.outputs == {}
