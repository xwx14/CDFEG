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

# 工程文件读写：load 时按 baseClass 分发重建 DataEleSubG/DataEleSub，
# 绕开 DataField.fromDict 的基类退化（其内部写死 DataEleSub.fromDict）。
import json

from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from DataEleSubG import DataEleSubG
from DataSch import DataSch

EXT = ".cdfeg.json"


def save(project: DataProject, path: str) -> None:
    """保存为 .cdfeg.json（toDict 基础 + cmds 补充）。"""
    data = project.toDict()
    data["cmds"] = project.cmds  # toDict 不含 cmds，手动补上
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=4, ensure_ascii=False)


def _buildEle(ele_data: dict):
    """按 baseClass / 是否含 gaussPoints 判断单元子类。"""
    if ele_data.get("baseClass") == "IsoEleBase" or "gaussPoints" in ele_data:
        return DataEleSubG.fromDict(ele_data)
    return DataEleSub.fromDict(ele_data)


def load(path: str) -> DataProject:
    """从 .cdfeg.json 重建 DataProject（G 单元高斯字段不丢失）。"""
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)  # 格式损坏时抛异常，由调用方处理

    project = DataProject(data.get("name", ""), data.get("dim", 2))
    project.coordVars = data.get("coordVars", ["x", "y", "z"][:project.dim])
    project.eleType = data.get("eleType", [])
    project.caculateCode = data.get("caculateCode", "")
    project.preParams = data.get("preParams", [])

    for field_data in data.get("fields", []):
        field = DataField(field_data.get("name", ""))
        field.eleTypes = field_data.get("eleTypes", [])
        field.pdeType = field_data.get("pdeType", 1)
        field.index = field_data.get("index", 1)
        field.bDynamic = field_data.get("bDynamic", False)
        field.preParams = field_data.get("preParams", [])
        for ele_data in field_data.get("eleSubs", []):
            field.eleSubs.append(_buildEle(ele_data))
        if "sch" in field_data:
            field.sch = DataSch.fromDict(field_data["sch"])
        field.makeData()  # 重新聚合 dispNames / eleResNames
        project.addField(field)

    project.cmds = data.get("cmds", [])

    return project
