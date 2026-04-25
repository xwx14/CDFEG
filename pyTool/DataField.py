from DataEleSub import DataEleSub
from DataSch import DataSch

class DataField:
    def __init__(self, name: str = ""):
        self.name=name
        self.fieldDataClassName = f"{self.name}FieldData"
        self.eleSubs=[]
        self.eleTypes=[]
        # 1椭圆型（K），2抛物型(K/M),3双曲型(K/M/D)
        self.pdeType=1
        # 广义位移，需通过遍历self.eleSubs获得
        self.dispNames=[]
        self.project=None
        self.index=1
        self.bDynamic=False
        # 求解方案
        self.sch = DataSch()
    def addEleSub(self,ele0):
        if type(ele0)==DataEleSub:
            ele=ele0
        elif type(ele0)==str:
            ele=DataEleSub(ele0)
        else:
            return None
        self.eleSubs.append(ele)
        for dispName in ele.dispNames:
            if dispName not in self.dispNames:
                self.dispNames.append(dispName)

    def makeData(self):
        self.dispNames.clear()
        for ele in self.eleSubs:
            for disp in ele.dispNames:
                if disp not in self.dispNames:
                    self.dispNames.append(disp)
    def toDict(self):
        """
        将场数据转换为字典

        注意：
        - fieldDataClassName: 在 __init__ 中设置，此处直接使用
        - femDataClassName: 项目数据类名（如 ElasticData），由 DataProject.toDict() 添加
        - dof: 在调用 toDict() 时动态计算
        """
        # 动态计算 dof
        self.dof = len(self.dispNames)

        return {
            'name': self.name,
            'eleSubs': [ele.toDict() for ele in self.eleSubs],
            'eleTypes': self.eleTypes,
            'pdeType': self.pdeType,
            'dispNames': self.dispNames,
            'index': self.index,
            'bDynamic': self.bDynamic,
            'sch': self.sch.toDict(),
            # C++ 生成属性
            'fieldDataClassName': self.fieldDataClassName,  # 已在 __init__ 中设置
            'dof': self.dof,  # 动态计算
            # 可选属性（由 MakerCpp 设置）
            'dof2': getattr(self, 'dof2', None),
            'headerGuard': getattr(self, 'headerGuard', None),
            'eleResNames': getattr(self, 'eleResNames', None)
            # 注意：femDataClassName 由 DataProject.toDict() 添加
        }

    @classmethod
    def fromDict(cls, data: dict):
        """
        从字典构建场对象

        Args:
            data: 包含场数据的字典

        Returns:
            DataField 对象
        """
        # 创建实例
        field = cls(data.get('name', ''))

        # 恢复基本属性
        field.eleTypes = data.get('eleTypes', [])
        field.pdeType = data.get('pdeType', 1)
        field.dispNames = data.get('dispNames', [])
        field.index = data.get('index', 1)
        field.bDynamic = data.get('bDynamic', False)

        # 恢复可选属性
        if 'dof2' in data and data['dof2'] is not None:
            field.dof2 = data['dof2']
        if 'headerGuard' in data and data['headerGuard'] is not None:
            field.headerGuard = data['headerGuard']
        if 'eleResNames' in data and data['eleResNames'] is not None:
            field.eleResNames = data['eleResNames']

        # 恢复单元子程序
        if 'eleSubs' in data:
            for ele_data in data['eleSubs']:
                ele = DataEleSub.fromDict(ele_data)
                field.eleSubs.append(ele)

        # 恢复求解方案
        if 'sch' in data:
            field.sch = DataSch.fromDict(data['sch'])

        return field

