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

#include "Truss1D.h"
#include "Truss1DData.h"
#include "Truss1DDispFieldData.h"
Truss1D::Truss1D(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(2, pData) {
    _name="Truss1D";
    _dispNames = { "u"};
    _paramNames ={ "E","A"};
    _types.insert("Truss1D");
    // TODO: 设置 VTK 单元类型
    // _vtkCellType = VTKCellType::VTK_<TYPE>;

}
Truss1D::~Truss1D() {

}
CDFEG::EleSubResult& Truss1D::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {


    _result.nodeIds.clear();
    _result.eload.resize(2, 0);

    double E = matParams.at("E");
    double A = matParams.at("A");
    double L = abs(r[0] - r[1]);
    double X = E * A / L;
    _result.estif = { X,-X,-X,X };
    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult Truss1D::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;
    double E = matParams.at("E");
    double A = matParams.at("A");
    double L = abs(r[0] - r[1]);
    double axialDisp = coef.at("u").at(1) - coef.at("u").at(0);
    res.eleResult["T"] = A * E * axialDisp / L;
    res.eleResult["sigma"] = E * axialDisp / L;

    return res;
}