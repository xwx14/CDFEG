"""GiD .post.res 结果文件解析。

格式（权威：GiD 17 Customization Manual）：
  Result "<名>" "<分析>" <步> <类型> <位置>
  ComponentNames "<c1>" "<c2>" ...
  Values
      <实体号> <v1> <v2> ...
  End Values
"""
import re
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
