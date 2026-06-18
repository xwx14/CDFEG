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

#include <iostream>
#include "NewmarkData.h"
#include "NewmarkDispFieldData.h"
#include "CDFEG/gidPrePost.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <project> <path>" << std::endl;
		return 1;
	}
    std::string project = argv[1];
    std::string path = argv[2];

    NewmarkData data;
    CDFEG::GidPrePost gidPrePost(&data);
    gidPrePost.setFilePath(path, project);
    gidPrePost.pre();

    // 前处理完成后设置初值（按程序内节点号，此处示例置零，可按需调用）
    NewmarkDispFieldData* field = static_cast<NewmarkDispFieldData*>(data._phyDatas[0]);
    // 例：初值全零；如有初速度/初加速度可在此循环调用 field->setInitialVel / setInitialAcc

    // 注册 GiD 结果项：位移、速度、加速度、应力（按节点输出）
    // post2 内按 GidResItem._iFields 指向物理场、_ValNames 指向节点结果列
    CDFEG::GidResItem dispItem("disp", CDFEG::GidResultType::Vector);
    dispItem.addVal(0, "u");
    dispItem.addVal(0, "v");
    gidPrePost._resItems.push_back(dispItem);

    CDFEG::GidResItem velItem("velocity", CDFEG::GidResultType::Vector);
    velItem.addVal(0, "velU");
    velItem.addVal(0, "velV");
    gidPrePost._resItems.push_back(velItem);

    CDFEG::GidResItem accItem("acceleration", CDFEG::GidResultType::Vector);
    accItem.addVal(0, "accU");
    accItem.addVal(0, "accV");
    gidPrePost._resItems.push_back(accItem);

    CDFEG::GidResItem stressItem("stress", CDFEG::GidResultType::Matrix);
    stressItem.addVal(0, "sigmaXX");
    stressItem.addVal(0, "sigmaYY");
    stressItem.addVal(0, "sigmaXY");
    gidPrePost._resItems.push_back(stressItem);

    // 将后处理器交给数据类，供 caculate 每步调用 post2 输出
    data.setPost(&gidPrePost);

    data.caculate();
    return 0;
}
