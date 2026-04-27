#include <iostream>
#include "Truss3DDispFieldData.h"
#include "Truss3DData.h"

// 3D空间桁架示例
// 创建一个简单的3D悬臂桁架结构
int makeData(Truss3DData& data) {
    // 节点信息 (x, y, z坐标)
    // 节点1: 固定端支座 (0, 0, 0)
    data.addNode(1, 0.0, 0.0, 0.0);
    // 节点2: (0, 0, 3) - 沿z轴方向
    data.addNode(2, 0.0, 0.0, 3.0);
    // 节点3: (3, 0, 0) - 沿x轴方向
    data.addNode(3, 3.0, 0.0, 0.0);
    // 节点4: (3, 0, 3) - 对角顶点
    data.addNode(4, 3.0, 0.0, 3.0);
    // 节点5: (0, 4, 0) - 沿y轴方向
    data.addNode(5, 0.0, 4.0, 0.0);
    // 节点6: (0, 4, 3) - y和z方向
    data.addNode(6, 0.0, 4.0, 3.0);
    // 节点7: (3, 4, 0) - x和y方向
    data.addNode(7, 3.0, 4.0, 0.0);
    // 节点8: (3, 4, 3) - 自由端 (3,4,3)
    data.addNode(8, 3.0, 4.0, 3.0);
    data.addNodeEnd();

    // 单元信息 (连接两个节点)
    // 底部边框
    data.addEle(1, { 1, 3 }, "Truss3D");  // 1->3
    data.addEle(2, { 3, 7 }, "Truss3D");  // 3->7
    data.addEle(3, { 7, 5 }, "Truss3D");  // 7->5
    data.addEle(4, { 5, 1 }, "Truss3D");  // 5->1
    // 顶部边框
    data.addEle(5, { 2, 4 }, "Truss3D");  // 2->4
    data.addEle(6, { 4, 8 }, "Truss3D");  // 4->8
    data.addEle(7, { 8, 6 }, "Truss3D");  // 8->6
    data.addEle(8, { 6, 2 }, "Truss3D");  // 6->2
    // 垂直杆件
    data.addEle(9, { 1, 2 }, "Truss3D");  // 1->2
    data.addEle(10, { 3, 4 }, "Truss3D"); // 3->4
    data.addEle(11, { 7, 8 }, "Truss3D"); // 7->8
    data.addEle(12, { 5, 6 }, "Truss3D"); // 5->6
    // 对角支撑
    data.addEle(13, { 1, 4 }, "Truss3D"); // 对角支撑1
    data.addEle(14, { 3, 6 }, "Truss3D"); // 对角支撑2
    data.addEle(15, { 7, 2 }, "Truss3D"); // 对角支撑3
    data.addEle(16, { 5, 8 }, "Truss3D"); // 对角支撑4

    // 材料信息
    std::map<std::string, double> param;
    param["E"] = 2e11;       // 弹性模量 (Pa)
    param["A"] = 1e-4;      // 横截面积 (m^2)
    data.addMate(param);

    // 单元材料信息
    for (int i = 1; i <= 16; i++) {
        data.setEleMateId(i, 0);
    }

    Truss3DDispFieldData* phydata = static_cast<Truss3DDispFieldData*>(data._phyDatas[0]);

    // 边界条件设置 (第一类边界条件 - 固定位移)
    // 固定节点1的所有自由度 (u=0, v=0, w=0)
    phydata->setFirstBoundry(1, 0.0, 0);  // u = 0
    phydata->setFirstBoundry(1, 0.0, 1);  // v = 0
    phydata->setFirstBoundry(1, 0.0, 2);  // w = 0

    // 荷载条件设置 (第二类边界条件 - 施加荷载)
    // 在节点8施加荷载: Fx = 10000N, Fy = 5000N, Fz = -3000N
    phydata->setSecondBoundry(8, 10000.0, 0);  // Fx = 10000N
    phydata->setSecondBoundry(8, 5000.0, 1);   // Fy = 5000N
    phydata->setSecondBoundry(8, -3000.0, 2);   // Fz = -3000N

    return 0;
}

int main() {
    Truss3DData data;
    makeData(data);
    data.caculate();
    return 0;
}
