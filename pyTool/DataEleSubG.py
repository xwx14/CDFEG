from DataEleSub import DataEleSub


class DataEleSubG(DataEleSub):
    """
    高斯积分单元数据类

    继承自 DataEleSub，专门用于需要高斯积分的等参单元。
    扩展了高斯积分点的坐标和权重配置。

    Attributes:
        gaussOrder (int): 高斯积分阶数
        gaussPoints (list): 积分点坐标列表，格式为 [[xi,eta,...], ...]
        gaussWeights (list): 对应的权重列表
    """
    def __init__(self, name: str = "", nNode: int = 2, gaussOrder: int = 2):
        """
        初始化高斯积分单元数据

        Args:
            name: 单元名称
            nNode: 节点数
            gaussOrder: 高斯积分阶数，默认为 2
        """
        super().__init__(name, nNode)
        # 高斯积分阶数
        self.gaussOrder = gaussOrder
        # 积分点坐标列表，格式为 [[xi,eta,...], ...]
        self.gaussPoints = []
        # 对应的权重列表
        self.gaussWeights = []
        self.shapeFuns = []  # 形函数列表,格式实例：[["(1. - x[1]) / 2. * (1. - x[2]) / 2."], ...],x1,x2为局部坐标

    def toDict(self):
        """
        序列化为字典（包含高斯积分特有字段）

        Returns:
            dict: 包含所有单元数据的字典
        """
        dict_data = super().toDict()
        dict_data.update({
            'gaussOrder': self.gaussOrder,
            'gaussPoints': self.gaussPoints,
            'gaussWeights': self.gaussWeights,
            'shapeFuns': self.shapeFuns
        })
        return dict_data

    @classmethod
    def fromDict(cls, data: dict):
        """
        从字典构建高斯积分单元对象

        先调用父类的 fromDict 处理公共字段，然后处理高斯积分特有字段。

        Args:
            data: 包含单元数据的字典

        Returns:
            DataEleSubG 对象
        """
        # 先创建父类对象（复用父类的 fromDict 逻辑）
        base_ele = DataEleSub.fromDict(data)

        # 创建子类对象，复制父类属性
        ele = cls(base_ele.name, base_ele.nNodes, data.get('gaussOrder', 2))
        # 复制父类所有属性
        ele.__dict__.update(base_ele.__dict__)

        # 恢复高斯积分特有数据
        ele.gaussPoints = data.get('gaussPoints', [])
        ele.gaussWeights = data.get('gaussWeights', [])
        ele.shapeFuns = data.get('shapeFuns', [])

        return ele
