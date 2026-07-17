"""两段判定：结构一致性（硬信号）优先于数值对比。"""
from dataclasses import dataclass, field

from framework.tolerance import Tolerance

Blocks = dict  # dict[(result_name, step), ResBlock]


@dataclass
class CompareResult:
    structural_ok: bool
    structural_errors: list[str] = field(default_factory=list)
    max_abs_err: float = 0.0
    n_points: int = 0
    n_over_tol: int = 0
    first_over: str | None = None   # "result=disp step=1 node=42 comp=v"
    worst_point: str | None = None
    worst_delta: float = 0.0
    passed: bool = True


def _check_structure(actual: Blocks, baseline: Blocks) -> list[str]:
    """返回结构漂移描述列表；空列表表示结构一致。"""
    errors: list[str] = []
    a_keys, b_keys = set(actual.keys()), set(baseline.keys())
    if a_keys != b_keys:
        missing = b_keys - a_keys
        extra = a_keys - b_keys
        if missing:
            errors.append(f"缺失结果段: {sorted(missing)}")
        if extra:
            errors.append(f"新增结果段: {sorted(extra)}")
        return errors  # 键都不一致，分量/节点检查无意义
    for key in b_keys:
        a, b = actual[key], baseline[key]
        if a.components != b.components:
            errors.append(f"分量变化 [{key}]: actual={a.components} baseline={b.components}")
        if set(a.values.keys()) != set(b.values.keys()):
            errors.append(
                f"节点集合变化 [{key}]: actual 缺 {sorted(set(b.values)-set(a.values))}, "
                f"多 {sorted(set(a.values)-set(b.values))}"
            )
    return errors


def _compare_numeric(actual: Blocks, baseline: Blocks, tol: Tolerance) -> CompareResult:
    r = CompareResult(structural_ok=True)
    for key, b_blk in baseline.items():
        a_blk = actual[key]
        for node_id, b_vals in b_blk.values.items():
            a_vals = a_blk.values[node_id]
            for i, comp in enumerate(b_blk.components):
                a_v, b_v = a_vals[i], b_vals[i]
                delta = abs(a_v - b_v)
                r.n_points += 1
                if delta > r.worst_delta:
                    r.worst_delta = delta
                    r.worst_point = f"result={key[0]} step={key[1]} node={node_id} comp={comp}"
                if delta > r.max_abs_err:
                    r.max_abs_err = delta
                if not tol.accept(a_v, b_v):
                    r.n_over_tol += 1
                    if r.first_over is None:
                        r.first_over = f"result={key[0]} step={key[1]} node={node_id} comp={comp}"
    r.passed = (r.n_over_tol == 0)
    return r


def compare(actual: Blocks, baseline: Blocks, tol: Tolerance) -> CompareResult:
    structural_errors = _check_structure(actual, baseline)
    if structural_errors:
        return CompareResult(structural_ok=False, structural_errors=structural_errors, passed=False)
    return _compare_numeric(actual, baseline, tol)
