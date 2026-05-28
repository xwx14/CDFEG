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

#include "ElT3g.h"
#include "elData.h"
#include "ElDispFieldData.h"
#include "CDFEG/MatrixFun.h"
ElT3g::ElT3g(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(3, pData) {
    _name="ElT3g";
    _dispNames = { "u", "v" };
    _paramNames ={ "pe", "pv", "fu", "fv", "rou", "alpha" };
    _types.insert("ElT3g");
	_dim = 2;
	_nGaus = 3;
	_nDisp = 2;
	_nRefc = 2;
	_nCoor = 2;
	_nVar = 6;
	_nNode = 3;
	_gaus.resize(3);
	_refc.resize(6);
	_gaus[0] = 0.16666666666666666;
	_refc[0] = 0.6666666666666666;
	_refc[1] = 0.16666666666666666;
	_gaus[1] = 0.16666666666666666;
	_refc[2] = 0.16666666666666666;
	_refc[3] = 0.6666666666666666;
	_gaus[2] = 0.16666666666666666;
	_refc[4] = 0.16666666666666666;
	_refc[5] = 0.16666666666666666;
	caculateShapeCoef(2);
	_result.emass.resize(_nVar);
	_result.eload.resize(_nVar);
	_result.estif.resize(_nVar * _nVar);
	_result.edamp.resize(_nVar * _nVar);
	_vtkCellType = VTKCellType::VTK_TRIANGLE;

}
ElT3g::~ElT3g() {

}
CDFEG::EleSubResult& ElT3g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    std::vector<double> refcoor(2);
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double fu = matParams.at("fu");
    double fv = matParams.at("fv");
    double vol = 1.0;
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
        std::vector<double> eexx(6, 0.0);
        std::vector<double> eeyy(6, 0.0);
        std::vector<double> eexy(6, 0.0);
        int i1, i2;

        for (int i = 0; i < 3; ++i)
        {
            i1 = i * 2;
            i2 = i * 2 + 1;
            eexx[i1] = +cu[i][1];
            eeyy[i2] = +cv[i][2];
            eexy[i1] += cu[i][2];
            eexy[i2] += cv[i][1];
        }
        double stif;
        int ii = -1;
        for (int i = 0; i < 3; ++i)
        {
            stif = cu[i][0] * fu * vol;
            _result.eload[2 * i] += stif * weight;
            stif = cv[i][0] * fv * vol;
            _result.eload[2 * i + 1] += stif * weight;
        }
        for (int i = 0; i < 6; ++i)
        {
            for (int j = 0; j < 6; ++j)
            {
                stif = +eexx[i] * eexx[j] * (1. - pv) * fact
                    + eexx[i] * eeyy[j] * pv * fact
                    + eeyy[i] * eexx[j] * pv * fact
                    + eeyy[i] * eeyy[j] * (1. - pv) * fact
                    + eexy[i] * eexy[j] * shear * fact;
                _result.estif[++ii] += stif * weight;
            }
        }
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult ElT3g::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult res;

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double vol = 1.0;
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = (0.5 - pv);

    const std::vector<double>& u = coef.at("u");
    const std::vector<double>& vDisp = coef.at("v");

    std::vector<double> nodeSigmaXX(3, 0.0), nodeSigmaYY(3, 0.0);
    std::vector<double> nodeSigmaXY(3, 0.0);
    std::vector<double> nodeWeight(3, 0.0);

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

        for (int i = 0; i < 3; ++i)
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

        for (int i = 0; i < 3; ++i)
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

std::vector<double> ElT3g::shapeFun(
    const std::vector<double>& refc
) {
    std::vector<double> shapes;
    double fval;
    double x = refc[0];
    double y = refc[1];
    fval = 1. - x - y;
    shapes.push_back(fval);
    fval = x;
    shapes.push_back(fval);
    fval = y;
    shapes.push_back(fval);
    return shapes;
}

