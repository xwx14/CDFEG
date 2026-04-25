import sys
sys.path.append("./")
sys.path.append("../")
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from MakerCpp import MakerCpp
# 总体数据
project = DataProject("Truss1D",1)
# 物理场
field = DataField("Truss1DDisp")
# 单元子程序
ele = DataEleSub("Truss1D")
# 添加单元
field.addEleSub(ele)
# 添加场
project.addField(field)
# 生成器
outPath="truss1D"
maker=MakerCpp(project,outPath)
maker.makeAll()