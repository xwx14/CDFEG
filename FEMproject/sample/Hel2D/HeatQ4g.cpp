#include "HeatQ4g.h"
#include "hel2dData.h"
#include "HeatFieldData.h"
HeatQ4g::HeatQ4g(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(4, pData) {
    _name="HeatQ4g";
    _dispNames = { "T" };
    _paramNames ={ "ek", "ec", "q" };
    _types.insert("HeatQ4g");
    _types.insert("HelQ4g");

    _dim = 2;
    _nGaus = 4;
    _nDisp = 1;
    _nRefc = 2;
    _nCoor = 2;
    _nVar = 4;
    _nNode = 4;
    _gaus.resize(4);
    _refc.resize(8);
    _gaus[0] = 1.0;
    _refc[0] = 0.5773502691896258;
    _refc[1] = 0.5773502691896258;
    _gaus[1] = 1.0;
    _refc[2] = 0.5773502691896258;
    _refc[3] = -0.5773502691896258;
    _gaus[2] = 1.0;
    _refc[4] = -0.5773502691896258;
    _refc[5] = 0.5773502691896258;
    _gaus[3] = 1.0;
    _refc[6] = -0.5773502691896258;
    _refc[7] = -0.5773502691896258;
    caculateShapeCoef(2);
    _result.emass.resize(_nVar);
    _result.eload.resize(_nVar);
    _result.estif.resize(_nVar * _nVar);
    _result.edamp.resize(_nVar * _nVar);
    _vtkCellType =VTKCellType::VTK_QUAD;
}
HeatQ4g::~HeatQ4g() {

}
CDFEG::EleSubResult& HeatQ4g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {



    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult HeatQ4g::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;



    return res;
}

std::vector<double> HeatQ4g::shapeFun(
    const std::vector<double>& refc
) {
    std::vector<double> shapes;
    double fval;
    double x = refc[0];
    double y = refc[1];
    fval = (1. - x) / 2. * (1. - y) / 2.;
    shapes.push_back(fval);
    fval = (1. + x) / 2. * (1. - y) / 2.;
    shapes.push_back(fval);
    fval = (1. + x) / 2. * (1. + y) / 2.;
    shapes.push_back(fval);
    fval = (1. - x) / 2. * (1. + y) / 2.;
    shapes.push_back(fval);
    return shapes;
}

