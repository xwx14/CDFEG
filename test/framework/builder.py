"""cmake configure + build 封装。target 级缓存避免重复构建。"""
import subprocess
import sys
from pathlib import Path


class BuildError(RuntimeError):
    pass


class Builder:
    def __init__(self, source_dir, build_dir, generator="Unix Makefiles",
                 extra_cmake_args=None, output_subdir="output"):
        self.source_dir = str(source_dir)
        self.build_dir = str(build_dir)
        self.generator = generator
        self.extra_cmake_args = extra_cmake_args or []
        self.output_subdir = output_subdir
        self._configured = False
        self._built_targets: set[str] = set()

    def _configure(self):
        Path(self.build_dir).mkdir(parents=True, exist_ok=True)
        cmd = ["cmake", "-B", self.build_dir, "-S", self.source_dir,
               "-G", self.generator] + self.extra_cmake_args
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            raise BuildError(f"cmake configure 失败:\n{r.stdout}\n{r.stderr}")
        self._configured = True

    def _exe_path(self, target):
        # cygwin/Windows 下可执行带 .exe；Linux 无后缀
        exe = Path(self.build_dir) / self.output_subdir / target
        if sys.platform == "cygwin" or sys.platform.startswith("win"):
            exe = exe.with_suffix(".exe")
        return exe

    def build(self, targets):
        if not self._configured:
            self._configure()
        new = [t for t in targets if t not in self._built_targets]
        if new:
            cmd = ["cmake", "--build", self.build_dir, "--target", *new, "-j"]
            r = subprocess.run(cmd, capture_output=True, text=True)
            if r.returncode != 0:
                raise BuildError(f"cmake build 失败 ({new}):\n{r.stdout}\n{r.stderr}")
            self._built_targets.update(new)
        return {t: self._exe_path(t) for t in targets}

    def force_reconfigure(self):
        self._configured = False
        self._built_targets.clear()
