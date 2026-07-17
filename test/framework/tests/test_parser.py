from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.parser import parse_res_file, ResBlock

FIXTURE = Path(__file__).parent / "fixtures" / "del2d_sample.res"


def test_parse_disp_block_step1():
    blocks = parse_res_file(FIXTURE)
    assert ("disp", 1) in blocks
    b = blocks[("disp", 1)]
    assert b.result_name == "disp"
    assert b.step == 1
    assert b.result_type == "Vector"
    assert b.location == "OnNodes"
    assert b.components == ["u", "v"]
    assert b.values[1] == [-1.0214411e-05, -9.7636690e-06]


def test_parse_stress_matrix_components():
    blocks = parse_res_file(FIXTURE)
    b = blocks[("stress", 1)]
    assert b.components == ["sigmaXX", "sigmaYY", "sigmaXY"]
    assert b.result_type == "Matrix"
    assert len(b.values[1]) == 3


def test_parse_multiple_steps():
    blocks = parse_res_file(FIXTURE)
    assert ("disp", 1) in blocks
    assert ("disp", 2) in blocks
    assert blocks[("disp", 2)].values[1][0] == -5.0607906e-05


def test_parse_node_keys_are_int():
    blocks = parse_res_file(FIXTURE)
    for node_id in blocks[("disp", 1)].values:
        assert isinstance(node_id, int)
