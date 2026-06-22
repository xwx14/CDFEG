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
//
// getCoef 跨场取数测试：验证 key 格式"场名::变量名"、越界兜底 0、跨场收集
#include "CDFEG/PhyFieldData.h"
#include "CDFEG/FemData.h"
#include <iostream>
#include <vector>
#include <string>
int main() {
    using namespace CDFEG;
    FEMData* fd = new FEMData();
    fd->setNPts(4);                 // 4 节点
    fd->addNodeEnd();
    PhyFieldData* ela = new PhyFieldData(2, fd);   // dof=2
    ela->_name = "ela";
    ela->_nodeRes["u"] = {1.0, 2.0, 3.0, 4.0};
    ela->_nodeRes["v"] = {10.0, 20.0, 30.0, 40.0};
    PhyFieldData* elb = new PhyFieldData(3, fd);   // dof=3
    elb->_name = "elb";
    elb->_nodeRes["dxx"] = {};   // 空向量 → 所有节点取值兜底 0
    fd->_phyDatas.push_back(ela);
    fd->_phyDatas.push_back(elb);
    // elb 调 getCoef 取所有场（含 ela）的 _nodeRes
    auto coef = elb->getCoef({0, 1, 3});
    bool ok = true;
    ok &= (coef.count("ela::u") && coef["ela::u"] == std::vector<double>{1.0, 2.0, 4.0});
    ok &= (coef.count("ela::v") && coef["ela::v"] == std::vector<double>{10.0, 20.0, 40.0});
    ok &= (coef.count("elb::dxx") && coef["elb::dxx"] == std::vector<double>{0.0, 0.0, 0.0}); // elb 未填 _nodeRes → 兜底 0
    // 越界节点（nodeId=9 超出 4 节点）→ 兜底 0
    auto coef2 = elb->getCoef({0, 9});
    ok &= (coef2["ela::u"] == std::vector<double>{1.0, 0.0});
    std::cout << (ok ? "TEST PASS" : "TEST FAIL") << std::endl;
    fd->_phyDatas.clear();   // 清除悬空指针，防止 FEMData 析构时访问已释放内存
    delete elb;
    delete ela;
    delete fd;
    return ok ? 0 : 1;
}
