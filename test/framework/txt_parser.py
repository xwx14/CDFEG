"""truss 示例 .txt 输出解析（节点位移 / 单元内力分节文本）。

格式：
  ========== <节名> ==========
  <列名1>\t<列名2>\t...
  <id>\t<val1>\t<val2>\t...
  ...

映射到与 parser.parse_res_file 相同的 dict[(name, step, gp_name), ResBlock]，从而复用
comparator：节名→result_name（step 固定 1），表头去首列(实体号列)→components，
数据行首列→实体号键，其余→数值列。comparator 只用 components + values 做对比，
result_type/location 仅占位。
"""
from framework.parser import ResBlock


def parse_truss_txt(path) -> dict:
    blocks: dict = {}
    cur = None
    expect_header = False
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            s = line.strip()
            if not s:
                continue
            if "==========" in s:
                name = s.replace("=", "").strip()
                cur = ResBlock(result_name=name, analysis="", step=1,
                               result_type="Scalar", location="OnNodes")
                blocks[(name, 1, "")] = cur
                expect_header = True
                continue
            if cur is None:
                continue
            parts = line.split()                       # 折叠任意空白（含 \t\t）
            if expect_header:
                cur.components = parts[1:]             # 去首列（实体号列名）
                expect_header = False
                continue
            if len(parts) < 2:
                continue
            try:
                eid = int(float(parts[0]))
                cur.values[eid] = [float(x) for x in parts[1:]]
            except ValueError:
                continue                               # 跳过无法解析行
    return blocks
