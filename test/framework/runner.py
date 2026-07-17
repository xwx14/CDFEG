"""运行可执行程序并收集产出文件。"""
import os
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


def run(exe, args, cwd, expect_outputs, timeout=600, extra_dll_dirs=None) -> RunResult:
    exe = str(exe)
    args = [str(a) for a in args]
    cwd = Path(cwd)
    # 将 exe 所在目录及额外 DLL 目录加入 PATH，使运行时 DLL 可被找到
    exe_dir = str(Path(exe).resolve().parent)
    env = os.environ.copy()
    path_entries = [exe_dir]
    if extra_dll_dirs:
        path_entries.extend(extra_dll_dirs)
    env["PATH"] = os.pathsep.join(path_entries) + os.pathsep + env.get("PATH", "")
    try:
        proc = subprocess.run(
            [exe, *args], cwd=str(cwd), capture_output=True,
            text=True, timeout=timeout, env=env,
            encoding="utf-8", errors="replace",
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
