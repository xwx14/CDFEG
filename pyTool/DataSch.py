from typing import Dict, Any


class DataSch:
    """求解方案类"""

    def __init__(self):
        pass

    def toDict(self) -> Dict[str, Any]:
        return {}

    @classmethod
    def fromDict(cls, data: Dict[str, Any]) -> "DataSch":
        return cls()
