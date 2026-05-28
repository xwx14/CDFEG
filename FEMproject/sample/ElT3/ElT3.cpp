#include "ElT3.h"
#include "ElasticT3Data.h"
#include "Elastic2DDispFieldData.h"
ElT3::ElT3(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(3, pData) {
    _name="ElT3";
    _dispNames = { "u", "v" };
    _paramNames ={ "E", "nu", "t", "fx", "fy" };
    _types.insert("ElT3");

    _vtkCellType =VTKCellType::VTK_TRIANGLE;
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
