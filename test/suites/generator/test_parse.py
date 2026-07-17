"""pyTool 解析正确性：迁移自 pyTool/test/test_preParser.py 的断言。
需 macs 基准数据 E:/mfelProject/RegTest/testData/macs/el 存在；不存在则 skip。
"""
import os
import sys
from pathlib import Path

import pytest

PROJ_ROOT = Path(__file__).resolve().parents[3]
PYTOOL = PROJ_ROOT / "pyTool"
sys.path.insert(0, str(PYTOOL))

MACS = r"E:/mfelProject/RegTest/testData/macs"


@pytest.fixture(scope="module")
def el_project():
    if not os.path.isdir(MACS):
        pytest.skip(f"macs 基准目录不存在: {MACS}")
    from fepgParser import parseProject
    return parseProject(MACS, "el")


def test_dim_and_fields(el_project):
    assert el_project.dim == 2
    names = [f.name for f in el_project.fields]
    assert set(names) == {"ela", "elb"}


def test_ela_disp_names(el_project):
    ela = next(f for f in el_project.fields if f.name == "ela")
    assert ela.dispNames == ["u", "v"]


def test_ela_eleSubs(el_project):
    ela = next(f for f in el_project.fields if f.name == "ela")
    ele_names = [e.name for e in ela.eleSubs]
    assert "a1eq4g2" in ele_names
    assert "a2ll2" in ele_names
    a1 = next(e for e in ela.eleSubs if e.name == "a1eq4g2")
    assert a1.nNodes == 4


def test_elb_disp_names(el_project):
    elb = next(f for f in el_project.fields if f.name == "elb")
    assert elb.dispNames == ["dxx", "dyy", "dxy"]
