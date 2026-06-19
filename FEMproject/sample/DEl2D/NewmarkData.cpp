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
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>

#include "NewmarkData.h"
#include "NewmarkDispFieldData.h"
#include "CDFEG/gidPrePost.h"
#include <iostream>

NewmarkData::NewmarkData() {
    _dim = 2;
    // 动力位移场（Newmark-β）
    NewmarkDispFieldData* field = new NewmarkDispFieldData(this);
    _phyDatas.push_back(field);
}

NewmarkData::~NewmarkData() {

}

void NewmarkData::setPost(CDFEG::GidPrePost* prePost)
{
    _prePost = prePost;
}

int NewmarkData::caculate() {
    NewmarkDispFieldData* aField = static_cast<NewmarkDispFieldData*>(_phyDatas[0]);

    // 一次方程编号 + 稀疏骨架初始化（稀疏结构全程不变）
    aField->initMatrix();

    // dt/tmax 来自 dat 的 time 段（FEMData 持有）；gamma/beta 来自 dat 的 newmark 段（物理场 _addParams 声明）
    if (_dt <= 0.0)
    {
        std::cerr << "[NewmarkData] dt <= 0, 请检查 dat 文件 time 段" << std::endl;
        return -1;
    }
    double gamma = aField->getParam("newmark","gamma");
    double beta  = aField->getParam("newmark","beta");
    if (beta <= 0.0)
    {
        std::cerr << "[NewmarkData] beta <= 0, 请检查 dat 文件 newmark 段" << std::endl;
        return -1;
    }
    aField->setNewmarkParams(gamma, beta, _dt);

    int nStep = static_cast<int>(_tMax / _dt + 0.5);
    double time = 0.0;

    // 时间步主循环（对应旧 newmark.c 的 l1 循环）
    for (int it = 0; it < nStep; ++it)
    {
        aField->eProgram();   // 装配有效矩阵 K+a0M+a1C + 有效载荷
        aField->solve();      // Eigen LDLT 求解
        aField->uPhy();       // 更新加速度/速度/位移历史 + 应力恢复

        if (_prePost)
        {
            // 按 GiD 步号输出本步位移/速度/加速度/应力
            _prePost->post2(it);
        }

        time += _dt;
    }

    return 1;
}

int NewmarkData::main() {
    caculate();
    return 1;
}
