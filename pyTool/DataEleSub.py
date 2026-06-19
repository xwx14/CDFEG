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

from vtkCellType import VTKCellType, get_vtk_cell_type, get_vtk_cell_type_name

class DataEleSub:
    def __init__(self, name: str = "", nNode: int = 2):
        self.name=name
        self.dim=2
        self.coordVars=['x','y']
        # 节点数
        self.nNodes=nNode
        # 广义位移
        self.dispNames=[]
        # 单元变量
        self.eleResNames=[]
        # 参数名称
        self.paramNames=[]
        # 参数默认值
        self.paramValues=[]
        # 是否为边界条件
        self.bBC=False
        # index
        self.index=1
        # 0,点单元；1，线单元；2，面单元；3，体单元
        self.type=1
        # VTK 单元类型（默认为空，可自动推断或手动设置）
        self.vtkCellType=None
        # VTK 单元类型名称（用于代码生成）
        self.vtkCellTypeName=None
        self.runCode=""           # 单元计算代码
        self.uCode=""             # 单元后处理代码
        self.initCode=""          # 构造函数初始化代码
        self.shapeFunCode=""      # 形函数代码（等参单元）
        self.coordTransFunCode="" # 坐标转换函数代码（等参单元）
        self.baseClass="ElementBase"         # 基类名称："EleSubBase" 或 "IsoEleBase"
        self.baseClassParam = f"{self.nNodes}, pData"
        # 根据需要改变
        self.calMatrix=['eload','estif','emass','edamp']
        # 结构为多个{"name":"pr1","params":[]}
        self.preParams=[]

    def inferVTKCellType(self):
        """
        根据单元几何类型和节点数自动推断 VTK 单元类型

        Returns:
            VTKCellType 枚举值
        """
        self.vtkCellType = get_vtk_cell_type(self.type, self.nNodes)
        self.vtkCellTypeName = get_vtk_cell_type_name(self.vtkCellType)
        return self.vtkCellType

    def setVTKCellType(self, vtk_cell_type: VTKCellType):
        """
        手动设置 VTK 单元类型

        Args:
            vtk_cell_type: VTKCellType 枚举值
        """
        self.vtkCellType = vtk_cell_type
        self.vtkCellTypeName = get_vtk_cell_type_name(vtk_cell_type)

    def toDict(self):
        dict_data = {
            'name': self.name,
            'dim': self.dim,
            'coordVars': self.coordVars,
            'nNodes': self.nNodes,
            'dispNames': self.dispNames,
            'eleResNames': self.eleResNames,
            'paramNames': self.paramNames,
            'paramValues': self.paramValues,
            'bBC': self.bBC,
            'index': self.index,
            'type': self.type,
            'runCode': self.runCode,          # 单元计算代码
            'uCode': self.uCode,              # 单元后处理代码
            'initCode': self.initCode,        # 构造函数初始化代码
            'shapeFunCode': self.shapeFunCode,      # 形函数代码
            'coordTransFunCode': self.coordTransFunCode, # 坐标转换函数代码
            'baseClass': self.baseClass,       # 基类名称："EleSubBase" 或 "IsoEleBase"
            'calMatrix':self.calMatrix,
            # 预处理参数，结构为多个{"name":"pr1","params":[]}
            'preParams':self.preParams
        }
        # 如果设置了 vtkCellType，添加到字典中
        if self.vtkCellTypeName is not None:
            dict_data['vtkCellType'] = self.vtkCellTypeName
        return dict_data

    @classmethod
    def fromDict(cls, data: dict):
        """
        从字典构建单元对象

        Args:
            data: 包含单元数据的字典

        Returns:
            DataEleSub 对象
        """
        # 创建实例
        ele = cls(data.get('name', ''), data.get('nNodes', 2))

        # 恢复基本属性
        ele.dim = data.get('dim', 2)
        ele.coordVars = data.get('coordVars', ['x', 'y'])
        ele.dispNames = data.get('dispNames', [])
        ele.eleResNames = data.get('eleResNames', [])
        ele.paramNames = data.get('paramNames', [])
        ele.paramValues = data.get('paramValues', [])
        ele.bBC = data.get('bBC', False)
        ele.index = data.get('index', 1)
        ele.type = data.get('type', 1)

        # 恢复代码片段
        ele.runCode = data.get('runCode', '')
        ele.uCode = data.get('uCode', '')
        ele.initCode = data.get('initCode', '')
        ele.shapeFunCode = data.get('shapeFunCode', '')
        ele.coordTransFunCode = data.get('coordTransFunCode', '')
        ele.baseClass = data.get('baseClass', '')
        ele.calMatrix=data.get('calMatrix', [])
        # 恢复预处理参数（结构为多个{"name":"pr1","params":[]}）
        ele.preParams=data.get('preParams', [])

        # 恢复 VTK 单元类型（如果有）
        if 'vtkCellType' in data:
            ele.vtkCellTypeName = data['vtkCellType']
            # 尝试转换为 VTKCellType 枚举
            try:
                from vtkCellType import VTKCellType
                vtk_type_name = data['vtkCellType']
                # 尝试从枚举名称获取值
                for vtk_type in VTKCellType:
                    if vtk_type.name == vtk_type_name:
                        ele.vtkCellType = vtk_type
                        break
            except (ImportError, ValueError):
                pass  # 如果转换失败，保持为 None

        return ele
    
    def makeCode(self):
        "生成代码，由子类实现"
        pass
