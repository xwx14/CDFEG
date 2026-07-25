"""VTU/PVD 解析单测。"""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))  # 使 import framework 可用

from framework.parser import parse_vtu_file, parse_pvd_file


def _write_vtu(path, arrays):
    """arrays: list of (name, ncomp, location, values_flat)。location: 'PointData'/'CellData'。"""
    with open(path, "w", encoding="utf-8") as f:
        f.write('<?xml version="1.0"?>\n<VTKFile type="UnstructuredGrid" version="0.1">'
                '<UnstructuredGrid><Piece NumberOfPoints="2" NumberOfCells="1">\n')
        f.write('<Points><DataArray type="Float64" NumberOfComponents="3" format="ascii">\n0 0 0 1 0 0\n'
                '</DataArray></Points>\n')
        f.write('<Cells><DataArray type="Int32" Name="connectivity" format="ascii">0 1</DataArray>'
                '<DataArray type="Int32" Name="offsets" format="ascii">2</DataArray>'
                '<DataArray type="UInt8" Name="types" format="ascii">3</DataArray></Cells>\n')
        for name, ncomp, loc, vals in arrays:
            f.write(f'<{loc}><DataArray type="Float64" Name="{name}" '
                    f'NumberOfComponents="{ncomp}" format="ascii">\n')
            f.write(" ".join(str(v) for v in vals) + "\n")
            f.write(f'</DataArray></{loc}>\n')
        f.write('</Piece></UnstructuredGrid></VTKFile>\n')


def test_parse_vtu_point_data(tmp_path):
    p = tmp_path / "a.vtu"
    _write_vtu(p, [("disp", 2, "PointData", [1.0, 2.0, 3.0, 4.0])])  # 2 节点 × 2 分量
    blocks = parse_vtu_file(p, step=0)
    assert ("disp", 0) in blocks
    blk = blocks[("disp", 0)]
    assert blk.components == ["comp_0", "comp_1"]
    assert blk.values[1] == [1.0, 2.0]   # 第一节点（entity_id 从 1 起，与 GiD res 一致）
    assert blk.values[2] == [3.0, 4.0]   # 第二节点
    assert blk.location == "OnNodes"


def test_parse_vtu_cell_data(tmp_path):
    p = tmp_path / "a.vtu"
    _write_vtu(p, [("stress", 3, "CellData", [10.0, 20.0, 30.0])])  # 1 单元 × 3 分量
    blocks = parse_vtu_file(p, step=5)
    assert ("stress", 5) in blocks
    assert blocks[("stress", 5)].values[1] == [10.0, 20.0, 30.0]
    assert blocks[("stress", 5)].location == "OnCells"


def test_parse_pvd_collects_steps(tmp_path):
    for it in range(3):
        _write_vtu(tmp_path / f"s_{it:04d}.vtu",
                   [("disp", 1, "PointData", [float(it), float(it)])])
    pvd = tmp_path / "s.pvd"
    pvd.write_text(
        '<?xml version="1.0"?>\n<VTKFile type="Collection"><Collection>\n'
        '<DataSet timestep="0.0" part="0" file="s_0000.vtu"/>\n'
        '<DataSet timestep="0.1" part="0" file="s_0001.vtu"/>\n'
        '<DataSet timestep="0.2" part="0" file="s_0002.vtu"/>\n'
        '</Collection></VTKFile>\n', encoding="utf-8")
    blocks = parse_pvd_file(pvd)
    assert sorted(k[1] for k in blocks) == [0, 1, 2]   # 顺序索引作 step
    assert blocks[("disp", 2)].values[1] == [2.0]
