#include "a1eq4g2.h"
#include "elData.h"
#include "elaFieldData.h"
#include "CDFEG/MatrixFun.h"
a1eq4g2::a1eq4g2(CDFEG::PhyFieldData* pData)
    : CDFEG::IsoEleBase(4, pData) {
    _name="a1eq4g2";
    _dispNames = { "u", "v" };
    _paramNames ={ "pe", "pv", "fu", "fv", "rou", "alpha" };
    _types.insert("a1eq4g2");

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
a1eq4g2::~a1eq4g2() {

}
CDFEG::EleSubResult& a1eq4g2::run(
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
    double fu = matParams.at("fu");
    double fv = matParams.at("fv");
    double vol = 1.0;
    // 平面应力 D 矩阵系数（与 a1eq4g2.c 一致）
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = 0.5 - pv;

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
        std::vector<std::vector<double>> cv = cu;
        double weight = _gaus[iGaus] * det;

        // B 矩阵分量（a1eq4g2.c:106-134）
        std::vector<double> eexx(8, 0.0);
        std::vector<double> eeyy(8, 0.0);
        std::vector<double> eexy(8, 0.0);
        for (int i = 0; i < 4; ++i)
        {
            int i1 = i * 2;
            int i2 = i * 2 + 1;
            eexx[i1] = +cu[i][1];     // du/dx 对 u 自由度
            eeyy[i2] = +cv[i][2];     // dv/dy 对 v 自由度
            eexy[i1] += cu[i][2];     // du/dy 对 u 自由度（剪切）
            eexy[i2] += cv[i][1];     // dv/dx 对 v 自由度（剪切）
        }

        // 单刚组装：K += B^T D B * weight（a1eq4g2.c:136-147）
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                double stif = +eexx[i] * eexx[j] * (1.0 - pv) * fact
                    + eexx[i] * eeyy[j] * pv * fact
                    + eeyy[i] * eexx[j] * pv * fact
                    + eeyy[i] * eeyy[j] * (1.0 - pv) * fact
                    + eexy[i] * eexy[j] * shear * fact;
                _result.estif[i * 8 + j] += stif * weight;
            }
        }

        // 载荷：体力 fu/fv（a1eq4g2.c:149-158）
        for (int i = 0; i < 4; ++i)
        {
            _result.eload[2 * i] += cu[i][0] * fu * vol * weight;
            _result.eload[2 * i + 1] += cv[i][0] * fv * vol * weight;
        }
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}
CDFEG::uResult a1eq4g2::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult res;

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double vol = 1.0;
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = 0.5 - pv;

    const std::vector<double>& u = coef.at("u");
    const std::vector<double>& vDisp = coef.at("v");

    // 节点应力累加器 + 权重
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

        // 应变 exx/eyy/exy（参考 ElQ4g::uEle）
        double exx = 0.0, eyy = 0.0, exy = 0.0;
        for (int i = 0; i < 4; ++i)
        {
            exx += cu[i][1] * u[i];
            eyy += cu[i][2] * vDisp[i];
            exy += cu[i][2] * u[i] + cu[i][1] * vDisp[i];
        }

        // 应力 sigma = D * epsilon
        double gSigmaXX = fact * ((1.0 - pv) * exx + pv * eyy);
        double gSigmaYY = fact * (pv * exx + (1.0 - pv) * eyy);
        double gSigmaXY = fact * shear * exy;

        sigmaXX += gSigmaXX * weight;
        sigmaYY += gSigmaYY * weight;
        sigmaXY += gSigmaXY * weight;

        // 形函数加权外推到节点
        for (int i = 0; i < 4; ++i)
        {
            double nodeW = cu[i][0] * weight;
            nodeSigmaXX[i] += gSigmaXX * nodeW;
            nodeSigmaYY[i] += gSigmaYY * nodeW;
            nodeSigmaXY[i] += gSigmaXY * nodeW;
            nodeWeight[i] += nodeW;
        }
    }

    // 单元平均应力（除以总权重）
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

std::vector<double> a1eq4g2::shapeFun(
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

