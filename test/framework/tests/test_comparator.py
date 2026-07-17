from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.parser import parse_res_file, ResBlock
from framework.tolerance import Tolerance
from framework.comparator import compare

FIXTURE = Path(__file__).parent / "fixtures" / "del2d_sample.res"


def _blocks():
    return parse_res_file(FIXTURE)


def test_identical_blocks_pass():
    b = _blocks()
    r = compare(b, b, Tolerance(atol=1e-12))
    assert r.structural_ok
    assert r.passed
    assert r.n_over_tol == 0
    assert r.max_abs_err == 0.0


def test_numeric_drift_detected():
    actual = _blocks()
    # 篡改 disp step1 node1 的 u 分量
    actual[("disp", 1)].values[1][0] += 1e-5
    r = compare(actual, _blocks(), Tolerance(atol=1e-12))
    assert r.structural_ok
    assert not r.passed
    assert r.n_over_tol >= 1
    assert "disp" in r.first_over
    assert "1" in r.first_over  # step 或 node


def test_structural_drift_step_missing():
    actual = _blocks()
    del actual[("disp", 2)]
    r = compare(actual, _blocks(), Tolerance(atol=1e-12))
    assert not r.structural_ok
    assert not r.passed
    assert any("disp" in e for e in r.structural_errors)


def test_structural_drift_component_change():
    actual = _blocks()
    actual[("disp", 1)].components = ["u", "v", "w"]
    r = compare(actual, _blocks(), Tolerance(atol=1e-12))
    assert not r.structural_ok


def test_worst_point_tracked():
    actual = _blocks()
    actual[("disp", 1)].values[1][0] += 1e-3
    actual[("disp", 1)].values[2][0] += 1e-2  # 更大
    r = compare(actual, _blocks(), Tolerance(atol=1e-12))
    assert abs(r.worst_delta - 1e-2) < 1e-13
