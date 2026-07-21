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
import time
from datetime import datetime
from pathlib import Path

TEST_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(TEST_DIR))  # 使 `import framework` 可用
PROJ_ROOT = TEST_DIR.parent

from framework.config import load_config
from framework.tolerance import Tolerance
from framework.builder import Builder
from framework.case import E2ECase, UnitCase, GeneratorCase, AnalyticalCase, SkipCase
from framework.report import aggregate
from framework.timing import TimingDb, detect_regress, git_short_commit


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
                format=c.get("format", "gid"),
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


def persist_timing(db, results, threshold, run_id, ts, git_commit):
    """先查上次 pass → 回归检测 → 批量写入。顺序固定，避免查到本次自身。"""
    for r in results:
        if r.status == "pass" and r.secs > 0:
            last = db.last_pass_secs(r.name)
            reg, detail = detect_regress(r.secs, last, threshold)
            if reg:
                r.timing_regress = True
                r.timing_detail = detail
    for r in results:
        db.insert(r, run_id=run_id, ts=ts, git_commit=git_commit)


def format_timing_list(case_name, rows):
    """rows: iterable of (ts, status, secs, git_commit)。"""
    lines = [f"=== {case_name} 历史耗时 ==="]
    rows = list(rows)
    if not rows:
        lines.append("（无记录）")
        return "\n".join(lines)
    lines.append(f"{'TS':<22} {'STATUS':<8} {'SECS':<8} GIT")
    for ts, status, secs, git_commit in rows:
        lines.append(f"{ts:<22} {status:<8} {secs:<8.2f} {git_commit or ''}")
    return "\n".join(lines)


def main():
    # Windows 终端默认 GBK，强制 stdout/stderr 用 UTF-8，避免中文报告乱码
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
    ap = argparse.ArgumentParser(description="CDFEG 测试系统")
    ap.add_argument("--suite", default="all", choices=["all", "e2e", "unit", "generator", "analytical"])
    ap.add_argument("--case", default=None, help="只跑指定用例名")
    ap.add_argument("--rebuild", action="store_true", help="强制重新 cmake configure")
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--update", default=None, help="刷新指定 e2e 用例基准（Task 13）")
    ap.add_argument("--timing-list", default=None, help="列出指定用例的历史耗时记录")
    args = ap.parse_args()

    if args.update:
        from framework.update import update_baseline  # Task 13 实现
        cfg = load_config(TEST_DIR / "config.toml")
        return update_baseline(cfg, args.update, PROJ_ROOT)

    cfg = load_config(TEST_DIR / "config.toml")
    if args.timing_list:
        db = TimingDb(PROJ_ROOT / cfg.timing.db_path)
        db.init()
        rows = db.list_history(args.timing_list)
        print(format_timing_list(args.timing_list, rows))
        return 0
    cases, _ = build_cases(cfg, args)
    if not cases:
        print("无匹配用例")
        return 0
    results = [c.run(None) for c in cases]

    if cfg.timing.enabled:
        run_id = time.strftime("%Y%m%d_%H%M%S")
        ts = datetime.now().isoformat(timespec="seconds")
        commit = git_short_commit()
        db = TimingDb(PROJ_ROOT / cfg.timing.db_path)
        db.init()
        persist_timing(db, results, cfg.timing.regress_threshold,
                       run_id=run_id, ts=ts, git_commit=commit)

    return aggregate(results)


if __name__ == "__main__":
    sys.exit(main())
