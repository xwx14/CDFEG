# SPDX-License-Identifier: GPL-3.0
# This file is part of CDFEG.
#
# CDFEG is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CDFEG is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.

# 解析 macs 项目的 pre/ges/gcn，构造 pyTool DataProject
import os
from DataProject import DataProject
from DataField import DataField
from DataEleSubG import DataEleSubG

def parsePre(projDir, projName):
    """解析 <projName>.pre → DataProject（dim/场/单元骨架/参数）。
    pre 格式：
      第1行: 2dxy 2 3 8 10   ← 首字段 2dxy/3dxyz 给 dim
      <场名> 0 <dof> <?> <dispNames...>
      # 分隔
      element y
      <场名>
      <单元名> <nNode> <paramNames...>
      #
      matedata
      <场名> <单元名>
       <paramValues...>
      #
    """
    path = os.path.join(projDir, projName, projName + ".pre")
    dim = 2
    project = DataProject(projName, dim)
    fields = {}          # name -> DataField
    field_order = []
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f.readlines()]
    # 第1行取 dim
    if lines:
        head = lines[0].split()
        if head and head[0].startswith("2"):
            dim = 2
        elif head and head[0].startswith("3"):
            dim = 3
        project.dim = dim
    # 状态机解析（从第2行开始，首行已处理 dim）
    mode = "field"
    cur_field = None
    cur_ele_name = None
    i = 1 if lines else 0
    while i < len(lines):
        ln = lines[i].strip()
        i += 1
        if not ln:
            continue
        toks = ln.split()
        if ln == "#":
            # # 仅重置当前场/单元上下文，不改变 mode
            cur_field = None
            cur_ele_name = None
            continue
        if toks[0] == "element":
            mode = "element"
            continue
        if toks[0] == "matedata":
            mode = "matedata"
            continue
        if mode == "field":
            # <场名> 0 <dof> <?> <dispNames...>，场名首字母非数字
            fname = toks[0]
            dispNames = toks[4:]
            fld = DataField(fname)
            fld.project = project
            fields[fname] = fld
            field_order.append(fname)
            # 暂存 dispNames（单元 addEleSub 后会从单元汇总，这里先存）
            fld._preDispNames = dispNames
            cur_field = fld
            project.addField(fld)
            continue
        if mode == "element":
            if cur_field is None or toks[0] in fields:
                # 该行是场名（切换本组场）
                if toks[0] in fields:
                    cur_field = fields[toks[0]]
                continue
            # <单元名> <nNode> <paramNames...>
            eleName = toks[0]
            nNode = int(toks[1])
            paramNames = toks[2:]
            ele = DataEleSubG(eleName, nNode)
            ele.paramNames = paramNames
            ele.dispNames = list(cur_field._preDispNames)
            cur_field.addEleSub(ele)
            continue
        if mode == "matedata":
            # <场名> <单元名>，下一非空行是数值
            if len(toks) >= 2 and toks[0] in fields and toks[1]:
                fname, eleName = toks[0], toks[1]
                # 读下一行数值
                vals = []
                while i < len(lines):
                    nl = lines[i].strip()
                    i += 1
                    if nl == "" or nl == "#":
                        break
                    try:
                        vals = [float(x) for x in nl.split()]
                        break
                    except ValueError:
                        break
                fld = fields[fname]
                for e in fld.eleSubs:
                    if e.name == eleName:
                        e.paramValues = vals
                        break
            continue
    # 用 ges 填充积分/形函数/结果名
    for fld in project.fields:
        for e in fld.eleSubs:
            e.project = fld
            parseGes(projDir, e.name, e)
            e.inferVTKCellType()
    return project


def parseGcn(projDir, projName, project):
    """解析 <projName>.gcn → project.caculateCode。
    gcn 格式（el）：
      defi
      a ell              ← 场名 a（对应 _name），求解器 ell
      b str a            ← 场名 b，求解器 str，依赖 a
      START a
      SOLVc a            ← 标准椭圆
      SOLVstr b a        ← 最小二乘（b 依赖 a）
      gidres(coor0);
    渲染为按声明顺序的各场调用（场名取 gcn 的 a/b，映射到 project.fields 的 _name）。
    """
    path = os.path.join(projDir, projName, projName + ".gcn")
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n").strip() for ln in f.readlines()]
    # defi 段：场标识+求解器+依赖（b str a → b 依赖 a）
    keysOrder = []
    inDefi = False
    for ln in lines:
        if ln == "defi":
            inDefi = True
            continue
        if inDefi:
            if ln == "" or ln.startswith("START") or ln.startswith("SOLV") or ln.startswith("gidres"):
                break
            toks = ln.split()
            if len(toks) >= 2:
                keysOrder.append(toks[0])
    # gcn 的 a/b 映射到 project.fields：按声明顺序对应
    fieldList = project.fields
    keyToField = {}
    for idx, k in enumerate(keysOrder):
        if idx < len(fieldList):
            keyToField[k] = fieldList[idx]
    # 渲染 caculateCode：每场 initMatrix→eProgram→solve→uPhy
    linesOut = []
    for k in keysOrder:
        fld = keyToField.get(k)
        if not fld:
            continue
        var = fld.fieldDataClassName[0].lower() + fld.fieldDataClassName[1:]  # 小驼峰实例名
        linesOut.append(f"        {var}->initMatrix();")
        linesOut.append(f"        {var}->eProgram();")
        linesOut.append(f"        {var}->solve();")
        linesOut.append(f"        {var}->uPhy();")
    project.caculateCode = "\n".join(linesOut) + "\n"


