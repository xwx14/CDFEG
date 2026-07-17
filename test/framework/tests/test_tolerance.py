import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))  # test/ 入 path

from framework.tolerance import Tolerance


class TestPureAtol:
    """纯 atol 容差（rtol=0）：|delta| <= atol"""

    def test_pass_within_atol(self):
        t = Tolerance(atol=1e-6, rtol=0.0)
        assert t.accept(actual=1.0, baseline=1.0 + 5e-7) is True

    def test_fail_exceeds_atol(self):
        t = Tolerance(atol=1e-6, rtol=0.0)
        assert t.accept(actual=1.0, baseline=1.0 + 2e-6) is False


class TestPureRtol:
    """纯 rtol 容差（atol=0）：|delta| <= rtol * |baseline|"""

    def test_pass_within_rtol(self):
        t = Tolerance(atol=0.0, rtol=1e-6)
        assert t.accept(actual=1.0e6, baseline=1.0e6 + 0.5) is True

    def test_fail_exceeds_rtol(self):
        t = Tolerance(atol=0.0, rtol=1e-6)
        assert t.accept(actual=1.0e6, baseline=1.0e6 + 2.0) is False


class TestCombinedAtolRtol:
    """atol + rtol 共同作用：单独 atol 不够、单独 rtol 也不够，合起来才通过"""

    def test_pass_only_when_both_contribute(self):
        # baseline=1000, atol=0.3, rtol=5e-4
        # 阈值 = 0.3 + 5e-4 * 1000 = 0.3 + 0.5 = 0.8
        # 单独 atol=0.3: delta=0.6 > 0.3 → 不够
        # 单独 rtol 贡献=0.5: delta=0.6 > 0.5 → 不够
        # 合起来: delta=0.6 <= 0.8 → 通过
        t = Tolerance(atol=0.3, rtol=5e-4)
        assert t.accept(actual=1000.6, baseline=1000.0) is True

        # delta=0.81 > 0.8 → 失败（验证边界外侧）
        assert t.accept(actual=1000.81, baseline=1000.0) is False


class TestBoundary:
    """边界条件：精确相等、刚好越界"""

    def test_exact_equality_passes(self):
        t = Tolerance(atol=0.0, rtol=0.0)
        assert t.accept(actual=42.0, baseline=42.0) is True

    def test_just_over_boundary_fails(self):
        atol = 1e-10
        t = Tolerance(atol=atol, rtol=0.0)
        epsilon = 1e-15  # 极小偏移量
        assert t.accept(actual=0.0, baseline=atol + epsilon) is False

    def test_just_inside_boundary_passes(self):
        atol = 1e-10
        t = Tolerance(atol=atol, rtol=0.0)
        epsilon = 1e-15
        assert t.accept(actual=0.0, baseline=atol - epsilon) is True
