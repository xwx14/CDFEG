"""CDFEG 测试系统统一入口。

用法：
  python test/run_tests.py                         # 默认 all（e2e+unit）
  python test/run_tests.py --suite e2e --case del2d1
  python test/run_tests.py --suite e2e --update del2d1   # 刷新基准（见 Task 13）
  python test/run_tests.py --suite generator       # 生成器（默认不含）
  python test/run_tests.py --rebuild -v
"""
import argparse
import sys
from pathlib import Path

TEST_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(TEST_DIR))  # 使 `import framework` 可用
PROJ_ROOT = TEST_DIR.parent

from framework.config import load_config
from framework.tolerance import Tolerance
from framework.builder import Builder
from framework.case import E2ECase, UnitCase, GeneratorCase, AnalyticalCase, SkipCase
from framework.report import aggregate


def build_cases(cfg, args):
    extra_cmake_args = ["-DCDFEG_BUILD_TESTS=ON"]
    if cfg.toolchain.make_program:
        extra_cmake_args.append(f"-DCMAKE_MAKE_PROGRAM={cfg.toolchain.make_program}")
    builder = Builder(
        source_dir=str(PROJ_ROOT / cfg.toolchain.source_dir),
        build_dir=str(PROJ_ROOT / cfg.toolchain.build_dir),
        generator=cfg.toolchain.cmake_generator,
        extra_cmake_args=extra_cmake_args,
        output_subdir=cfg.toolchain.output_subdir,
    )
    if args.rebuild:
        builder.force_reconfigure()

    cases = []
    suites = _resolve_suites(args)
    if "e2e" in suites:
        for c in cfg.suite_e2e():
            if args.case and c["name"] != args.case:
                continue
            if c.get("skip"):
                cases.append(SkipCase(name=f"e2e.{c['name']}", suite="e2e",
                                      reason=c.get("skip_reason", "配置跳过")))
                continue
            cases.append(E2ECase(
                name=f"e2e.{c['name']}", target=c["target"], project=c["project"],
                case_dir=TEST_DIR / c["case_dir"], baseline=c["baseline"],
                output=c["output"], tol=Tolerance(c["tol_atol"], c.get("tol_rtol", 0.0)),
                builder=builder, dll_dirs=cfg.toolchain.dll_dirs,
            ))
    if "unit" in suites:
        for c in cfg.suite_unit():
            if args.case and c["name"] != args.case:
                continue
            cases.append(UnitCase(name=f"unit.{c['name']}", binary=c["binary"], builder=builder,
                                  dll_dirs=cfg.toolchain.dll_dirs))
    if "generator" in suites:
        g = cfg.suite_generator()
        cases.append(GeneratorCase(name="generator.pytool",
                                   pytest_dir=TEST_DIR / g.get("pytest_dir", "suites/generator")))
    if "analytical" in suites:
        for c in cfg.suite_analytical():
            cases.append(AnalyticalCase(name=f"analytical.{c['name']}"))
    return cases, builder


def _resolve_suites(args):
    if args.suite == "all":
        return ["e2e", "unit"]  # generator 默认不含
    return [args.suite]


def main():
    ap = argparse.ArgumentParser(description="CDFEG 测试系统")
    ap.add_argument("--suite", default="all", choices=["all", "e2e", "unit", "generator", "analytical"])
    ap.add_argument("--case", default=None, help="只跑指定用例名")
    ap.add_argument("--rebuild", action="store_true", help="强制重新 cmake configure")
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--update", default=None, help="刷新指定 e2e 用例基准（Task 13）")
    args = ap.parse_args()

    if args.update:
        from framework.update import update_baseline  # Task 13 实现
        cfg = load_config(TEST_DIR / "config.toml")
        return update_baseline(cfg, args.update, PROJ_ROOT)

    cfg = load_config(TEST_DIR / "config.toml")
    cases, _ = build_cases(cfg, args)
    if not cases:
        print("无匹配用例")
        return 0
    results = [c.run(None) for c in cases]
    return aggregate(results)


if __name__ == "__main__":
    sys.exit(main())
