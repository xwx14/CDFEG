from dataclasses import dataclass


@dataclass(frozen=True)
class Tolerance:
    """数值对比容差：|actual-baseline| <= atol + rtol*|baseline|"""
    atol: float
    rtol: float = 0.0

    def accept(self, actual: float, baseline: float) -> bool:
        return abs(actual - baseline) <= self.atol + self.rtol * abs(baseline)
