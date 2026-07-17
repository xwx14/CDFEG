"""读取 config.toml，提供结构化访问。Python 3.11+ 用标准库 tomllib。"""
import sys
from dataclasses import dataclass
from pathlib import Path

if sys.version_info >= (3, 11):
    import tomllib
else:  # pragma: no cover
    import tomli as tomllib  # 需 pip install tomli


@dataclass
class Toolchain:
    cmake_generator: str
    source_dir: str
    build_dir: str
    output_subdir: str


class Config:
    def __init__(self, raw: dict, base_dir: Path):
        self._raw = raw
        self.base_dir = base_dir  # config.toml 所在目录，用于解析相对路径
        tc = raw.get("toolchain", {})
        self.toolchain = Toolchain(
            cmake_generator=tc.get("cmake_generator", "Unix Makefiles"),
            source_dir=tc.get("source_dir", "FEMproject"),
            build_dir=tc.get("build_dir", "test/build"),
            output_subdir=tc.get("output_subdir", "output"),
        )

    def suite_e2e(self) -> list[dict]:
        return self._raw.get("suite", {}).get("e2e", {}).get("cases", [])

    def suite_unit(self) -> list[dict]:
        return self._raw.get("suite", {}).get("unit", {}).get("cases", [])

    def suite_generator(self) -> dict:
        return self._raw.get("suite", {}).get("generator", {})

    def suite_analytical(self) -> list[dict]:
        return self._raw.get("suite", {}).get("analytical", {}).get("cases", [])


def load_config(path) -> Config:
    path = Path(path)
    with open(path, "rb") as f:
        raw = tomllib.load(f)
    return Config(raw, base_dir=path.parent)
