"""Case 抽象与 CaseResult。框架只依赖 Case 协议，套件是其实现。"""
from dataclasses import dataclass, field


@dataclass
class CaseResult:
    name: str          # "e2e.del2d1"
    suite: str         # "e2e" | "unit" | "generator" | "analytical"
    status: str        # "pass" | "fail" | "error" | "skip"
    metric: dict = field(default_factory=dict)
    detail: str = ""
