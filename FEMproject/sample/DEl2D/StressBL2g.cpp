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

#include "StressBL2g.h"
#include <cmath>
#include <algorithm>

StressBL2g::StressBL2g(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(2, pData) {
    _name = "StressBL2g";
    _dispNames = { "u", "v" };
    // 旧 a2ll2 材料：fu/fv 两参数（x/y 向面力）
    _paramNames = { "fu", "fv" };
    _types.insert("StressBL2g");

    _dim = 2;
    _nNode = 2;
    _nDisp = 2;
    _nCoor = 2;
    _nVar = 4;   // 2 节点 × 2 自由度
    _vtkCellType = VTKCellType::VTK_LINE;
    // estif/emass/edamp 恒为 0（纯载荷单元），eload 每步重算
    _result.estif.resize(_nVar * _nVar, 0.0);
    _result.emass.resize(_nVar, 0.0);
    _result.edamp.resize(_nVar, 0.0);
    _result.eload.resize(_nVar, 0.0);
}

StressBL2g::~StressBL2g() {
}

CDFEG::EleSubResult& StressBL2g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    // 清零本边单元结果（estif/emass/edamp 恒 0，仅算 eload）
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
    std::fill(_result.edamp.begin(), _result.edamp.end(), 0.0);
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);

    // fu/fv 为局部坐标的面力密度：fu 沿线切向、fv 沿线法向（对应基准 a2ll2/a2gl2）
    double fu = matParams.at("fu");
    double fv = matParams.at("fv");

    // 2 节点坐标 r = [x1, y1, x2, y2]（与 DelQ4g 一致：外层节点、内层维度）
    double x1 = r[0], y1 = r[1];
    double x2 = r[2], y2 = r[3];
    double dx = x2 - x1, dy = y2 - y1;
    double len = std::hypot(dx, dy);
    double det = len / 2.0;   // 参考坐标 → 弧长的雅可比（线长一半）

    // 局部坐标轴（与基准 smit 施密特正交一致）：
    //   切向 t̂=(dx,dy)/L（节点1→节点2），法向 n̂=(-dy,dx)/L（逆时针90°）
    double tx = dx / len, ty = dy / len;
    double nx = -ty, ny = tx;

    // 2 高斯点 rx = ±1，权重 = 1；形函数 N1=(1-rx)/2，N2=(1+rx)/2。
    // 先按 a2ll2 算局部节点载荷（切向 N·fu、法向 N·fv），再按 a2gl2 的 tl(egl=tᵀ·ell)
    // 变换到全局：全局u = tx·(切向)+nx·(法向)；全局v = ty·(切向)+ny·(法向)。
    //   eload[0/1]=节点1 全局u/v，eload[2/3]=节点2 全局u/v
    const double gausPts[2] = { -1.0, 1.0 };
    const double gausW[2] = { 1.0, 1.0 };
    for (int g = 0; g < 2; ++g)
    {
        double rx = gausPts[g];
        double N1 = (1.0 - rx) / 2.0;
        double N2 = (1.0 + rx) / 2.0;
        double w = det * gausW[g];
        // 节点1 / 节点2 的局部（切向 lu / 法向 lv）载荷
        double lu1 = N1 * fu * w, lv1 = N1 * fv * w;
        double lu2 = N2 * fu * w, lv2 = N2 * fv * w;
        _result.eload[0] += tx * lu1 + nx * lv1;
        _result.eload[1] += ty * lu1 + ny * lv1;
        _result.eload[2] += tx * lu2 + nx * lv2;
        _result.eload[3] += ty * lu2 + ny * lv2;
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

CDFEG::uResult StressBL2g::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    // 边单元不参与应力恢复：返回空结果，避免写越界 _elemRes
    CDFEG::uResult res;
    return res;
}
