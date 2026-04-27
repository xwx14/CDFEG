#include "Truss1D.h"
#include "Truss1DData.h"
#include "Truss1DDispFieldData.h"
Truss1D::Truss1D(CDFEG::PhyFieldData* pData)
    : CDFEG::EleSubBase(2, pData) {
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
    res.eleResult["T"] = A * E * (coef.at("u").at(1) - coef.at("u").at(0)) / L;

    return res;
}