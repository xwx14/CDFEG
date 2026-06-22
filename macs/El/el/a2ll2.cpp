#include "a2ll2.h"
#include "elData.h"
#include "elaFieldData.h"
a2ll2::a2ll2(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(2, pData) {
    _name="a2ll2";
    _dispNames = { "u", "v" };
    _paramNames ={ "fu", "fv" };
    _types.insert("a2ll2");

    _dim = 2;
    _nGaus = 2;
    _nDisp = 2;
    _nRefc = 2;
    _nCoor = 2;
    _nVar = 4;
    _nNode = 2;
    _gaus.resize(2);
    _refc.resize(4);
    _gaus[0] = 1.0;
    _refc[0] = -1.0;
    _refc[1] = 0.0;
    _gaus[1] = 1.0;
    _refc[2] = 1.0;
    _refc[3] = 0.0;
    caculateShapeCoef(2);
    _result.emass.resize(_nVar);
    _result.eload.resize(_nVar);
    _result.estif.resize(_nVar * _nVar);
    _result.edamp.resize(_nVar * _nVar);
    _vtkCellType =VTKCellType::VTK_LINE;
}
a2ll2::~a2ll2() {

}
CDFEG::EleSubResult& a2ll2::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {



    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult a2ll2::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;



    return res;
}

std::vector<double> a2ll2::shapeFun(
    const std::vector<double>& refc
) {
    std::vector<double> shapes;
    double fval;
    double x = refc[0];
    double y = refc[1];
    fval = (1.-x)/2;
    shapes.push_back(fval);
    fval = (1.+x)/2;
    shapes.push_back(fval);
    return shapes;
}

