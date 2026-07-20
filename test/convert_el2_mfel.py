"""把旧 mfel el2 测例(el.dat + el.mat)转为 CDFEG El2D dat。

旧 FEPG 紧凑标志格式 → CDFEG * name= 段头格式。
段顺序固定：-1000(coord) -2001(nodvar) -2001(ubf) -2002(空) -4(ElQ4g) -2(StressBL2g) -4(重复Q4忽略) -5000(结束)
边单元 eleId 接续编号(避开 ElQ4g 1..162 冲突)。
"""
from pathlib import Path

OLD = Path(r"E:\mfelProject\RegTest\testData\models\el2")
DAT = OLD / "el.gid" / "el.dat"
MAT = OLD / "el.mat"
OUT = Path(r"E:\myProject\CDFEG\FEMproject\sample\El2D\el2_mfel.dat")


def parse_segments(path):
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    segs = []
    cur_flag, cur = None, []
    for l in lines:
        s = l.strip()
        if not s:
            continue
        if s[0] == "-":                     # 段标志行
            if cur_flag is not None:
                segs.append((cur_flag, cur))
            cur_flag = s.split()[0]         # -1000 / -2001 / -4 / -2 ...
            cur = []
        else:
            cur.append(s)
    if cur_flag is not None:
        segs.append((cur_flag, cur))
    return segs


segs = parse_segments(DAT)
flag_count = {}
coords = bc = q4 = edges = None
for flag, data in segs:
    flag_count[flag] = flag_count.get(flag, 0) + 1
    n = flag_count[flag]
    if flag == "-1000":
        coords = data                       # 节点 id x y
    elif flag == "-2001" and n == 2:        # 第二个 -2001 = ubf 约束值
        bc = data
    elif flag == "-4" and n == 1:           # 第一组 ElQ4g
        q4 = data
    elif flag == "-2":                      # StressBL2g 边单元
        edges = data

mat_lines = MAT.read_text(encoding="utf-8", errors="replace").splitlines()
elq4_mat = mat_lines[2].strip()             # line 3: pe pv fu fv rou alpha
bl2_mat = mat_lines[4].strip()              # line 5: fu fv (面力)

nnode = len(coords)
nq4 = len(q4)
nedge = len(edges)
nelem = nq4 + nedge

with OUT.open("w", encoding="utf-8") as f:
    f.write('* name=baseData,structure="I32"\n')
    f.write(f"{nnode} {nelem}\n")
    f.write('* name=time,structure="F64"\n1.0 1.0\n')
    f.write('* name=mat_ElQ4g,structure="F64*6",type="mat",index=1\n')
    f.write(f"{elq4_mat}\n")
    f.write('* name=mat_StressBL2g,structure="F64*2",type="mat",index=1\n')
    f.write(f"{bl2_mat}\n")
    f.write('* name=coord,structure="I32*1 F64*2",type="coord",index=1\n')
    for c in coords:
        p = c.split()
        f.write(f"{p[0]} {float(p[1]):.12e} {float(p[2]):.12e}\n")
    f.write('* name=ubfElDisp,structure="I32*1 F64*2",type="ubf",index=0\n')
    for b in bc:
        p = b.split()
        f.write(f"{p[0]} {float(p[1]):.12e} {float(p[2]):.12e}\n")
    f.write('* name=ElQ4g,structure="I32*6",type="elem",index=1\n')
    for q in q4:
        p = q.split()
        f.write(f"{p[0]} {p[1]} {p[2]} {p[3]} {p[4]} {p[5]}\n")
    f.write('* name=StressBL2g,structure="I32*4",type="elem",index=1\n')
    for i, e in enumerate(edges):
        p = e.split()
        ele_id = nq4 + 1 + i                # 163, 164, ... 避开 ElQ4g
        f.write(f"{ele_id} {p[1]} {p[2]} {p[3]}\n")

print(f"wrote {OUT}: {nnode} nodes, {nq4} ElQ4g, {nedge} StressBL2g")
print(f"mat_ElQ4g={elq4_mat}  mat_StressBL2g={bl2_mat}")
