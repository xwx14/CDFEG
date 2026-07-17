"""聚合 CaseResult，打印报告，返回退出码。"""
from framework.case import CaseResult

EXIT_OK = 0
EXIT_FAIL = 1     # 有真实回归（fail）
EXIT_ERROR = 2    # 框架/基础设施问题（error）


def aggregate(results: list[CaseResult]) -> int:
    print_report(results)
    has_fail = any(r.status == "fail" for r in results)
    has_error = any(r.status == "error" for r in results)
    if has_fail:
        return EXIT_FAIL
    if has_error:
        return EXIT_ERROR
    return EXIT_OK


def print_report(results: list[CaseResult]):
    print("\n" + "=" * 70)
    print(f"{'CASE':<28} {'STATUS':<8} {'METRIC'}")
    print("-" * 70)
    for r in results:
        metric_str = ""
        if r.status == "pass" and r.metric.get("max_abs_err") is not None:
            metric_str = f"max|Δ|={r.metric['max_abs_err']:.2e}"
        elif r.status == "fail":
            metric_str = r.detail[:40]
        elif r.status == "error":
            metric_str = r.detail[:40]
        print(f"{r.name:<28} {r.status:<8} {metric_str}")
    print("=" * 70)
    n_pass = sum(1 for r in results if r.status == "pass")
    n_fail = sum(1 for r in results if r.status == "fail")
    n_err = sum(1 for r in results if r.status == "error")
    n_skip = sum(1 for r in results if r.status == "skip")
    print(f"合计: {n_pass} pass / {n_fail} fail / {n_err} error / {n_skip} skip")
    # 失败详情
    for r in results:
        if r.status in ("fail", "error") and r.detail:
            print(f"\n[{r.status.upper()}] {r.name}\n      {r.detail}")
