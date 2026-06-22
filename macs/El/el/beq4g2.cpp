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
    // 清零 —— adda 累加，不清零会叠加错误值
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double vol = 1.0;
    // 平面应变本构系数（beq4g2.c:124-125）
    double fact = pe / (1.0 + pv) / (1.0 - 2.0 * pv);
    double shear = (1.0 - 2.0 * pv) / 2.0;

    // 从 ela 场取节点位移（Task 10 eProgram 调 getCoef 传入）
    const std::vector<double>& u = coef.at("ela::u");
    const std::vector<double>& v = coef.at("ela::v");

    for (int iGaus = 0; iGaus < _nGaus; ++iGaus)
    {
        // 高斯积分：雅可比 → 逆矩阵 → 形函数偏导
        std::vector<std::vector<double>> rctr;
        std::vector<double> coor;
        dcoor(r, iGaus, coor, rctr);
        std::vector<std::vector<double>> crtr;
        double det = CDFEG::inverse(rctr, crtr);
        std::vector<std::vector<double>> cu;
        shapn(iGaus, coor, crtr, cu);
        double weight = _gaus[iGaus] * det;

        // 应变：由 ela 位移 u,v 经形函数偏导求物理坐标导数
        // beq4g2.c:126-132 / ebeq4g2 → coefc → exx,eyy,exy
        double exx = 0.0, eyy = 0.0, exy = 0.0;
        for (int i = 0; i < 4; ++i)
        {
            // cu[i][1] = dN_i/dx, cu[i][2] = dN_i/dy
            exx += cu[i][1] * u[i];       // du/dx
            eyy += cu[i][2] * v[i];       // dv/dy
            exy += cu[i][2] * u[i]        // du/dy
                 + cu[i][1] * v[i];       // dv/dx
        }

        // 应力：平面应变本构（beq4g2.c:133-135）
        double fxx = fact * ((1.0 - pv) * exx + pv * eyy);
        double fyy = fact * (pv * exx + (1.0 - pv) * eyy);
        double fxy = fact * shear * exy;

        // emass = lumped mass（beq4g2.c:146-181 / beq4g2.ges mass lump）
        // 旧程序 estif 全 0（stiff*0.0），mass 单独存 emass；最小二乘法的"质量"分母
        // emass[iv] += vol*weigh * N_i（带形函数值，与 eload 口径一致）
        for (int i = 0; i < 4; ++i)
        {
            double cd = cu[i][0];  // 形函数值 N_i
            _result.emass[i * 3 + 0] += cd * vol * weight;  // dxx
            _result.emass[i * 3 + 1] += cd * vol * weight;  // dyy
            _result.emass[i * 3 + 2] += cd * vol * weight;  // dxy
        }

        // eload = 应力载荷（beq4g2.c:183-197 / beq4g2.ges load）
        // cdxx/cdyy/cdxy 即形函数值 cu[i][0]（beq4g2.c 中 cdxx 对应第一个未知量的形函数）
        // kvord: 节点i的 dxx/dyy/dxy 方程号 = i*3+0, i*3+1, i*3+2
        for (int i = 0; i < 4; ++i)
        {
            double cd = cu[i][0];  // 形函数值
            int ivDxx = i * 3;       // dxx 方程号
            int ivDyy = i * 3 + 1;   // dyy 方程号
            int ivDxy = i * 3 + 2;   // dxy 方程号
            _result.eload[ivDxx] += cd * fxx * vol * weight;
            _result.eload[ivDyy] += cd * fyy * vol * weight;
            _result.eload[ivDxy] += cd * fxy * vol * weight;
        }
    }

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

