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

#ifndef STRESS_BL2G_H
#define STRESS_BL2G_H
#include "CDFEG/ElementBase.h"

// 二维 2 节点边界面力载荷单元（对应旧 newmark 项目的 a2ll2）
// 纯载荷单元：刚度/质量/阻尼恒为 0，仅 eload 沿线积分把面力 fu/fv 分配到 2 节点。
// 依附于同 id 的 DelQ4g 体单元的一条边（边 id = 所属体单元 id，由 FEMData::addEle
// 的「节点更少即边」机制识别）。材料参数：fu（x 向面力）、fv（y 向面力）。
class StressBL2g : public CDFEG::ElementBase {
public:
    StressBL2g(CDFEG::PhyFieldData* pData);
    ~StressBL2g();

    // 装配面力载荷 eload（estif/emass/edamp 恒 0）
    virtual CDFEG::EleSubResult& run(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    // 边单元无应力恢复，返回空结果
    virtual CDFEG::uResult uEle(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;
};

#endif
