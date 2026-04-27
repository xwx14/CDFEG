#include "Truss3D.h"
#include "Truss3DData.h"
#include "Truss3DDispFieldData.h"
#include "CDFEG\MatrixFun.h"

Truss3D::Truss3D(CDFEG::PhyFieldData* pData)
    : CDFEG::EleSubBase(3, pData) {
    _name = "Truss3D";
    _dispNames = { "u", "v", "w" };
    _paramNames = { "E", "A" };
    _types.insert("Truss3D");
}

Truss3D::~Truss3D() {

}

CDFEG::EleSubResult& Truss3D::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    double E = matParams.at("E");
    double A = matParams.at("A");

    // 计算3D方向余弦
    std::array<double, 4> dirVal = CDFEG::calcDir3D2(r);
    double l = dirVal[0];  // cos(alpha) - x方向
    double m = dirVal[1];  // cos(beta) - y方向
    double n = dirVal[2];  // cos(gamma) - z方向
    double L = dirVal[3];  // 单元长度

    // 预计算系数
    double EA_L = A * E / L;

    // 3D桁架单元刚度矩阵 (6x6)
    // 节点顺序: [u1, v1, w1, u2, v2, w2]
    _result.estif = {
        EA_L * l * l,     EA_L * l * m,     EA_L * l * n,    -EA_L * l * l,    -EA_L * l * m,    -EA_L * l * n,
        EA_L * m * l,     EA_L * m * m,     EA_L * m * n,    -EA_L * m * l,    -EA_L * m * m,    -EA_L * m * n,
        EA_L * n * l,     EA_L * n * m,     EA_L * n * n,    -EA_L * n * l,    -EA_L * n * m,    -EA_L * n * n,
       -EA_L * l * l,    -EA_L * l * m,    -EA_L * l * n,     EA_L * l * l,     EA_L * l * m,     EA_L * l * n,
       -EA_L * m * l,    -EA_L * m * m,    -EA_L * m * n,     EA_L * m * l,     EA_L * m * m,     EA_L * m * n,
       -EA_L * n * l,    -EA_L * n * m,    -EA_L * n * n,     EA_L * n * l,     EA_L * n * m,     EA_L * n * n
    };

    _result.eload.resize(6, 0.0);

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

CDFEG::uResult Truss3D::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult res;
    double E = matParams.at("E");
    double A = matParams.at("A");

    std::array<double, 4> dirVal = CDFEG::calcDir3D2(r);
    double l = dirVal[0];
    double m = dirVal[1];
    double n = dirVal[2];
    double L = dirVal[3];

    double u1 = coef.at("u").at(0);
    double u2 = coef.at("u").at(1);
    double v1 = coef.at("v").at(0);
    double v2 = coef.at("v").at(1);
    double w1 = coef.at("w").at(0);
    double w2 = coef.at("w").at(1);

    double axialDisp = l * (u2 - u1) + m * (v2 - v1) + n * (w2 - w1);
    res.eleResult["T"] = A * E / L * axialDisp;
    res.eleResult["sigma"] = E / L * axialDisp;

    return res;
}
