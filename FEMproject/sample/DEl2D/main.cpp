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
#include "del2dData.h"
#include "DelDispFieldData.h"
#include "CDFEG/gidPrePost.h"
#include "CDFEG/vtkPost.h"
#include "CDFEG/Processor.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <project> <path>" << std::endl;
		return 1;
	}
    std::string project = argv[1];
    std::string path = argv[2];

    del2dData data;
    CDFEG::GidPrePost gidPrePost(&data);
    gidPrePost.setFilePath(path, project);
    gidPrePost.pre();

    // VTK 后处理器：与 GidPrePost 并存，输出 del2d_<it>.vtu + del2d.pvd 多步时间序列
    CDFEG::vtkPost vtkpost(&data);
    vtkpost.setFilePath(path, project);

    // 前处理完成后设置初值（按程序内节点号，此处示例置零，可按需调用）
    DelDispFieldData* field = static_cast<DelDispFieldData*>(data._phyDatas[0]);
    // 例：初值全零；如有初速度/初加速度可在此循环调用 field->setInitialVel / setInitialAcc

    // 注册结果项：位移、速度、加速度、应力（写入 _prePostConfig，所有 processor 共享）
    auto registerItems = [&data]() {
        CDFEG::ResItem dispItem("disp", CDFEG::ResType::Vector);
        dispItem.addVal(0, "u");
        dispItem.addVal(0, "v");
        data._prePostConfig._nodeResItems.push_back(dispItem);

        CDFEG::ResItem velItem("velocity", CDFEG::ResType::Vector);
        velItem.addVal(0, "velU");
        velItem.addVal(0, "velV");
        data._prePostConfig._nodeResItems.push_back(velItem);

        CDFEG::ResItem accItem("acceleration", CDFEG::ResType::Vector);
        accItem.addVal(0, "accU");
        accItem.addVal(0, "accV");
        data._prePostConfig._nodeResItems.push_back(accItem);

        // 节点应力（OnNodes，与原 main 一致）
        CDFEG::ResItem stressItem("stress", CDFEG::ResType::Matrix);
        stressItem.addVal(0, "sigmaXX");
        stressItem.addVal(0, "sigmaYY");
        stressItem.addVal(0, "sigmaXY");
        data._prePostConfig._nodeResItems.push_back(stressItem);

        // 单元应力（OnElements，单元平均；动力学每步输出）
        CDFEG::ResItem eleStress("eleStress", CDFEG::ResType::Matrix, CDFEG::ResLocation::OnElements);
        eleStress.addVal(0, "sigmaXX");
        eleStress.addVal(0, "sigmaYY");
        eleStress.addVal(0, "sigmaXY");
        data._prePostConfig._eleResItems.push_back(eleStress);
        // 单元体积（OnElements）
        CDFEG::ResItem eleVolume("eleVolume", CDFEG::ResType::Scalar, CDFEG::ResLocation::OnElements);
        eleVolume.addVal(0, "volume");
        data._prePostConfig._eleResItems.push_back(eleVolume);
    };
    registerItems();

    data.caculate();   // 内部 post(it) 同时驱动 GidPrePost(res) + vtkPost(vtu/pvd)
    return 0;
}
