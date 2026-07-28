#include "ElT3.h"
#include "CDFEG/MatrixFun.h"
#include "ElasticT3Data.h"
#include "Elastic2DDispFieldData.h"
#include <cmath>
#include <vector>

static std::vector<std::vector<double>> calcBD(double E, double nu) {
    double coeff = E / (1.0 - nu * nu);
    return {
        { coeff,      coeff * nu, 0                        },
        { coeff * nu, coeff,      0                        },
        { 0,          0,          coeff * (1.0 - nu) / 2.0 }
    };
}

ElT3::ElT3(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(3, pData) {
    _name="ElT3";
    _dispNames = { "u", "v" };
    _mateTypeName = "ElT3";
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
    double x1 = r[0], y1 = r[1];
    double x2 = r[2], y2 = r[3];
    double x3 = r[4], y3 = r[5];

    double b1 = y2 - y3, b2 = y3 - y1, b3 = y1 - y2;
    double c1 = x3 - x2, c2 = x1 - x3, c3 = x2 - x1;
    double Area = 0.5 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

    double E = matParams.at("E");
    double nu = matParams.at("nu");
    double t = matParams.at("t");
    double fx = matParams.at("fx");
    double fy = matParams.at("fy");

    auto D = calcBD(E, nu);

    double inv2A = 1.0 / (2.0 * Area);
    std::vector<std::vector<double>> Bmat = {
        { b1*inv2A, 0,        b2*inv2A, 0,        b3*inv2A, 0        },
        { 0,        c1*inv2A, 0,        c2*inv2A, 0,        c3*inv2A },
        { c1*inv2A, b1*inv2A, c2*inv2A, b2*inv2A, c3*inv2A, b3*inv2A }
    };

    // 单元刚度矩阵 Ke = t * Area * B^T * D * B（行主序一维，直接写入 estif）
    int n = 6;
    _result.estif = CDFEG::computeBTDB(t * Area, Bmat, D);

    _result.eload.resize(n, 0.0);
    for (int i = 0; i < 3; i++) {
        _result.eload[2 * i]     = t * Area * fx / 3.0;
        _result.eload[2 * i + 1] = t * Area * fy / 3.0;
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

CDFEG::uResult ElT3::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    CDFEG::uResult res;

    double x1 = r[0], y1 = r[1];
    double x2 = r[2], y2 = r[3];
    double x3 = r[4], y3 = r[5];

    double b1 = y2 - y3, b2 = y3 - y1, b3 = y1 - y2;
    double c1 = x3 - x2, c2 = x1 - x3, c3 = x2 - x1;
    double Area = 0.5 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

    double E = matParams.at("E");
    double nu = matParams.at("nu");

    auto D = calcBD(E, nu);

    double inv2A = 1.0 / (2.0 * Area);
    std::vector<std::vector<double>> Bmat = {
        { b1*inv2A, 0,        b2*inv2A, 0,        b3*inv2A, 0        },
        { 0,        c1*inv2A, 0,        c2*inv2A, 0,        c3*inv2A },
        { c1*inv2A, b1*inv2A, c2*inv2A, b2*inv2A, c3*inv2A, b3*inv2A }
    };

    const auto& u = coef.at("u");
    const auto& v = coef.at("v");
    double disp[6] = { u[0], v[0], u[1], v[1], u[2], v[2] };
    // 常应变三角形：应力在整个单元内为常数
    const char* stressNames[3] = { "Sxx", "Syy", "Sxy" };
    double sigma[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < 3; i++) {
        for (int k = 0; k < 6; k++) {
            double DB_val = 0;
            for (int m = 0; m < 3; m++) {
                DB_val += D[i][m] * Bmat[m][k];
            }
            sigma[i] += DB_val * disp[k];
        }
        res.eleResult[stressNames[i]] = sigma[i];
    }

    // 节点应力外推：常应变→3 节点应力相同；nodeResult 存"已乘权"的应力贡献（与 ElT3g 约定一致，
    // uPhy 再做 sum(σ·w)/sum(w) 加权平均），权 = ∫N_i dA = Area/3（线性形函数）
    double w = Area / 3.0;
    for (int i = 0; i < 3; ++i) {
        double sw = sigma[i] * w;
        res.nodeResult[stressNames[i]] = { sw, sw, sw };
    }
    res.nodeResult["weight"] = { w, w, w };

    return res;
}
