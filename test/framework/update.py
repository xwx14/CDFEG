"""基准更新：显式、逐用例、强制 diff 确认。禁用 --update all。
在隔离工作目录跑示例，确认后才覆盖 case_dir 中的基准（基准永不在跑测时被触碰）。"""
import shutil
import sys
from pathlib import Path

from framework.config import Config
from framework.builder import Builder
from framework.runner import run
from framework.parser import parse_res_file, parse_pvd_file
from framework.txt_parser import parse_truss_txt
from framework.comparator import compare
from framework.tolerance import Tolerance


class UpdateError(RuntimeError):
    pass


def _find_e2e_case(cfg: Config, case_name: str) -> dict:
    for c in cfg.suite_e2e():
        if c["name"] == case_name:
            return c
    raise UpdateError(f"未知 e2e 用例: {case_name}")


def _run_in_work_dir(builder, c, case_dir, build_dir, dll_dirs) -> Path:
    """在隔离工作目录跑示例，返回产出文件路径（不触碰 case_dir 基准）。"""
    work_dir = Path(build_dir) / "run" / f"update_{c['name']}"
    if work_dir.exists():
        shutil.rmtree(work_dir)
    work_dir.mkdir(parents=True)
    for f in case_dir.iterdir():
        if (f.is_file() and ".post.res" not in f.name
                and not f.name.endswith(".bak") and f.name != c["baseline"]):
            shutil.copy2(f, work_dir / f.name)
    exes = builder.build([c["target"]])
    rr = run(exes[c["target"]], [c["project"], "."], work_dir, [c["output"]],
             timeout=600, extra_dll_dirs=dll_dirs)
    if rr.returncode != 0 or c["output"] not in rr.outputs:
        raise UpdateError(f"运行失败 (code={rr.returncode}): {rr.stderr[:300]}")
    return rr.outputs[c["output"]]


def update_baseline(cfg: Config, case_name: str, proj_root: Path) -> int:
    if case_name == "all":
        raise UpdateError("禁止 --update all：必须逐用例更新，强制人工过目每个变更")
    c = _find_e2e_case(cfg, case_name)

    test_dir = proj_root / "test"
    case_dir = test_dir / c["case_dir"]
    baseline_path = case_dir / c["baseline"]
    build_dir = proj_root / cfg.toolchain.build_dir

    builder = Builder(
        source_dir=str(proj_root / cfg.toolchain.source_dir),
        build_dir=str(build_dir),
        generator=cfg.toolchain.cmake_generator,
        extra_cmake_args=["-DCDFEG_BUILD_TESTS=ON"],
        output_subdir=cfg.toolchain.output_subdir,
    )
    actual_path = _run_in_work_dir(builder, c, case_dir, build_dir, cfg.toolchain.dll_dirs)

    # diff 摘要（若旧基准存在）
    max_delta = 0.0
    if baseline_path.exists():
        fmt = c.get("format", "gid")
        if fmt == "truss_txt":
            _parse = parse_truss_txt
        elif fmt == "pvd":
            _parse = parse_pvd_file
        else:
            _parse = parse_res_file
        cr = compare(_parse(actual_path), _parse(baseline_path),
                     Tolerance(atol=0.0, rtol=0.0))
        max_delta = cr.max_abs_err
        print(f"[diff] {case_name}: max|Δ|={cr.max_abs_err:.3e} "
              f"结构错误={cr.structural_errors or '无'}")
    else:
        print(f"[diff] {case_name}: 无旧基准，将新建")

    # 强制人工确认
    print(f"\n即将覆盖: {baseline_path}")
    print("输入 yes 确认更新，其他取消: ", end="", flush=True)
    answer = sys.stdin.readline().strip()
    if answer != "yes":
        print("已取消，基准未改动。")
        return 1

    if c.get("format") == "pvd":
        # 多文件基线：拷贝 <project>.pvd + <project>_*.vtu 群，先清旧基线群
        import glob
        for old in glob.glob(str(case_dir / f"{c['project']}*.vtu")):
            Path(old).unlink()
        old_pvd = case_dir / c["baseline"]
        if old_pvd.exists():
            old_pvd.unlink()
        for vtu in glob.glob(str(Path(actual_path).parent / f"{c['project']}_*.vtu")):
            shutil.copy2(vtu, case_dir / Path(vtu).name)
        shutil.copy2(actual_path, baseline_path)
    else:
        shutil.copy2(actual_path, baseline_path)
    # 记录更新日志
    log = test_dir / "reports" / "update_log.txt"
    log.parent.mkdir(parents=True, exist_ok=True)
    with open(log, "a", encoding="utf-8") as f:
        f.write(f"{case_name}\tmax|Δ|={max_delta:.3e}\t(原因待填)\n")
    print(f"已更新 {baseline_path}")
    print(f"建议: git add {baseline_path} && git commit -m 'test: 刷新 {case_name} regression 基准'")
    return 0
