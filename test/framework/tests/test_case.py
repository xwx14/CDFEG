from pathlib import Path
import sys
import shutil
from unittest.mock import MagicMock
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.case import E2ECase
from framework.tolerance import Tolerance
from framework.runner import RunResult

FIXTURE = Path(__file__).parent / "fixtures" / "del2d_sample.res"


def _make_case(case_dir, tmp_path):
    builder = MagicMock()
    builder.build.return_value = {"del2d": tmp_path / "del2d.exe"}
    builder.build_dir = str(tmp_path / "build")  # _prepare_work_dir 用
    return builder, E2ECase(
        name="e2e.del2d1", target="del2d", project="del2d",
        case_dir=case_dir, baseline="del2d.post.res", output="del2d.post.res",
        tol=Tolerance(atol=1e-12), builder=builder,
    )


def _setup_case_dir(tmp_path):
    case_dir = tmp_path / "models" / "del2d1.gid"
    case_dir.mkdir(parents=True)
    shutil.copy(FIXTURE, case_dir / "del2d.post.res")  # 基准
    return case_dir


def test_e2e_pass_when_identical(tmp_path):
    case_dir = _setup_case_dir(tmp_path)
    builder, case = _make_case(case_dir, tmp_path)

    def fake_run(exe, args, cwd, expect, timeout=600, **kwargs):
        out = Path(cwd) / "del2d.post.res"
        shutil.copy(FIXTURE, out)  # 模拟 exe 产出 == 基准
        return RunResult(0, "", "", {"del2d.post.res": out})

    case._runner_run = fake_run
    r = case.run(None)
    assert r.status == "pass"


def test_e2e_work_dir_isolated_from_baseline(tmp_path):
    """关键：exe 在隔离 work_dir 跑，case_dir 的基准不被覆盖。"""
    case_dir = _setup_case_dir(tmp_path)
    before = (case_dir / "del2d.post.res").read_bytes()
    builder, case = _make_case(case_dir, tmp_path)

    def fake_run(exe, args, cwd, expect, timeout=600, **kwargs):
        out = Path(cwd) / "del2d.post.res"
        shutil.copy(FIXTURE, out)
        return RunResult(0, "", "", {"del2d.post.res": out})

    case._runner_run = fake_run
    case.run(None)
    after = (case_dir / "del2d.post.res").read_bytes()
    assert before == after  # 基准未被改动
    work_output = Path(builder.build_dir) / "run" / "e2e.del2d1" / "del2d.post.res"
    assert work_output.exists(), "产出应在隔离 work_dir 中，而非 case_dir"


def test_e2e_error_when_exe_crash(tmp_path):
    case_dir = _setup_case_dir(tmp_path)
    builder, case = _make_case(case_dir, tmp_path)
    case._runner_run = MagicMock(return_value=RunResult(139, "", "segfault", {}))
    r = case.run(None)
    assert r.status == "error"


def test_e2e_error_when_output_missing(tmp_path):
    case_dir = _setup_case_dir(tmp_path)
    builder, case = _make_case(case_dir, tmp_path)
    case._runner_run = MagicMock(return_value=RunResult(0, "", "", {}))  # 无产出
    r = case.run(None)
    assert r.status == "error"
