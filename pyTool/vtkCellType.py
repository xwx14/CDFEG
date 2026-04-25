"""
VTK 单元类型枚举

根据 include/SIFEG/vtkCellType.h 生成的 Python 枚举类
用于指定有限元单元的 VTK 可视化类型

作者：系统自动生成
日期：2025-01-06
"""
from enum import IntEnum


class VTKCellType(IntEnum):
    """
    VTK 单元类型枚举

    定义可视化工具包 (VTK) 中支持的所有单元类型。
    用于有限元后处理时的 VTK 文件输出。
    """

    # ========== 线性单元 ==========

    VTK_EMPTY_CELL = 0  # 空单元
    VTK_VERTEX = 1  # 顶点（点单元）
    VTK_POLY_VERTEX = 2  # 多顶点
    VTK_LINE = 3  # 线段（2节点）
    VTK_POLY_LINE = 4  # 多段线
    VTK_TRIANGLE = 5  # 三角形（3节点）
    VTK_TRIANGLE_STRIP = 6  # 三角形带
    VTK_POLYGON = 7  # 多边形
    VTK_PIXEL = 8  # 像素（四边形）
    VTK_QUAD = 9  # 四边形（4节点）
    VTK_TETRA = 10  # 四面体（4节点）
    VTK_VOXEL = 11  # 体素（六面体）
    VTK_HEXAHEDRON = 12  # 六面体（8节点）
    VTK_WEDGE = 13  # 楔形体（6节点）
    VTK_PYRAMID = 14  # 金字塔（5节点）
    VTK_PENTAGONAL_PRISM = 15  # 五边形棱柱
    VTK_HEXAGONAL_PRISM = 16  # 六边形棱柱

    # ========== 二次等参单元 ==========

    VTK_QUADRATIC_EDGE = 21  # 二次边（3节点）
    VTK_QUADRATIC_TRIANGLE = 22  # 二次三角形（6节点）
    VTK_QUADRATIC_QUAD = 23  # 二次四边形（8节点）
    VTK_QUADRATIC_POLYGON = 36  # 二次多边形
    VTK_QUADRATIC_TETRA = 24  # 二次四面体（10节点）
    VTK_QUADRATIC_HEXAHEDRON = 25  # 二次六面体（20节点）
    VTK_QUADRATIC_WEDGE = 26  # 二次楔形体（15节点）
    VTK_QUADRATIC_PYRAMID = 27  # 二次金字塔（13节点）
    VTK_BIQUADRATIC_QUAD = 28  # 双二次四边形（9节点）
    VTK_TRIQUADRATIC_HEXAHEDRON = 29  # 三二次六面体（27节点）
    VTK_TRIQUADRATIC_PYRAMID = 37  # 三二次金字塔
    VTK_QUADRATIC_LINEAR_QUAD = 30  # 二次线性四边形
    VTK_QUADRATIC_LINEAR_WEDGE = 31  # 二次线性楔形体
    VTK_BIQUADRATIC_QUADRATIC_WEDGE = 32  # 双二次二次楔形体
    VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON = 33  # 双二次二次六面体
    VTK_BIQUADRATIC_TRIANGLE = 34  # 双二次三角形

    # ========== 三次等参单元 ==========

    VTK_CUBIC_LINE = 35  # 三次线段（4节点）

    # ========== 特殊单元类型 ==========

    VTK_CONVEX_POINT_SET = 41  # 凸点集
    VTK_POLYHEDRON = 42  # 多面体

    # ========== 参数形式的高阶单元 ==========

    VTK_PARAMETRIC_CURVE = 51  # 参数曲线
    VTK_PARAMETRIC_SURFACE = 52  # 参数曲面
    VTK_PARAMETRIC_TRI_SURFACE = 53  # 参数三角曲面
    VTK_PARAMETRIC_QUAD_SURFACE = 54  # 参数四边形曲面
    VTK_PARAMETRIC_TETRA_REGION = 55  # 参数四面体区域
    VTK_PARAMETRIC_HEX_REGION = 56  # 参数六面体区域

    # ========== 高阶单元 ==========

    VTK_HIGHER_ORDER_EDGE = 60  # 高阶边
    VTK_HIGHER_ORDER_TRIANGLE = 61  # 高阶三角形
    VTK_HIGHER_ORDER_QUAD = 62  # 高阶四边形
    VTK_HIGHER_ORDER_POLYGON = 63  # 高阶多边形
    VTK_HIGHER_ORDER_TETRAHEDRON = 64  # 高阶四面体
    VTK_HIGHER_ORDER_WEDGE = 65  # 高阶楔形体
    VTK_HIGHER_ORDER_PYRAMID = 66  # 高阶金字塔
    VTK_HIGHER_ORDER_HEXAHEDRON = 67  # 高阶六面体

    # ========== 拉格朗日单元 ==========

    VTK_LAGRANGE_CURVE = 68  # 拉格朗日曲线
    VTK_LAGRANGE_TRIANGLE = 69  # 拉格朗日三角形
    VTK_LAGRANGE_QUADRILATERAL = 70  # 拉格朗日四边形
    VTK_LAGRANGE_TETRAHEDRON = 71  # 拉格朗日四面体
    VTK_LAGRANGE_HEXAHEDRON = 72  # 拉格朗日六面体
    VTK_LAGRANGE_WEDGE = 73  # 拉格朗日楔形体
    VTK_LAGRANGE_PYRAMID = 74  # 拉格朗日金字塔

    # ========== 贝塞尔单元 ==========

    VTK_BEZIER_CURVE = 75  # 贝塞尔曲线
    VTK_BEZIER_TRIANGLE = 76  # 贝塞尔三角形
    VTK_BEZIER_QUADRILATERAL = 77  # 贝塞尔四边形
    VTK_BEZIER_TETRAHEDRON = 78  # 贝塞尔四面体
    VTK_BEZIER_HEXAHEDRON = 79  # 贝塞尔六面体
    VTK_BEZIER_WEDGE = 80  # 贝塞尔楔形体
    VTK_BEZIER_PYRAMID = 81  # 贝塞尔金字塔

    VTK_NUMBER_OF_CELL_TYPES = 82  # 单元类型总数


