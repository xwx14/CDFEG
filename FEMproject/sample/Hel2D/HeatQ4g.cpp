#include "HeatQ4g.h"
#include "hel2dData.h"
#include "HeatFieldData.h"
#include "CDFEG/MatrixFun.h"
HeatQ4g::HeatQ4g(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(4, pData) {
    // 同域热固耦合：GiD 中仅定义一种单元 HelQ4g，热场与位移场共用。
    // 单元名取 GiD 类型名 "HelQ4g"，使 readMate 能匹配统一材料段 mat_HelQ4g。
    _name="HelQ4g";
    _dispNames = { "T" };
    // 材料参数取全量（热场 ek/ec/q + 弹性场 pe/pv/fu/fv/rou/alpha/alfa），
    // 供 readMate 为统一材料段命名全部 10 个值；run 仅按名取所需。
    _paramNames ={ "ek", "ec", "q", "pe", "pv", "fu", "fv", "rou", "alpha", "alfa" };
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
// 热传导单元：参照 macs/hel aeq4g2.c
// K = ek·∫(∂N/∂x·∂N/∂x + ∂N/∂y·∂N/∂y) dΩ
// eload = ∫ N·q dΩ （热源密度）
CDFEG::EleSubResult& HeatQ4g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    std::vector<double> refcoor(2);
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);

    double ek = matParams.at("ek");
    double q = matParams.at("q");
    double vol = 1.0;

    for (int iGaus = 0; iGaus < _nGaus; ++iGaus)
    {
        for (int i = 0; i < _nRefc; ++i)
            refcoor[i] = _refc[_dim * iGaus + i];
        std::vector<std::vector<double>> rctr;
        std::vector<double> coor;
        dcoor(r, iGaus, coor, rctr);
        std::vector<std::vector<double>> crtr;
        double det = CDFEG::inverse(rctr, crtr);
        std::vector<std::vector<double>> cu;
        shapn(iGaus, coor, crtr, cu);
        double weight = _gaus[iGaus] * det;

        // 单刚：K_ij = ek·(∂Ni/∂x·∂Nj/∂x + ∂Ni/∂y·∂Nj/∂y)
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                double stif = ek * (cu[i][1] * cu[j][1] + cu[i][2] * cu[j][2]) * vol;
                _result.estif[i * 4 + j] += stif * weight;
            }
        }
        // 热源载荷：eload_i = N_i·q
        for (int i = 0; i < 4; ++i)
        {
            _result.eload[i] += cu[i][0] * q * vol * weight;
        }
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
// 热场无后处理结果（温度由基类 uPhy 回填）
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