def _inferType(nNode, dim, coordVars):
    """由节点数+维度推断单元几何类型：0点/1线/2面/3体。"""
    if dim == 1:
        return 1
    if dim == 2:
        return 2 if nNode >= 3 else 1     # >=3节点为面，否则线
    if dim == 3:
        return 3 if nNode >= 5 else 2
    return 1


def parseGes(projDir, eleName, ele):
    """解析 <eleName>.ges -> 填充 gaussPoints/Weights/shapeFuns/eleResNames/type。
    ges 关键段：
      node N          -> 校验 nNodes
      refc rx,ry,     -> coordVars（参考坐标名）
      coor x,y,       -> 维度
      gaus = N        + 下 N 行: rx ry weight（2D）/ rx weight（1D）
      shap 段         -> 形函数表达式（取 disp 的第一个分量的 N 个表达式）
      func = a,b,c    -> eleResNames
      coef u,v        -> 记录到 ele._gesCoefVars（依赖其他场位移）
    """
    path = os.path.join(projDir, ele.project.project.name, eleName + ".ges")
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f.readlines()]
    gaus_n = 0
    gaus_lines = []
    shap_exprs = []
    in_shap = False
    ges_coef_vars = []
    # 收集 nrefc（参考坐标维度）
    nrefc = 0
    for idx, raw in enumerate(lines):
        s = raw.strip()
        if s == "":
            continue
        # refc rx,ry, -> 推断参考坐标维度
        if s.startswith("refc"):
            toks_refc = s.replace(",", " ").split()
            # refc 后面是坐标名列表，过滤 "refc" 本身
            nrefc = len([t for t in toks_refc[1:] if t])
            continue
        # gaus = N 或 gaus  N
        if s.startswith("gaus"):
            toks = s.replace("=", " ").split()
            toks = [t for t in toks if t]  # 过滤空串（"gaus = 4"产生空串）
            if len(toks) >= 2:
                gaus_n = int(toks[1])
            # 收集后续 gaus_n 个数值行
            cnt = 0
            j = idx + 1
            while j < len(lines) and cnt < gaus_n:
                lj = lines[j].strip()
                j += 1
                if lj == "":
                    continue
                try:
                    vals = [float(x) for x in lj.split()]
                    gaus_lines.append(vals)
                    cnt += 1
                except ValueError:
                    break
            continue
        # shap 段开始
        if s.startswith("shap"):
            in_shap = True
            continue
        if in_shap:
            # 分量标题行（u= / v= / dxx= / dxy= 等）：以字母开头、以=结尾、无空格
            if s.endswith("=") and "=" not in s[:-1]:
                continue
            # 表达式行（u1 = (...)）：含=但不以=结尾
            if "=" in s and not s.endswith("="):
                expr = s.split("=", 1)[1].strip()
                if expr:
                    shap_exprs.append(expr)
                continue
            # 遇到其他 ges 段关键字结束 shap 收集
            if any(s.startswith(k) for k in
                   ("tran", "func", "dist", "mass", "load", "end",
                    "coef", "refc", "coor", "node", "mate", "gaus")):
                in_shap = False
            continue
        # func = exx,eyy,exy, （defi 头部声明）
        if s.startswith("func") and "=" in s:
            rhs = s.split("=", 1)[1]
            ele.eleResNames = [x.strip() for x in rhs.split(",") if x.strip()]
            continue
        # coef u,v, （defi 头部声明，无=号）
        if s.startswith("coef") and not s.startswith("coef="):
            rhs = s[len("coef"):].strip()
            if rhs:
                # "u,v," -> 过滤空串
                ges_coef_vars = [x.strip() for x in rhs.replace(",", " ").split()
                                 if x.strip()]
            continue
    # 填充积分点（每行前 nrefc 个为坐标，末个为权重）
    ele.gaussPoints = []
    ele.gaussWeights = []
    if gaus_lines:
        actual_nrefc = len(gaus_lines[0]) - 1 if len(gaus_lines[0]) > 1 else 1
        if nrefc == 0:
            nrefc = actual_nrefc  # 若 refc 未解析到，用积分点行推断
        for gl in gaus_lines:
            ele.gaussPoints.append(gl[:nrefc])
            ele.gaussWeights.append(gl[-1])
    # 形函数：rx->x[1], ry->x[2] 占位符（pyTool _replaceCoordVars 会替换）
    # 只取与节点数相等的前 nNode 个表达式
    def _conv(expr):
        return expr.replace("rx", "x[1]").replace("ry", "x[2]")
    ele.shapeFuns = [_conv(e) for e in shap_exprs[:ele.nNodes]]
    # type 推断：含 coef 先假设线单元，再按 nNode>=3 修正回面单元
    if ges_coef_vars:
        ele._gesCoefVars = ges_coef_vars
    if ges_coef_vars:
        ele.type = 1  # 含 coef 的依赖单元多为线单元（beq4g2 修正回 2）
    inferred = _inferType(ele.nNodes, ele.project.project.dim, ele.coordVars)
    if ele.type == 1 and ele.nNodes >= 3 and ele.project.project.dim == 2:
        ele.type = 2  # beq4g2（4节点面单元含 coef）修正回面
    elif not ges_coef_vars:
        ele.type = inferred
    if ele.type == 1:
        ele.bBC = True
    # coordVars
    if nrefc == 1:
        ele.coordVars = ["x"]
    elif nrefc >= 2:
        ele.coordVars = ["x", "y"]
