"""Case 抽象与 CaseResult。框架只依赖 Case 协议，套件是其实现。"""
from dataclasses import dataclass, field
from pathlib import Path
import shutil
import time
from typing import Protocol

from framework.parser import parse_res_file
from framework.txt_parser import parse_truss_txt
from framework.comparator import compare
from framework.runner import run as _runner_run_default, RunResult
from framework.tolerance import Tolerance


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


class Case(Protocol):
    name: str
    suite: str
    def run(self, ctx) -> "CaseResult": ...


class E2ECase:
    """端到端回归：构建示例 -> 在隔离工作目录跑 -> 对比基准（基准永不被 exe 覆盖）。"""

    suite = "e2e"

    def __init__(self, name, target, project, case_dir, baseline, output,
                 tol: Tolerance, builder, timeout=600, dll_dirs=None, format="gid"):
        self.name = name
        self.target = target
        self.project = project
        self.case_dir = Path(case_dir)
        self.baseline = baseline
        self.output = output
        self.tol = tol
        self.builder = builder
        self.timeout = timeout
        self.dll_dirs = dll_dirs or []
        self.format = format
        self._runner_run = _runner_run_default  # 可被测试替换

    def _prepare_work_dir(self) -> Path:
        """隔离工作目录：拷贝输入（排除 .post.res 基准与 .bak），避免 exe 覆盖基准。"""
        work_dir = Path(self.builder.build_dir) / "run" / self.name
        if work_dir.exists():
            shutil.rmtree(work_dir)
        work_dir.mkdir(parents=True)
        for f in self.case_dir.iterdir():
            if (f.is_file() and ".post.res" not in f.name
                    and not f.name.endswith(".bak") and f.name != self.baseline):
                shutil.copy2(f, work_dir / f.name)
        return work_dir

    def _parse(self, path):
        """按 format 选解析器：gid→GiD .post.res；truss_txt→分节文本。两者都产出
        dict[(name,step), ResBlock]，复用 comparator。"""
        if self.format == "truss_txt":
            return parse_truss_txt(path)
        return parse_res_file(path)

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


class UnitCase:
    """C++ Catch2 单测：跑 cdfeg_unit_test.exe，退出码 0=全过。"""
    suite = "unit"

    def __init__(self, name, binary, builder, timeout=300, dll_dirs=None):
        self.name = name
        self.binary = binary
        self.builder = builder
        self.timeout = timeout
        self.dll_dirs = dll_dirs or []

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


class GeneratorCase:
    """pyTool 生成器测试：跑 pytest 子目录。"""
    suite = "generator"

    def __init__(self, name, pytest_dir):
        self.name = name
        self.pytest_dir = pytest_dir

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


class AnalyticalCase:
    """解析解校验：跑示例后取关键点与解析公式比（首批仅占位）。"""
    suite = "analytical"

    def __init__(self, name):
        self.name = name

    def run(self, ctx) -> CaseResult:
        return CaseResult(self.name, self.suite, "skip", detail="解析解算例待实现")


class SkipCase:
    """config 标 skip=true 的用例：不执行，报告中显示 skip 与原因。"""
    def __init__(self, name, suite="e2e", reason=""):
        self.name = name
        self.suite = suite
        self.reason = reason

    def run(self, ctx) -> CaseResult:
        return CaseResult(self.name, self.suite, "skip", detail=self.reason or "配置跳过")
