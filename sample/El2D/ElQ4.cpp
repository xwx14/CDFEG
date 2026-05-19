#include "ElQ4.h"
#include "Elastic2DData.h"
#include "Elastic2DDispFieldData.h"
ElQ4::ElQ4(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(4, pData) {
    _name="ElQ4";
    _dispNames = { "u", "v" };
    _paramNames ={ "E", "nu", "fx", "fy" };
    _types.insert("ElQ4");
    // TODO: 设置 VTK 单元类型
    // _vtkCellType = VTKCellType::VTK_<TYPE>;


}
ElQ4::~ElQ4() {

}
CDFEG::EleSubResult& ElQ4::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {



    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult ElQ4::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;



    return res;
}

std::vector<double> ElQ4::shapeFun(
    const std::vector<double>& refc
) {
    std::vector<double> shapes;
    double fval;
    double x1 = refc[0];
    double x2 = refc[1];
    fval = (1. - x[1]) / 2. * (1. - x[2]) / 2.;
    shapes.push_back(fval);
    fval = (1. + x[1]) / 2. * (1. - x[2]) / 2.;
    shapes.push_back(fval);
    fval = (1. + x[1]) / 2. * (1. + x[2]) / 2.;
    shapes.push_back(fval);
    fval = (1. - x[1]) / 2. * (1. + x[2]) / 2.;
    shapes.push_back(fval);
    return shapes;
}

