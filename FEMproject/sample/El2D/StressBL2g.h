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

#ifndef STRESS_BL2G_H
#define STRESS_BL2G_H
#include "CDFEG/ElementBase.h"

// 二维 2 节点边界面力载荷单元
// 纯载荷单元：刚度/质量/阻尼恒为 0，仅 eload 沿线积分把面力分配到 2 节点。
// 材料参数：fu（沿线切向面力密度）、fv（沿线法向面力密度）。
//
// 计算分两步：
//   1) computeLocalMatrix：局部坐标（切向/法向）下算单元矩阵，
//      estif/emass/edamp=0，eload=∫N·(fu,fv)dΓ；
//   2) coordTransform：smit 构造局部坐标轴→t 矩阵，tkt/tmt/tl 把局部矩阵
//      变换到全局写入 _result。
class StressBL2g : public CDFEG::ElementBase {
public:
    StressBL2g(CDFEG::PhyFieldData* pData);
    ~StressBL2g();

    // run = computeLocalMatrix + coordTransform
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

private:
    // 局部坐标下的单元矩阵计算（对应 a2ll2）
    void computeLocalMatrix(const std::vector<double>& r,
                            const std::map<std::string, double>& matParams,
                            std::vector<double>& estif,
                            std::vector<double>& emass,
                            std::vector<double>& edamp,
                            std::vector<double>& eload);
    // 坐标转换：局部→全局（对应 a2gl2 的 smit + tkt/tmt/tl）
    void coordTransform(const std::vector<double>& r,
                        const std::vector<double>& locEstif,
                        const std::vector<double>& locEmass,
                        const std::vector<double>& locEdamp,
                        const std::vector<double>& locEload);
};

#endif
