#include "ElT3.h"
#include "Elastic2DData.h"
#include "Elastic2DDispFieldData.h"
ElT3::ElT3(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(3, pData) {
    _name="ElT3";
    _dispNames = { "u", "v" };
    _paramNames ={ "E", "nu", "fx", "fy" };
    _types.insert("ElT3");
    // TODO: 设置 VTK 单元类型
    // _vtkCellType = VTKCellType::VTK_<TYPE>;


}
ElT3::~ElT3() {

}
CDFEG::EleSubResult& ElT3::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {



    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult ElT3::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;



    return res;
}

std::vector<double> ElT3::shapeFun(
    const std::vector<double>& refc
) {
    std::vector<double> shapes;
    double fval;
    double x1 = refc[0];
    double x2 = refc[1];
    fval = 1. - x[1] - x[2];
    shapes.push_back(fval);
    fval = x[1];
    shapes.push_back(fval);
    fval = x[2];
    shapes.push_back(fval);
    return shapes;
}

