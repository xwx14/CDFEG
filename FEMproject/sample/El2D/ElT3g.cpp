#include "ElT3g.h"
#include "elData.h"
#include "ElDispFieldData.h"
ElT3g::ElT3g(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(3, pData) {
    _name="ElT3g";
    _dispNames = { "u", "v" };
    _paramNames ={ "pe", "pv", "fu", "fv", "rou", "alpha" };
    _types.insert("ElT3g");
    // TODO: 设置 VTK 单元类型
    // _vtkCellType = VTKCellType::VTK_<TYPE>;


}
ElT3g::~ElT3g() {

}
CDFEG::EleSubResult& ElT3g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {



    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult ElT3g::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;



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

