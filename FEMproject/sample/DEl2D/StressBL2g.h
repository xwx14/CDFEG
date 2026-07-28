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
#include "CDFEG/IsoEleBase.h"

// 二维 2 节点边界面力载荷单元（一维等参元 + smit 坐标变换，对应基准 a2ll2/a2gl2）。
// 纯载荷单元：刚度/质量/阻尼恒为 0，仅 eload 沿线积分把面力分配到 2 节点。
// 依附于同 id 的 DelQ4g 体单元的一条边（边 id = 所属体单元 id，由 DomainData::addEle
// 的「节点更少即边」机制识别）。材料参数：fu（沿线切向面力密度）、fv（沿线法向面力密度）。
//
// 作为一维等参元（_dim=_nRefc=1）：形函数与高斯积分
// 由 IsoEleBase 的 caculateShapeCoef/shapeFun/_refShapCoef 提供。
//
// 计算分两步：
//   1) computeLocalMatrix：一维等参荷载向量，estif/emass/edamp=0，
//      eload=∫N·(fu,fv)dΓ（局部 [节点1切向,节点1法向,节点2切向,节点2法向]）；
//   2) coordTransform：smit 构造局部切向/法向轴→t 矩阵，tl 把局部 eload
//      变换到全局写入 _result。
class StressBL2g : public CDFEG::IsoEleBase {
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

    // 一维线性形函数（沿线参考坐标 rx）
    virtual std::vector<double> shapeFun(
        const std::vector<double>& refc
    ) override;

private:
    // 一维等参荷载向量：局部 [节点1切向,节点1法向,节点2切向,节点2法向]
    void computeLocalMatrix(const std::vector<double>& r,
                            const std::map<std::string, double>& matParams,
                            std::vector<double>& eload);
    // 坐标变换：局部→全局
    void coordTransform(const std::vector<double>& r,
                        const std::vector<double>& locEload);
};

#endif
