#include "beq4g2.h"
#include "elData.h"
#include "elbFieldData.h"
beq4g2::beq4g2(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(4, pData) {
    _name="beq4g2";
    _dispNames = { "dxx", "dyy", "dxy" };
    _paramNames ={ "pe", "pv" };
    _types.insert("beq4g2");

    _dim = 2;
    _nGaus = 4;
    _nDisp = 3;
    _nRefc = 2;
    _nCoor = 2;
    _nVar = 12;
    _nNode = 4;
    _gaus.resize(4);
    _refc.resize(8);
    _gaus[0] = 1.0;
    _refc[0] = 0.5773502692;
    _refc[1] = 0.5773502692;
    _gaus[1] = 1.0;
    _refc[2] = 0.5773502692;
    _refc[3] = -0.5773502692;
    _gaus[2] = 1.0;
    _refc[4] = -0.5773502692;
    _refc[5] = 0.5773502692;
    _gaus[3] = 1.0;
    _refc[6] = -0.5773502692;
    _refc[7] = -0.5773502692;
    caculateShapeCoef(2);
    _result.emass.resize(_nVar);
    _result.eload.resize(_nVar);
    _result.estif.resize(_nVar * _nVar);
    _result.edamp.resize(_nVar * _nVar);
    _vtkCellType =VTKCellType::VTK_QUAD;
}
beq4g2::~beq4g2() {

}
CDFEG::EleSubResult& beq4g2::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {



    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult beq4g2::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult  res;



    return res;
}

std::vector<double> beq4g2::shapeFun(
    const std::vector<double>& refc
) {
    std::vector<double> shapes;
    double fval;
    double x = refc[0];
    double y = refc[1];
    fval = (1.-x)/2*(1.-y)/2;
    shapes.push_back(fval);
    fval = (1.+x)/2*(1.-y)/2;
    shapes.push_back(fval);
    fval = (1.+x)/2*(1.+y)/2;
    shapes.push_back(fval);
    fval = (1.-x)/2*(1.+y)/2;
    shapes.push_back(fval);
    return shapes;
}

