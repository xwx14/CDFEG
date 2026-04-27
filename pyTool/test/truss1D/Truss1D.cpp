#include "Truss1D.h"
#include "Truss1DData.h"
#include "Truss1DDispFieldData.h"
Truss1D::Truss1D(CDFEG::PhyFieldData* pData)
    : CDFEG::EleSubBase(2, pData) {
    _name="Truss1D";
    _dispNames = { "u" };
    _paramNames ={ "E", "A" };
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



    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult Truss1D::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;



    return res;
}
