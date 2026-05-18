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
#include "Truss1DDispFieldData.h"
#include "Truss1DData.h"
int makeData(Truss1DData& data1) {
    // TODO: 添加测试数据
	// 节点信息
	data1.addNode(1, 0.0);
	data1.addNode(2, 0.6);
	data1.addNode(3, 1.2);
	data1.addNode(4, 1.8);
	data1.addNodeEnd();
	// 单元信息
	data1.addEle(1, { 1,2 }, "Truss1D");
	data1.addEle(2, { 2,3 },"Truss1D");
	data1.addEle(3, { 3,4 }, "Truss1D");
	//材料信息
	std::map<std::string, double> param1;
	param1["E"] = 2e11;
	param1["A"] = 6e-4;
	data1.addMate(param1);
	std::map<std::string, double> param2;
	param2["E"] = 1e11;
	param2["A"] = 12e-4;
	data1.addMate(param2);
	// 单元材料信息
	data1.setEleMateId(1, 0);
	data1.setEleMateId(2, 0);
	data1.setEleMateId(3, 1);
	Truss1DDispFieldData* phydata = static_cast<Truss1DDispFieldData*>(data1._phyDatas[0]);
	phydata->setFirstBoundry(1, 0.0);
	phydata->setFirstBoundry(4, 0.0);
	phydata->setSecondBoundry(2, 15000);

    return 0;
}

int main() {
    Truss1DData data;
    makeData(data);
    data.caculate();
    return 0;
}