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

#include "Truss2D.h"
#include "Truss2DData.h"
#include "Truss2DDispFieldData.h"
#include "CDFEG\MatrixFun.h"
Truss2D::Truss2D(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(2, pData) {
    _name="Truss2D";
    _dispNames = { "u", "v" };
    _paramNames ={ "E", "A" };
    _types.insert("Truss2D");
    // TODO: 设置 VTK 单元类型
    // _vtkCellType = VTKCellType::VTK_<TYPE>;

}
Truss2D::~Truss2D() {

}
CDFEG::EleSubResult& Truss2D::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    double E = matParams.at("E");
    double A = matParams.at("A");
    _result.eload.resize(4, 0);
    std::array<double,3> dirVal= CDFEG::calcDir2D2(r);
    double c = dirVal[0];
    double s = dirVal[1];
    double L = dirVal[2];
    double cc = A * E / L*c*c;
    double cs= A * E / L * c * s;
    double ss = A * E / L * s * s;
    _result.estif = {cc,cs,-cc,-cs,
                     cs,ss,-cs,-ss,
                     -cc,-cs,cc,cs,
                     -cs,-ss,cs,ss};
    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult Truss2D::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;
    double E = matParams.at("E");
    double A = matParams.at("A");
    std::array<double, 3> dirVal = CDFEG::calcDir2D2(r);
    double c = dirVal[0];
    double s = dirVal[1];
    double L = dirVal[2];

    double u1 = coef.at("u").at(0);
    double u2 = coef.at("u").at(1);
    double v1 = coef.at("v").at(0);
    double v2 = coef.at("v").at(1);

    double axialDisp = c * (u2 - u1) + s * (v2 - v1);
    res.eleResult["T"] = A * E / L * axialDisp;
    res.eleResult["sigma"] = E / L * axialDisp;

    return res;
}
