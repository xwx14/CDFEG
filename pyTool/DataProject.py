from DataField import DataField

class DataProject:
    def __init__(self, name: str = "", dim: int = 2):
        self.name = name
        # 总体坐标维度
        self.dim = dim
        # 单元类型
        self.eleType = []
        # 场
        self.fields = []
        # 求解命令流
        self.cmds=[]
        # caculate代码
        self.caculateCode = ""

    def addField(self,field0):
        if type(field0) == str:
            field = DataField(field0)
        elif type(field0) == DataField:
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
            'eleType': self.eleType,
            'fields': fields_data,
            'caculateCode': self.caculateCode
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

        # 恢复单元类型
        project.eleType = data.get('eleType', [])

        # 恢复场
        if 'fields' in data:
            for field_data in data['fields']:
                field = DataField.fromDict(field_data)
                project.addField(field)

        # 恢复caculateCode
        project.caculateCode = data.get('caculateCode', '')

        return project

