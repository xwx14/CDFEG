import pytest
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))  # test/ 入 path
from framework.tolerance import Tolerance
from framework.config import load_config


def test_tolerance_defaults():
    t = Tolerance(atol=1e-12)
    assert t.atol == 1e-12
    assert t.rtol == 0.0


def test_load_config_reads_toolchain(tmp_path):
    cfg_file = tmp_path / "config.toml"
    cfg_file.write_text(
        '[toolchain]\n'
        'cmake_generator = "Unix Makefiles"\n'
        'source_dir = "FEMproject"\n'
        'build_dir = "test/build"\n'
        'output_subdir = "output"\n',
        encoding="utf-8",
    )
    cfg = load_config(cfg_file)
    assert cfg.toolchain.cmake_generator == "Unix Makefiles"
    assert cfg.toolchain.source_dir == "FEMproject"


def test_load_config_reads_e2e_cases(tmp_path):
    cfg_file = tmp_path / "config.toml"
    cfg_file.write_text(
        '[[suite.e2e.cases]]\n'
        'name = "del2d1"\n'
        'target = "del2d"\n'
        'project = "del2d"\n'
        'case_dir = "models/del2d1.gid"\n'
        'baseline = "del2d.post.res"\n'
        'output = "del2d.post.res"\n'
        'tol_atol = 1e-12\n',
        encoding="utf-8",
    )
    cfg = load_config(cfg_file)
    assert len(cfg.suite_e2e()) == 1
    c = cfg.suite_e2e()[0]
    assert c["name"] == "del2d1"
    assert c["tol_atol"] == 1e-12


def test_load_config_reads_timing(tmp_path):
    cfg_file = tmp_path / "config.toml"
    cfg_file.write_text(
        '[timing]\n'
        'enabled = true\n'
        'db_path = "test/timing.db"\n'
        'regress_threshold = 0.05\n',
        encoding="utf-8",
    )
    cfg = load_config(cfg_file)
    assert cfg.timing.enabled is True
    assert cfg.timing.db_path == "test/timing.db"
    assert cfg.timing.regress_threshold == 0.05


def test_load_config_timing_defaults_when_absent(tmp_path):
    cfg_file = tmp_path / "config.toml"
    cfg_file.write_text('[toolchain]\n', encoding="utf-8")
    cfg = load_config(cfg_file)
    assert cfg.timing.enabled is True
    assert cfg.timing.db_path == "test/timing.db"
    assert cfg.timing.regress_threshold == 0.05
