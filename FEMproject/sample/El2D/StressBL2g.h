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

// 二维 2 节点边界面力载荷单元（闭式载荷分配 + smit 坐标变换，对应基准 a2ll2/a2gl2）。
// 纯载荷单元：刚度/质量/阻尼恒为 0，仅 eload 把面力分配到 2 节点。
// 材料参数：fu（沿线切向面力密度）、fv（沿线法向面力密度）。
//
// 线性形函数下均匀面力的等效节点载荷有闭式解 ∫N_i dΓ = L/2，
// 故每节点各得总力一半，无需等参元数值积分（与 ElT3 闭式范式一致）。
//
// 计算分两步：
//   1) computeLocalMatrix：局部载荷向量 [节点1切向,节点1法向,节点2切向,节点2法向]，
//      每分量 = (fu 或 fv) * L/2；
//   2) coordTransform：smit 构造局部切向/法向轴→t 矩阵，tl 把局部 eload
//      变换到全局写入 _result（a2gl2）。
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
    // 局部载荷向量：[节点1切向,节点1法向,节点2切向,节点2法向]
    void computeLocalMatrix(const std::vector<double>& r,
                            const std::map<std::string, double>& matParams,
                            std::vector<double>& eload);
    // 坐标变换：局部→全局
    void coordTransform(const std::vector<double>& r,
                        const std::vector<double>& locEload);
};

#endif
