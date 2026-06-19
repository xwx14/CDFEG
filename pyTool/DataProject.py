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
            'preParams': self.preParams
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

        return project

