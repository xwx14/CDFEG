#include <iostream>
#include "Truss2DDispFieldData.h"
#include "Truss2DData.h"
int makeData(Truss2DData& data) {
    // 节点信息
    data.addNode(1, 0.0, 0.0);
    data.addNode(2, 0.0, 3.0);
    data.addNode(3, 3.0, 3.0);
    data.addNode(4, 3.0, 0.0);
    data.addNodeEnd();
    // 单元信息
    data.addEle(1, { 1,2 }, "Truss2D");
    data.addEle(2, { 1,3 }, "Truss2D");
    data.addEle(3, { 1,4 }, "Truss2D");
    //材料信息
    std::map<std::string, double> param1;
    param1["E"] = 2e11;
    param1["A"] = 6e-4;
    data.addMate(param1);
    // 单元材料信息
    data.setEleMateId(1, 0);
    data.setEleMateId(2, 0);
    data.setEleMateId(3, 0);
    Truss2DDispFieldData* phydata = static_cast<Truss2DDispFieldData*>(data._phyDatas[0]);
    phydata->setFirstBoundry(2, 0.0);
    phydata->setFirstBoundry(3, 0.0);
    phydata->setFirstBoundry(4, 0.0);
    phydata->setFirstBoundry(2, 0.0,1);
    phydata->setFirstBoundry(3, 0.0,1);
    phydata->setFirstBoundry(4, 0.0,1);
    phydata->setSecondBoundry(1, -50000,1);
    return 0;
}

int main() {
    Truss2DData data;
    makeData(data);
    data.caculate();
    return 0;
}