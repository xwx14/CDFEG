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

#include "DelQ4g.h"
#include "DelDispFieldData.h"
#include "del2dData.h"
#include "CDFEG/MatrixFun.h"

DelQ4g::DelQ4g(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(4, pData) {
    _name = "DelQ4g";
    _dispNames = { "u", "v" };
    // 旧项目 a1eq4g2 材料：pe/pv/fu/fv/rou/alpha 六参数
    _paramNames = { "pe", "pv", "fu", "fv", "rou", "alpha" };
    _types.insert("DelQ4g");

    _dim = 2;
    _nGaus = 4;
    _nDisp = 2;
    _nRefc = 2;
    _nCoor = 2;
    _nVar = 8;
    _nNode = 4;
    _gaus.resize(4);
    _refc.resize(8);
    _gaus[0] = 1.0;
    _refc[0] = 0.5773502692;
    _refc[1] = 0.5773502692;
    _gaus[1] = 1.0;
    _refc[2] = 0.5773502692;
    _refc[3] = -0.5773502692;
    _gaus[2] = 1.0;
    _refc[4] = -0.5773502692;
    _refc[5] = 0.5773502692;
    _gaus[3] = 1.0;
    _refc[6] = -0.5773502692;
    _refc[7] = -0.5773502692;
    caculateShapeCoef(2);
    _result.emass.resize(_nVar);
    _result.eload.resize(_nVar);
    _result.estif.resize(_nVar * _nVar);
    _result.edamp.resize(_nVar);
    _vtkCellType = VTKCellType::VTK_QUAD;
}

DelQ4g::~DelQ4g() {

}

std::vector<double> DelQ4g::shapeFun(const std::vector<double>& refc) {
    std::vector<double> rt;
    double rx = refc[0];
    double ry = refc[1];
    double fval = (1. - rx) / 2. * (1. - ry) / 2.;
    rt.push_back(fval);
    fval = (1. + rx) / 2. * (1. - ry) / 2.;
    rt.push_back(fval);
    fval = (1. + rx) / 2. * (1. + ry) / 2.;
    rt.push_back(fval);
    fval = (1. - rx) / 2. * (1. + ry) / 2.;
    rt.push_back(fval);
    return rt;
}

CDFEG::EleSubResult& DelQ4g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    std::vector<double> refcoor(2);
    // 清零本单元结果
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
    std::fill(_result.edamp.begin(), _result.edamp.end(), 0.0);

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double fu = matParams.at("fu");
    double fv = matParams.at("fv");
    double rou = matParams.at("rou");
    double alpha = matParams.at("alpha");
    double vol = 1.0;
    // 平面应变本构因子（与 ElQ4g / 旧 a1eq4g2 一致）
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = 0.5 - pv;

    for (int iGaus = 0; iGaus < _nGaus; ++iGaus)
    {
        for (int i = 0; i < _nRefc; ++i)
        {
            refcoor[i] = _refc[_dim * iGaus + i];
        }
        std::vector<std::vector<double>> rctr;
        std::vector<double> coor;
        dcoor(r, iGaus, coor, rctr);
        std::vector<std::vector<double>> crtr;
        double det = CDFEG::inverse(rctr, crtr);
        std::vector<std::vector<double>> cu;
        shapn(iGaus, coor, crtr, cu);
        std::vector<std::vector<double>> cv = cu;
        double weight = _gaus[iGaus] * det;

        std::vector<double> eexx(8, 0.0);
        std::vector<double> eeyy(8, 0.0);
        std::vector<double> eexy(8, 0.0);
        int i1, i2;

        // B 矩阵分量（应变-位移）
        for (int i = 0; i < 4; ++i)
        {
            i1 = i * 2;
            i2 = i * 2 + 1;
            eexx[i1] = +cu[i][1];
            eeyy[i2] = +cv[i][2];
            eexy[i1] += cu[i][2];
            eexy[i2] += cv[i][1];
        }

        // 体力载荷向量 eload = ∫ N·f dΩ
        double stif;
        for (int i = 0; i < 4; ++i)
        {
            stif = cu[i][0] * fu * vol;
            _result.eload[2 * i] += stif * weight;
            stif = cv[i][0] * fv * vol;
            _result.eload[2 * i + 1] += stif * weight;
        }

        // 刚度矩阵 K = ∫ BᵀD B dΩ（含对角集中质量/阻尼的体积积分权重）
        int ii = -1;
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                stif = +eexx[i] * eexx[j] * (1. - pv) * fact
                    + eexx[i] * eeyy[j] * pv * fact
                    + eeyy[i] * eexx[j] * pv * fact
                    + eeyy[i] * eeyy[j] * (1. - pv) * fact
                    + eexy[i] * eexy[j] * shear * fact;
                _result.estif[++ii] += stif * weight;
            }
        }

        // 集中质量 emass[i] += ∫ rou·vol·Nᵢ dΩ（旧 a1eq4g2: elump·N）
        // 集中阻尼 edamp[i] += ∫ rou·alpha·vol·Nᵢ dΩ（Rayleigh 形式，仅乘 alpha）
        for (int i = 0; i < 4; ++i)
        {
            double ni = cu[i][0];
            double massLump = rou * vol * weight * ni;
            _result.emass[2 * i] += massLump;
            _result.emass[2 * i + 1] += massLump;
            double dampLump = rou * alpha * vol * weight * ni;
            _result.edamp[2 * i] += dampLump;
            _result.edamp[2 * i + 1] += dampLump;
        }
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

