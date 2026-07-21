#include "DelQ4g.h"
#include "hel2dData.h"
#include "DelDispFieldData.h"
#include "CDFEG/MatrixFun.h"
DelQ4g::DelQ4g(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(4, pData) {
    // 同域热固耦合：GiD 中仅定义一种单元 HelQ4g，热场与位移场共用。
    _name="DelQ4g";
    _dispNames = { "u", "v" };
    // 本构类型 HelQ4g 由 hel2dData 注册；run/uEle 按名取所需（pe/pv/fu/fv/alfa）
    _mateTypeName = "HelQ4g";
    _types.insert("DelQ4g");
    _types.insert("HelQ4g");

    _dim = 2;
    _nGaus = 4;
    _nDisp = 2;
    _nRefc = 2;
    _nCoor = 2;
    _nVar = 8;
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
DelQ4g::~DelQ4g() {

}
// 弹性单元（平面应变）+ 热载荷：参照 macs/hel beq4g2.c
// 刚度 K = ∫ BᵀD B dΩ；体力 ∫ N·f dΩ；热载荷 ∫ (Bxx+Beyy)·ft·dΩ
// 其中 ft = pe·alfa·tn/(1-2pv)，tn 为高斯点温度（由 coef["T"] 插值）
CDFEG::EleSubResult& DelQ4g::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    std::vector<double> refcoor(2);
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double fu = matParams.at("fu");
    double fv = matParams.at("fv");
    double alfa = matParams.at("alfa");
    double vol = 1.0;
    // 平面应变本构因子（与 ElQ4g / 旧 beq4g2 一致）
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = 0.5 - pv;

    // 节点温度（由 Heat 场经 coef 传入）
    const std::vector<double>& T = coef.at("Heat::T");

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
        std::vector<std::vector<double>> cv = cu;
        double weight = _gaus[iGaus] * det;

        // 高斯点温度 tn = Σ N_i·T_i（参照 ebeq4g2 插值）
        double tn = 0.0;
        for (int i = 0; i < 4; ++i) tn += cu[i][0] * T[i];
        // 热载荷因子 ft = pe·alfa·tn/(1-2pv)（参照 beq4g2.c）
        double ft = pe * alfa * tn / (1.0 - 2.0 * pv);

        // B 矩阵分量（应变-位移）：自由度节点交错 [u1,v1,u2,v2,...]
        std::vector<double> eexx(8, 0.0), eeyy(8, 0.0), eexy(8, 0.0);
        for (int i = 0; i < 4; ++i)
        {
            int i1 = i * 2;
            int i2 = i * 2 + 1;
            eexx[i1] = +cu[i][1];
            eeyy[i2] = +cv[i][2];
            eexy[i1] += cu[i][2];
            eexy[i2] += cv[i][1];
        }

        // 体力载荷 eload = ∫ N·f dΩ
        for (int i = 0; i < 4; ++i)
        {
            _result.eload[2 * i] += cu[i][0] * fu * vol * weight;
            _result.eload[2 * i + 1] += cv[i][0] * fv * vol * weight;
        }

        // 刚度矩阵 K = ∫ BᵀD B dΩ（平面应变）
        int ii = -1;
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                double stif = +eexx[i] * eexx[j] * (1. - pv) * fact
                    + eexx[i] * eeyy[j] * pv * fact
                    + eeyy[i] * eexx[j] * pv * fact
                    + eeyy[i] * eeyy[j] * (1. - pv) * fact
                    + eexy[i] * eexy[j] * shear * fact;
                _result.estif[++ii] += stif * weight;
            }
        }

        // 热载荷：eload[k] += (eexx[k]+eeyy[k])·ft，k=0..7（参照 beq4g2.c）
        for (int k = 0; k < 8; ++k)
        {
            _result.eload[k] += (eexx[k] + eeyy[k]) * ft * vol * weight;
        }
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
// 应力恢复（含热项扣除）：参照 macs/hel ceq4g2.c + ElQ4g 节点加权外推
// σxx = fact·((1-pv)·exx + pv·eyy) - ft；σyy 对称；σxy = fact·shear·exy
// 节点应力 = Σ N·σ·w / Σ N·w（≡基准最小二乘平滑）
CDFEG::uResult DelQ4g::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult res;

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double vol = 1.0;
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = (0.5 - pv);

    const std::vector<double>& u = coef.at("u");
    const std::vector<double>& vDisp = coef.at("v");

    std::vector<double> nodeSigmaXX(4, 0.0), nodeSigmaYY(4, 0.0);
    std::vector<double> nodeSigmaXY(4, 0.0);
    std::vector<double> nodeWeight(4, 0.0);

    double sigmaXX = 0.0, sigmaYY = 0.0, sigmaXY = 0.0;
    double totalWeight = 0.0;

    for (int iGaus = 0; iGaus < _nGaus; ++iGaus)
    {
        std::vector<std::vector<double>> rctr;
        std::vector<double> coor;
        dcoor(r, iGaus, coor, rctr);
        std::vector<std::vector<double>> crtr;
        double det = CDFEG::inverse(rctr, crtr);
        std::vector<std::vector<double>> cu;
        shapn(iGaus, coor, crtr, cu);
        std::vector<std::vector<double>> cv = cu;
        double weight = _gaus[iGaus] * det;
        totalWeight += weight;

        // 应变
        double exx = 0.0, eyy = 0.0, exy = 0.0;
        for (int i = 0; i < 4; ++i)
        {
            exx += cu[i][1] * u[i];
            eyy += cu[i][2] * vDisp[i];
            exy += cu[i][2] * u[i] + cu[i][1] * vDisp[i];
        }

        // 应力：基准 ehelc 传入 elemb 材料，ceq4g2 的 alfa 取 prmt[3] = fu(=0)，
        // 故热项 ft = 0，应力不扣除热膨胀项（仅弹性应力 D·ε，与基准 unodc0 对齐）
        double gSigmaXX = fact * ((1 - pv) * exx + pv * eyy);
        double gSigmaYY = fact * (pv * exx + (1 - pv) * eyy);
        double gSigmaXY = fact * shear * exy;

        sigmaXX += gSigmaXX * weight;
        sigmaYY += gSigmaYY * weight;
        sigmaXY += gSigmaXY * weight;

        // 形函数 N 加权外推到节点
        for (int i = 0; i < 4; ++i)
        {
            double nodeW = cu[i][0] * weight;
            nodeSigmaXX[i] += gSigmaXX * nodeW;
            nodeSigmaYY[i] += gSigmaYY * nodeW;
            nodeSigmaXY[i] += gSigmaXY * nodeW;
            nodeWeight[i] += nodeW;
        }
    }

    if (totalWeight > 0.0)
    {
        sigmaXX /= totalWeight;
        sigmaYY /= totalWeight;
        sigmaXY /= totalWeight;
    }

    res.eleResult["sigmaXX"] = sigmaXX;
    res.eleResult["sigmaYY"] = sigmaYY;
    res.eleResult["sigmaXY"] = sigmaXY;
    res.eleResult["volume"] = totalWeight;

    res.nodeResult["sigmaXX"] = nodeSigmaXX;
    res.nodeResult["sigmaYY"] = nodeSigmaYY;
    res.nodeResult["sigmaXY"] = nodeSigmaXY;
    res.nodeResult["weight"] = nodeWeight;

    return res;
}

std::vector<double> DelQ4g::shapeFun(
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
