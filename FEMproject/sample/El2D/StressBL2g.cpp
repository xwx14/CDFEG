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

#include "StressBL2g.h"
#include <cmath>
#include <algorithm>

StressBL2g::StressBL2g(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(2, pData) {
    _name = "StressBL2g";
    _dispNames = { "u", "v" };
    // 旧 a2ll2 材料：fu/fv 两参数（fu=沿线切向面力, fv=沿线法向面力）
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
    _result.edamp.resize(_nVar * _nVar, 0.0);
    _result.eload.resize(_nVar, 0.0);
}

StressBL2g::~StressBL2g() {
}

void StressBL2g::computeLocalMatrix(
    const std::vector<double>& r,
    const std::map<std::string, double>& matParams,
    std::vector<double>& estif,
    std::vector<double>& emass,
    std::vector<double>& edamp,
    std::vector<double>& eload
) {
    // 对应基准 a2ll2：在局部坐标（切向/法向）下计算单元矩阵。
    // stif/mass/damp 恒 0（a2ll2.ges 中 *0.0），仅 load=+[u]*fu+[v]*fv。
    const int nvar = 4;
    estif.assign(nvar * nvar, 0.0);
    emass.assign(nvar, 0.0);
    edamp.assign(nvar * nvar, 0.0);
    eload.assign(nvar, 0.0);

    double fu = matParams.at("fu");   // 切向面力密度
    double fv = matParams.at("fv");   // 法向面力密度

    // 线长 → 参考坐标到弧长的雅可比 det = L/2
    double len = std::hypot(r[2] - r[0], r[3] - r[1]);
    double det = len / 2.0;

    // 2 高斯点 rx=±1，权重=1；形函数 N1=(1-rx)/2，N2=(1+rx)/2。
    // 局部 eload 顺序：[节点1切向, 节点1法向, 节点2切向, 节点2法向]
    const double gausRx[2] = { -1.0, 1.0 };
    for (int g = 0; g < 2; ++g)
    {
        double N1 = (1.0 - gausRx[g]) / 2.0;
        double N2 = (1.0 + gausRx[g]) / 2.0;
        double w = det;   // gaus 权重 = 1
        eload[0] += N1 * fu * w;
        eload[1] += N1 * fv * w;
        eload[2] += N2 * fu * w;
        eload[3] += N2 * fv * w;
    }
}

void StressBL2g::coordTransform(
    const std::vector<double>& r,
    const std::vector<double>& locEstif,
    const std::vector<double>& locEmass,
    const std::vector<double>& locEdamp,
    const std::vector<double>& locEload
) {
    // 对应基准 a2gl2：smit 构造局部坐标轴 → t 矩阵 → tkt/tmt/tl 局部→全局。
    // smit（施密特正交）：切向 t̂=(dx,dy)/L（节点1→2），法向 n̂=(-dy,dx)/L（逆时针90°）
    double dx = r[2] - r[0], dy = r[3] - r[1];
    double len = std::hypot(dx, dy);
    double tx = dx / len, ty = dy / len;
    double nx = -ty, ny = tx;

    // t 矩阵 4×4（行=局部变量[节点1切向,节点1法向,节点2切向,节点2法向]，列=全局变量[u1,v1,u2,v2]）
    double t[4][4] = { {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0} };
    t[0][0] = tx; t[0][1] = ty;
    t[1][0] = nx; t[1][1] = ny;
    t[2][2] = tx; t[2][3] = ty;
    t[3][2] = nx; t[3][3] = ny;

    const int nvar = 4;
    // tl: egl = tᵀ·ell（载荷向量局部→全局）
    for (int i = 0; i < nvar; ++i)
    {
        double s = 0.0;
        for (int l = 0; l < nvar; ++l) s += t[l][i] * locEload[l];
        _result.eload[i] = s;
    }
    // tkt/tmt：estif/emass/edamp 局部恒 0，变换后仍 0，直接置 0
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
    std::fill(_result.edamp.begin(), _result.edamp.end(), 0.0);
}

CDFEG::EleSubResult& StressBL2g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    // Step 1: 局部坐标下的单元矩阵（a2ll2）
    std::vector<double> locEstif, locEmass, locEdamp, locEload;
    computeLocalMatrix(r, matParams, locEstif, locEmass, locEdamp, locEload);
    // Step 2: 坐标转换局部→全局（a2gl2: smit + tkt/tmt/tl），写入 _result
    coordTransform(r, locEstif, locEmass, locEdamp, locEload);

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
