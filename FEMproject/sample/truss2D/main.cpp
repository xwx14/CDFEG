// SPDX-License-Identifier: GPL-3.0
// This file is part of CDFEG.
//
// CDFEG is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDFEG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.

#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
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
int makeData2(Truss2DData& data) {
    return 1;
}
int main() {
    Truss2DData data;
    makeData(data);
    data.caculate();

    Truss2DDispFieldData* phy = static_cast<Truss2DDispFieldData*>(data._phyDatas[0]);

    std::map<int, int> nodeProgToFile;
    for (const auto& it : data._nodeIdMap)
        nodeProgToFile[it.second] = it.first;

    std::ofstream fout("Truss2D.txt");
    fout << std::setprecision(6) << std::fixed;
    fout << "========== 节点位移 ==========" << std::endl;
    fout << "节点ID\t\tx\t\ty\t\tu\t\tv" << std::endl;
    for (int i = 0; i < data._nPts; ++i) {
        int fileId = nodeProgToFile[i];
        double x = data._nodes[i * 2];
        double y = data._nodes[i * 2 + 1];
        double u = phy->_nodeRes["u"][i];
        double v = phy->_nodeRes["v"][i];
        fout << fileId << "\t\t" << x << "\t\t" << y << "\t\t" << u << "\t\t" << v << std::endl;
    }

    std::map<int, int> eleProgToFile;
    for (const auto& it : data._eleIdMap)
        eleProgToFile[it.second] = it.first;

    fout << std::endl;
    fout << "========== 单元内力 ==========" << std::endl;
    fout << "单元ID\t\t轴力T\t\t应力sigma" << std::endl;
    for (int i = 0; i < data._nElem; ++i) {
        int fileId = eleProgToFile[i];
        double T = phy->_elemRes["T"][i];
        double sigma = phy->_elemRes["sigma"][i];
        fout << fileId << "\t\t" << T << "\t\t" << sigma << std::endl;
    }
    fout.close();

    return 0;
}