#include "ElT3.h"
#include "ElasticT3Data.h"
#include "Elastic2DDispFieldData.h"
#include <cmath>
#include <vector>

static void calcBD(double b1, double b2, double b3,
                   double c1, double c2, double c3,
                   double E, double nu,
                   double D[3][3]) {
    double coeff = E / (1.0 - nu * nu);
    D[0][0] = coeff;            D[0][1] = coeff * nu;      D[0][2] = 0;
    D[1][0] = coeff * nu;       D[1][1] = coeff;           D[1][2] = 0;
    D[2][0] = 0;                D[2][1] = 0;               D[2][2] = coeff * (1.0 - nu) / 2.0;
}

ElT3::ElT3(CDFEG::PhyFieldData* pData)
    : CDFEG::ElementBase(3, pData) {
    _name = "ElT3";
    _dispNames = { "u", "v" };
    _paramNames = { "E", "nu","t", "fx", "fy"};
    _types.insert("ElT3");
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

    double D[3][3];
    calcBD(b1, b2, b3, c1, c2, c3, E, nu, D);

    double inv2A = 1.0 / (2.0 * Area);
    double Bmat[3][6] = {
        { b1*inv2A, 0,        b2*inv2A, 0,        b3*inv2A, 0        },
        { 0,        c1*inv2A, 0,        c2*inv2A, 0,        c3*inv2A },
        { c1*inv2A, b1*inv2A, c2*inv2A, b2*inv2A, c3*inv2A, b3*inv2A }
    };

    int n = 6;
    _result.estif.assign(n * n, 0.0);
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            double val = 0;
            for (int k = 0; k < 3; k++) {
                for (int m = 0; m < 3; m++) {
                    val += Bmat[k][i] * D[k][m] * Bmat[m][j];
                }
            }
            _result.estif[i * n + j] = t * Area * val;
        }
    }

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

    double D[3][3];
    calcBD(b1, b2, b3, c1, c2, c3, E, nu, D);

    double inv2A = 1.0 / (2.0 * Area);
    double Bmat[3][6] = {
        { b1*inv2A, 0,        b2*inv2A, 0,        b3*inv2A, 0        },
        { 0,        c1*inv2A, 0,        c2*inv2A, 0,        c3*inv2A },
        { c1*inv2A, b1*inv2A, c2*inv2A, b2*inv2A, c3*inv2A, b3*inv2A }
    };

    const auto& u = coef.at("u");
    const auto& v = coef.at("v");
    double disp[6] = { u[0], v[0], u[1], v[1], u[2], v[2] };
    double ll;
    for (int i = 0; i < 3; i++) {
        double stress_i = 0;
        for (int k = 0; k < 6; k++) {
            double DB_val = 0;
            for (int m = 0; m < 3; m++) {
                DB_val += D[i][m] * Bmat[m][k];
            }
            ll= DB_val * disp[k];
            stress_i +=ll;
        }
        if (i == 0) res.eleResult["Sxx"] = stress_i;
        else if (i == 1) res.eleResult["Syy"] = stress_i;
        else res.eleResult["Sxy"] = stress_i;
    }

    return res;
}
