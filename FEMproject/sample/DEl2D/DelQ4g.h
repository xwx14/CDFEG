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

#ifndef DEL_Q4G_H
#define DEL_Q4G_H
#include "CDFEG/IsoEleBase.h"

// 二维 Q4 等参弹性动力单元
// 相比静力单元 ElQ4g，额外输出集中质量 emass 与集中阻尼 edamp，供 Newmark 有效矩阵装配
// 对应旧项目 a1eq4g2.c：刚度 K + 一致集中质量(rou*vol) + Rayleigh 集中阻尼(rou*alpha*vol)
class DelQ4g : public CDFEG::IsoEleBase {
public:
    DelQ4g(CDFEG::PhyFieldData* pData);
    ~DelQ4g();

    virtual CDFEG::EleSubResult& run(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    virtual CDFEG::uResult uEle(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    virtual std::vector<double> shapeFun(const std::vector<double>& refc) override;
};

#endif