CDFEG::uResult DelQ4g::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    // 应力恢复：由节点位移求积分点应力，再外推到节点（与 ElQ4g 相同）
    CDFEG::uResult res;

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double vol = 1.0;
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = (0.5 - pv);

    const std::vector<double>& u = coef.at("u");
    const std::vector<double>& vDisp = coef.at("v");

    std::vector<double> nodeSigmaXX(4, 0.0), nodeSigmaYY(4, 0.0);
    std::vector<double> nodeSigmaXY(4, 0.0);
    std::vector<double> nodeWeight(4, 0.0);

    double sigmaXX = 0.0, sigmaYY = 0.0, sigmaXY = 0.0;
    double totalWeight = 0.0;

    for (int iGaus = 0; iGaus < _nGaus; ++iGaus)
    {
        std::vector<std::vector<double>> rctr;
        std::vector<double> coor;
        dcoor(r, iGaus, coor, rctr);
        std::vector<std::vector<double>> crtr;
        double det = CDFEG::inverse(rctr, crtr);
        std::vector<std::vector<double>> cu;
        shapn(iGaus, coor, crtr, cu);
        std::vector<std::vector<double>> cv = cu;
        double weight = _gaus[iGaus] * det;
        totalWeight += weight;

        double exx = 0.0, eyy = 0.0, exy = 0.0;
        for (int i = 0; i < 4; ++i)
        {
            exx += cu[i][1] * u[i];
            eyy += cu[i][2] * vDisp[i];
            exy += cu[i][2] * u[i] + cu[i][1] * vDisp[i];
        }

        double gSigmaXX = fact * ((1 - pv) * exx + pv * eyy);
        double gSigmaYY = fact * (pv * exx + (1 - pv) * eyy);
        double gSigmaXY = fact * shear * exy;

        sigmaXX += gSigmaXX * weight;
        sigmaYY += gSigmaYY * weight;
        sigmaXY += gSigmaXY * weight;

        for (int i = 0; i < 4; ++i)
        {
            double N = cu[i][0];
            double nodeW = N * weight;
            nodeSigmaXX[i] += gSigmaXX * nodeW;
            nodeSigmaYY[i] += gSigmaYY * nodeW;
            nodeSigmaXY[i] += gSigmaXY * nodeW;
            nodeWeight[i] += nodeW;
        }
    }

    if (totalWeight > 0.0)
    {
        sigmaXX /= totalWeight;
        sigmaYY /= totalWeight;
        sigmaXY /= totalWeight;
    }

    res.eleResult["sigmaXX"] = sigmaXX;
    res.eleResult["sigmaYY"] = sigmaYY;
    res.eleResult["sigmaXY"] = sigmaXY;
    res.eleResult["volume"] = totalWeight;

    res.nodeResult["sigmaXX"] = nodeSigmaXX;
    res.nodeResult["sigmaYY"] = nodeSigmaYY;
    res.nodeResult["sigmaXY"] = nodeSigmaXY;
    res.nodeResult["weight"] = nodeWeight;

    return res;
}
