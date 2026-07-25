"""GiD .post.res 结果文件解析。

格式（权威：GiD 17 Customization Manual）：
  Result "<名>" "<分析>" <步> <类型> <位置>
  ComponentNames "<c1>" "<c2>" ...
  Values
      <实体号> <v1> <v2> ...
  End Values
"""
import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path

RESULT_RE = re.compile(r'^Result\s+"([^"]+)"\s+"([^"]+)"\s+(\d+)\s+(\S+)\s+(\S+)')
COMPONENT_RE = re.compile(r'"([^"]*)"')


@dataclass
class ResBlock:
    result_name: str
    analysis: str
    step: int
    result_type: str
    location: str
    components: list[str] = field(default_factory=list)
    values: dict[int, list[float]] = field(default_factory=dict)


def parse_res_file(path) -> dict[tuple[str, int], ResBlock]:
    """解析 .res -> {(result_name, step): ResBlock}。"""
    blocks: dict[tuple[str, int], ResBlock] = {}
    cur: ResBlock | None = None
    in_values = False

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.rstrip("\n")
            m = RESULT_RE.match(line)
            if m:
                cur = ResBlock(
                    result_name=m.group(1), analysis=m.group(2),
                    step=int(m.group(3)), result_type=m.group(4), location=m.group(5),
                )
                blocks[(cur.result_name, cur.step)] = cur
                in_values = False
                continue
            if line.startswith("ComponentNames") and cur is not None:
                cur.components = COMPONENT_RE.findall(line)
                continue
            if line.strip() == "Values":
                in_values = True
                continue
            if line.strip() == "End Values":
                in_values = False
                continue
            if in_values and cur is not None:
                parts = line.split()
                if len(parts) < 2:
                    continue
                try:
                    node_id = int(parts[0])
                    cur.values[node_id] = [float(x) for x in parts[1:]]
                except ValueError:
                    continue  # 跳过无法解析行
    return blocks


def parse_vtu_file(path, step) -> dict[tuple[str, int], ResBlock]:
    """解析 .vtu -> {(name, step): ResBlock}。

    PointData -> location='OnNodes'（按节点序，entity_id 从 1 起）；
    CellData  -> location='OnCells'（按单元序）。
    分量名统一 comp_N（VTU 不存分量名，actual/baseline 同规则即可对齐）。
    """
    blocks: dict[tuple[str, int], ResBlock] = {}
    tree = ET.parse(path)
    root = tree.getroot()
    piece = root.find(".//Piece")
    if piece is None:
        return blocks
    for loc_tag, loc_name in (("PointData", "OnNodes"), ("CellData", "OnCells")):
        container = piece.find(loc_tag)
        if container is None:
            continue
        for da in container.findall("DataArray"):
            name = da.get("Name", "")
            ncomp = max(1, int(da.get("NumberOfComponents", "1")))
            vals = [float(x) for x in (da.text or "").split()]
            blk = ResBlock(result_name=name, analysis="", step=step,
                           result_type="", location=loc_name)
            blk.components = [f"comp_{i}" for i in range(ncomp)]
            for eid in range(len(vals) // ncomp):
                blk.values[eid + 1] = vals[eid * ncomp:(eid + 1) * ncomp]
            blocks[(name, step)] = blk
    return blocks


def parse_pvd_file(path) -> dict[tuple[str, int], ResBlock]:
    """解析 .pvd，按 <DataSet> 顺序（索引即 step）逐个解析引用的 vtu，合并。"""
    blocks: dict[tuple[str, int], ResBlock] = {}
    base = Path(path).parent
    tree = ET.parse(path)
    for step, ds in enumerate(tree.findall(".//DataSet")):
        fn = ds.get("file", "")
        if not fn:
            continue
        vtu = base / fn
        if not vtu.exists():
            continue
        for k, blk in parse_vtu_file(vtu, step).items():
            blocks[k] = blk
    return blocks
