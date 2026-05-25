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

#include "a2ll2.h"
#include "elData.h"
#include "aFieldData.h"
#include "CDFEG/MatrixFun.h"

a2ll2::a2ll2(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(2, pData) {
    _name = "a2ll2";
    _dispNames = { "u", "v" };
    _paramNames = { "fu", "fv" };
    _types.insert("a2ll2");
    _vtkCellType = ::VTK_LINE;

    _dim = 1;
    _nGaus = 2;
    _nDisp = 2;
    _nRefc = 1;
    _nCoor = 1;
    _nVar = 4;
    _nNode = 2;
    _gaus.resize(2);
    _refc.resize(2);
    _gaus[0] = 1.0;
    _refc[0] = -0.5773502691896257;
    _gaus[1] = 1.0;
    _refc[1] = 0.5773502691896257;
    caculateShapeCoef(1);
    _result.emass.resize(_nVar);
    _result.eload.resize(_nVar);
    _result.estif.resize(_nVar * _nVar);
    _result.edamp.resize(_nVar * _nVar);
}

a2ll2::~a2ll2() {

}

std::vector<double> a2ll2::shapeFun(const std::vector<double>& refc) {
    double xi = refc[0];
    std::vector<double> rt(2);
    rt[0] = 0.5 * (1.0 - xi);
    rt[1] = 0.5 * (1.0 + xi);
    return rt;
}

CDFEG::EleSubResult& a2ll2::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    _result.nodeIds.clear();
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
    std::fill(_result.edamp.begin(), _result.edamp.end(), 0.0);
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);

    double fu = matParams.at("fu");
    double fv = matParams.at("fv");

    std::vector<std::vector<double>> r_coord(_femData->_dim, std::vector<double>(_nNode));
    for (int i = 0; i < _femData->_dim; ++i) {
        for (int j = 0; j < _nNode; ++j) {
            r_coord[i][j] = r[i * _nNode + j];
        }
    }

    std::vector<std::vector<double>> z =
        CDFEG::computeTransformMatrix(_femData->_dim, _dim, _nNode, r_coord);

    std::vector<std::vector<double>> y(_dim, std::vector<double>(_nNode, 0.0));
    for (int j = 0; j < _nNode; ++j) {
        y[0][j] = r_coord[0][j] * z[0][0] + r_coord[1][j] * z[1][0];
    }

    std::vector<double> yy(_nNode);
    for (int j = 0; j < _nNode; ++j) {
        yy[j] = y[0][j];
    }

    std::vector<std::vector<double>> els(_nVar, std::vector<double>(_nVar, 0.0));
    std::vector<std::vector<double>> elm(_nVar, std::vector<double>(_nVar, 0.0));
    std::vector<std::vector<double>> eld(_nVar, std::vector<double>(_nVar, 0.0));
    std::vector<double> ell(_nVar, 0.0);

    run1D(yy, fu, fv, els, elm, eld, ell);

    std::vector<std::vector<double>> t(_nVar, std::vector<double>(_nVar, 0.0));

    t[0][0] = z[0][0];
    t[1][0] = z[1][0];
    t[2][2] = z[0][0];
    t[3][2] = z[1][0];

    t[0][1] = z[0][1];
    t[1][1] = z[1][1];
    t[2][3] = z[0][1];
    t[3][3] = z[1][1];

    auto estifTransformed = CDFEG::multiplyTAT(t, els);
    for (int i = 0; i < _nVar; ++i) {
        for (int j = 0; j < _nVar; ++j) {
            _result.estif[i * _nVar + j] = estifTransformed[i][j];
        }
    }

    auto egsMass = CDFEG::multiplyTAT(t, elm);
    for (int i = 0; i < _nVar; ++i) {
        for (int j = 0; j < _nVar; ++j) {
            _result.emass[i * _nVar + j] = egsMass[i][j];
        }
    }

    _result.eload = CDFEG::multiplyT(t, ell);

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

void a2ll2::run1D(
    const std::vector<double>& yy,
    double fu,
    double fv,
    std::vector<std::vector<double>>& els,
    std::vector<std::vector<double>>& elm,
    std::vector<std::vector<double>>& eld,
    std::vector<double>& ell
) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            els[i][j] = 0.0;
            elm[i][j] = 0.0;
            eld[i][j] = 0.0;
        }
        ell[i] = 0.0;
    }

    int kvord[] = {0, 2, 1, 3};

    for (int igaus = 0; igaus < _nGaus; ++igaus) {
        double refc = _refc[igaus];

        std::vector<double> refcVec = {refc};
        std::vector<double> shpr = shapeFun(refcVec);
        double cu[2] = {shpr[0], shpr[1]};
        double cv[2] = {shpr[0], shpr[1]};

        std::vector<double> fx = coordTransFun(yy, refcVec);
        std::vector<std::vector<double>> dfdx(1, std::vector<double>(2));
        dcoor(yy, igaus, fx, dfdx, 1);

        double det = dfdx[0][0];

        double weigh = det * _gaus[igaus];

        for (int i = 0; i < 2; ++i) {
            int iv = kvord[i];
            for (int j = 0; j < 2; ++j) {
                int jv = kvord[j];
                els[iv][jv] += cu[i] * cu[j] * 0.0 * weigh;
            }
        }

        for (int i = 0; i < 2; ++i) {
            int iv = kvord[i];
            ell[iv] += cu[i] * fu * weigh;
        }
        for (int i = 0; i < 2; ++i) {
            int iv = kvord[i + 2];
            ell[iv] += cv[i] * fv * weigh;
        }
    }
}

CDFEG::uResult a2ll2::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult res;
    return res;
}
