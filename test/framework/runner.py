"""运行可执行程序并收集产出文件。"""
import subprocess
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class RunResult:
    returncode: int
    stdout: str
    stderr: str
    outputs: dict = field(default_factory=dict)  # name -> Path（仅含存在的）
    timed_out: bool = False


def run(exe, args, cwd, expect_outputs, timeout=600) -> RunResult:
    exe = str(exe)
    args = [str(a) for a in args]
    cwd = Path(cwd)
    try:
        proc = subprocess.run(
            [exe, *args], cwd=str(cwd), capture_output=True,
            text=True, timeout=timeout,
        )
    except subprocess.TimeoutExpired as e:
        return RunResult(returncode=-1, stdout=e.stdout or "", stderr=e.stderr or "",
                         timed_out=True)

    outputs = {}
    for name in expect_outputs:
        p = cwd / name
        if p.exists():
            outputs[name] = p
    return RunResult(returncode=proc.returncode, stdout=proc.stdout,
                     stderr=proc.stderr, outputs=outputs)