# 常用单元类型映射表
# 根据单元几何类型和节点数快速查找对应的 VTK 类型
VTK_TYPE_MAPPING = {
    # 点单元
    (0, 1): VTKCellType.VTK_VERTEX,

    # 线单元
    (1, 2): VTKCellType.VTK_LINE,  # 2节点线段
    (1, 3): VTKCellType.VTK_QUADRATIC_EDGE,  # 3节点二次线

    # 面单元
    (2, 3): VTKCellType.VTK_TRIANGLE,  # 3节点三角形
    (2, 4): VTKCellType.VTK_QUAD,  # 4节点四边形
    (2, 6): VTKCellType.VTK_QUADRATIC_TRIANGLE,  # 6节点二次三角形
    (2, 8): VTKCellType.VTK_QUADRATIC_QUAD,  # 8节点二次四边形
    (2, 9): VTKCellType.VTK_BIQUADRATIC_QUAD,  # 9节点双二次四边形

    # 体单元
    (3, 4): VTKCellType.VTK_TETRA,  # 4节点四面体
    (3, 5): VTKCellType.VTK_PYRAMID,  # 5节点金字塔
    (3, 6): VTKCellType.VTK_WEDGE,  # 6节点楔形
    (3, 8): VTKCellType.VTK_HEXAHEDRON,  # 8节点六面体
    (3, 10): VTKCellType.VTK_QUADRATIC_TETRA,  # 10节点二次四面体
    (3, 20): VTKCellType.VTK_QUADRATIC_HEXAHEDRON,  # 20节点二次六面体
}


def get_vtk_cell_type(geom_type: int, n_nodes: int) -> VTKCellType:
    """
    根据几何类型和节点数获取对应的 VTK 单元类型

    Args:
        geom_type: 几何类型（0=点，1=线，2=面，3=体）
        n_nodes: 节点数

    Returns:
        对应的 VTK 单元类型枚举值

    Raises:
        ValueError: 如果找不到对应的单元类型
    """
    key = (geom_type, n_nodes)
    if key in VTK_TYPE_MAPPING:
        return VTK_TYPE_MAPPING[key]

    # 如果映射表中没有，尝试返回最接近的类型
    if geom_type == 0:
        return VTKCellType.VTK_VERTEX
    elif geom_type == 1:
        return VTKCellType.VTK_LINE
    elif geom_type == 2:
        if n_nodes <= 4:
            return VTKCellType.VTK_QUAD
        else:
            return VTKCellType.VTK_POLYGON
    elif geom_type == 3:
        if n_nodes == 4:
            return VTKCellType.VTK_TETRA
        elif n_nodes == 5:
            return VTKCellType.VTK_PYRAMID
        elif n_nodes == 6:
            return VTKCellType.VTK_WEDGE
        else:
            return VTKCellType.VTK_HEXAHEDRON

    raise ValueError(f"无法识别的单元类型: geom_type={geom_type}, n_nodes={n_nodes}")


def get_vtk_cell_type_name(vtk_cell_type: VTKCellType) -> str:
    """
    获取 VTK 单元类型的名称（不包含 VTK_ 前缀）

    Args:
        vtk_cell_type: VTK 单元类型枚举

    Returns:
        VTK 类型名称（如 "LINE", "TRIANGLE", "QUAD" 等）
    """
    return vtk_cell_type.name.replace("VTKCellType.", "").replace("VTK_", "")


# 导出列表
__all__ = [
    'VTKCellType',
    'VTK_TYPE_MAPPING',
    'get_vtk_cell_type',
]
