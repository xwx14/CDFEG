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

from DataField import DataField

class DataProject:
    def __init__(self, name: str = "", dim: int = 2):
        self.name = name
        # 总体坐标维度
        self.dim = dim
        self.gidName = name
        # 坐标变量名称列表，默认为 ['x', 'y']，根据维度自动调整
        self.coordVars = ['x', 'y', 'z'][:dim]
        # 单元类型
        self.eleType = []
        # 场
        self.fields = []
        # 求解命令流
        self.cmds=[]
        # caculate代码
        self.caculateCode = ""
        # 结构为多个{"name":"pr1","params":[]}
        self.preParams=[]
        # 材料本构类型表：[{name, params, defaults}]
        self.mateTypes = []
        # GiD 后处理输出项：[{name, type, location, vals:[(fieldIndex,valName),...]}]
        self.outputItems = []

    def addMateType(self, name, params, defaults=None):
        """注册材料本构类型（参数 schema），供多个单元共享。"""
        self.mateTypes.append({'name': name, 'params': list(params),
                               'defaults': list(defaults or [])})

    def addOutputItem(self, name, type, location="OnNodes", vals=None):
        """注册一个 GiD 后处理输出项，生成 main 中 ResItem 注册语句。

        Args:
            name: 结果名（如 "disp"、"stress"）
            type: ResType 枚举名（如 "Vector"、"Vector3"、"Scalar"、"Matrix"）
            location: "OnNodes"（默认，节点结果，生成代码写入 data._prePostConfig._nodeResItems）
                       或 "OnElements"（单元结果，写入 data._prePostConfig._eleResItems）
            vals: 分量列表 [(fieldIndex, valName), ...]，如 [(0,"u"),(0,"v")]
        """
        self.outputItems.append({
            'name': name,
            'type': type,
            'location': location,
            'vals': [tuple(v) for v in (vals or [])]
        })

    def addField(self,field0):
        if type(field0) == str:
            field = DataField(field0)
        elif isinstance(field0, DataField):
            field=field0
        else:
            return None
        field.project=self
        self.fields.append(field)
        return field

    def makeData(self):
        for field in self.fields:
            field.makeData()
        # 建立本构表：单物理场用单元 name，多物理场用 gidName（同 key 保序合并去重）
        self.mateTypes = []
        multiField = len(self.fields) > 1
        for field in self.fields:
            for ele in field.eleSubs:
                constitutiveName = ele.gidName
                ele.mateTypeName = constitutiveName
                self._mergeOrAddMateType(constitutiveName, ele.paramNames, ele.paramValues)

    def _mergeOrAddMateType(self, name, params, values):
        """注册或合并本构类型：同名保序合并去重，否则新增。"""
        for mt in self.mateTypes:
            if mt['name'] == name:
                seen = set(mt['params'])
                for i, p in enumerate(params):
                    if p not in seen:
                        seen.add(p)
                        mt['params'].append(p)
                        mt['defaults'].append(values[i] if i < len(values) else 0.0)
                return
        self.addMateType(name, params, values)

    def toDict(self):
        """
        将项目数据转换为字典

        对于每个场，会添加项目级别的属性（如 femDataClassName）。
        """
        # 收集所有场的字典数据
        fields_data = []
        for field in self.fields:
            field_dict = field.toDict()
            fields_data.append(field_dict)
        return {
            'name': self.name,
            'dim': self.dim,
            'coordVars': self.coordVars,
            'eleType': self.eleType,
            'fields': fields_data,
            'caculateCode': self.caculateCode,
            # 预处理参数，结构为多个{"name":"pr1","params":[]}
            'preParams': self.preParams,
            'mateTypes': self.mateTypes,
            'outputItems': self.outputItems,
        }

    @classmethod
    def fromDict(cls, data: dict):
        """
        从字典构建项目对象
        Args:
            data: 包含项目数据的字典

        Returns:
            DataProject 对象
        """
        # 创建实例
        project = cls(data.get('name', ''), data.get('dim', 2))

        project.coordVars = data.get('coordVars', ['x', 'y', 'z'][:project.dim])
        project.eleType = data.get('eleType', [])

        # 恢复场
        if 'fields' in data:
            for field_data in data['fields']:
                field = DataField.fromDict(field_data)
                project.addField(field)

        # 恢复caculateCode
        project.caculateCode = data.get('caculateCode', '')

        # 恢复预处理参数（结构为多个{"name":"pr1","params":[]}）
        project.preParams = data.get('preParams', [])

        # 恢复材料本构类型表
        project.mateTypes = data.get('mateTypes', [])

        # 恢复 GiD 后处理输出项
        project.outputItems = data.get('outputItems', [])

        return project

