"""update.py 的单元测试：安全护栏（--update all 禁用 + 未知用例拒绝）。"""
from pathlib import Path
import sys
import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.update import update_baseline, UpdateError


def test_update_all_rejected(tmp_path):
    with pytest.raises(UpdateError, match="禁止"):
        from framework.config import Config
        cfg = Config({}, base_dir=tmp_path)
        update_baseline(cfg, "all", tmp_path)


def test_update_unknown_case_rejected(tmp_path):
    from framework.config import Config
    cfg = Config({}, base_dir=tmp_path)
    with pytest.raises(UpdateError):
        update_baseline(cfg, "nonexistent", tmp_path)
