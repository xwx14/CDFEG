from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from framework.txt_parser import parse_truss_txt

FIXTURE = Path(__file__).parent / "fixtures" / "truss1D_sample.txt"


def test_parse_two_sections():
    blocks = parse_truss_txt(FIXTURE)
    assert ("节点位移", 1) in blocks
    assert ("单元内力", 1) in blocks


def test_node_disp_components_and_values():
    blocks = parse_truss_txt(FIXTURE)
    b = blocks[("节点位移", 1)]
    assert b.components == ["坐标", "位移u"]
    assert b.values[2] == [0.6, 0.00005]
    assert b.values[4] == [1.8, 0.0]


def test_element_force_components_and_values():
    blocks = parse_truss_txt(FIXTURE)
    b = blocks[("单元内力", 1)]
    assert b.components == ["轴力T", "应力sigma"]
    assert b.values[1] == [10000.0, 16666666.666667]
    assert b.values[3] == [-5000.0, -4166666.666667]


def test_entity_keys_are_int():
    blocks = parse_truss_txt(FIXTURE)
    for k in blocks[("节点位移", 1)].values:
        assert isinstance(k, int)
