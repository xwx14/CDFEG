#include "a3lq4.h"
#include "el3dData.h"
#include "aFieldData.h"
#include "SIFEG\MatrixFun.h"
#include <cmath>
#include <vector>
#include <map>

a3lq4::a3lq4(SIFEG::PhyFieldData* pData)
    : SIFEG::Q4g(3, pData) {  // 3个位移分量 (u, v, w)
    _name = "a3lq4";
    _dispNames = {"u", "v", "w"};
    _paramNames = {"fu", "fv", "fw"};
    _types.insert("a3lq4");

    // Q4g 基类已经设置了：
    // - 2D 参考坐标和物理坐标
    // - 4 节点
    // - 2×2 高斯积分
    // - 形函数系数 (_refShapCoef)
}

a3lq4::~a3lq4() {

}

// 坐标转换函数：从 3D 投影到 2D 平面
// 实现类似 smit 函数的功能
// 输入：r - 4 个节点的 3D 坐标 [x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4]
// 输出：y2d - 4 个节点的 2D 坐标，transMat - 3×3 转换矩阵
void a3lq4::coord3DTo2D(
    const std::vector<double>& r,
    std::vector<double>& y2d,
    std::vector<std::vector<double>>& transMat
) {
    // 节点坐标
    double x1 = r[0], y1 = r[1], z1 = r[2];
    double x2 = r[3], y2 = r[4], z2 = r[5];
    double x3 = r[6], y3 = r[7], z3 = r[8];
    double x4 = r[9], y4 = r[10], z4 = r[11];

    // 计算两个平面向量
    double v1x = x2 - x1, v1y = y2 - y1, v1z = z2 - z1;
    double v2x = x3 - x1, v2y = y3 - y1, v2z = z3 - z1;

    // 构造局部坐标系
    // 局部 x 轴：沿 v1 方向
    double lx = v1x, ly = v1y, lz = v1z;
    double l_len = std::sqrt(lx*lx + ly*ly + lz*lz);
    lx /= l_len; ly /= l_len; lz /= l_len;

    // 局部 z 轴：v1 × v2（平面法向量）
    double nz = ly*v2z - lz*v2y;
    double nx = lz*v2x - lx*v2z;
    double ny = lx*v2y - ly*v2x;
    double n_len = std::sqrt(nx*nx + ny*ny + nz*nz);
    nx /= n_len; ny /= n_len; nz /= n_len;

    // 局部 y 轴：n × l
    double mx = ly*nz - lz*ny;
    double my = lz*nx - lx*nz;
    double mz = lx*ny - ly*nx;

    // 转换矩阵：从局部坐标系到全局坐标系
    // [l  m  n] - 每列是局部坐标轴在全局坐标系中的分量
    transMat.resize(3, std::vector<double>(3));
    transMat[0][0] = lx; transMat[0][1] = mx; transMat[0][2] = nx;
    transMat[1][0] = ly; transMat[1][1] = my; transMat[1][2] = ny;
    transMat[2][0] = lz; transMat[2][1] = mz; transMat[2][2] = nz;

    // 将所有节点投影到局部 2D 平面
    y2d.resize(8);
    for (int i = 0; i < 4; ++i) {
        double xi = r[i*3 + 0] - x1;
        double yi = r[i*3 + 1] - y1;
        double zi = r[i*3 + 2] - z1;

        // 在局部坐标系中的坐标（只需要前两个分量）
        y2d[i*2 + 0] = lx*xi + ly*yi + lz*zi;
        y2d[i*2 + 1] = mx*xi + my*yi + mz*zi;
    }
}

// 构造 12×12 的转换矩阵 T
// 将 2D 单元的结果转换到 3D
void a3lq4::buildTransformMatrix(
    const std::vector<std::vector<double>>& transMat,
    std::vector<std::vector<double>>& T
) {
    T.resize(12, std::vector<double>(12, 0.0));

    // 转换矩阵模式：对于每个节点 i，使用相同的 3×3 转换
    // T 是对角块矩阵，每块是 transMat 的转置
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                T[i*3 + j][i*3 + k] = transMat[j][k];
            }
        }
    }
}

// 2D 单元计算（原来的 run 函数）
SIFEG::EleSubResult& a3lq4::run2D(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    // 清空结果
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);

    // 获取载荷
    double fu = matParams.at("fu");
    double fv = matParams.at("fv");
    double fw = matParams.at("fw");

    // 高斯积分循环
    for (int igaus = 0; igaus < _nGaus; ++igaus) {
        // 计算雅可比矩阵和行列式（使用基类的 dcoor）
        std::vector<double> x;
        std::vector<std::vector<double>> dfdx;
        dcoor(r, igaus, x, dfdx, 1);

        // 计算行列式
        double det = dfdx[0][0] * dfdx[1][1] - dfdx[0][1] * dfdx[1][0];
        double weigh = det * _gaus[igaus];

        // 获取形函数值（使用基类预计算的 _refShapCoef）
        std::vector<double> N(_nNode);
        for (int i = 0; i < _nNode; ++i) {
            N[i] = _refShapCoef[igaus][0][i];
        }

        // 组装刚度矩阵（简化版本）
        for (int i = 0; i < _nNode; ++i) {
            for (int j = 0; j < _nNode; ++j) {
                // u-u 刚度
                int iv = i * 3 + 0;
                int jv = j * 3 + 0;
                double stiff_uu = N[i] * N[j] * weigh;
                _result.estif[iv * _nVar + jv] += stiff_uu;

                // v-v 刚度
                iv = i * 3 + 1;
                jv = j * 3 + 1;
                double stiff_vv = N[i] * N[j] * weigh;
                _result.estif[iv * _nVar + jv] += stiff_vv;

                // w-w 刚度
                iv = i * 3 + 2;
                jv = j * 3 + 2;
                double stiff_ww = N[i] * N[j] * weigh;
                _result.estif[iv * _nVar + jv] += stiff_ww;
            }
        }

        // 组装载荷向量
        for (int i = 0; i < _nNode; ++i) {
            int iv = i * 3 + 0;  // u 自由度
            _result.eload[iv] += N[i] * fu * weigh;
        }
        for (int i = 0; i < _nNode; ++i) {
            int iv = i * 3 + 1;  // v 自由度
            _result.eload[iv] += N[i] * fv * weigh;
        }
        for (int i = 0; i < _nNode; ++i) {
            int iv = i * 3 + 2;  // w 自由度
            _result.eload[iv] += N[i] * fw * weigh;
        }
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

// 3D 单元主函数（实现 2D 到 3D 的转换）
SIFEG::EleSubResult& a3lq4::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    // Step 1: 将 3D 坐标转换到 2D 局部坐标系
    std::vector<double> y2d;
    std::vector<std::vector<double>> transMat;
    coord3DTo2D(r, y2d, transMat);

    // Step 2: 调用 2D 单元计算
    SIFEG::EleSubResult& result2D = run2D(y2d, coef, matParams);

    // Step 3: 构造 12×12 转换矩阵
    std::vector<std::vector<double>> T;
    buildTransformMatrix(transMat, T);

    // Step 4: 转换刚度矩阵：K_3D = T^T * K_2D * T
    // 先计算 K_2D * T
    std::vector<std::vector<double>> K2D(12, std::vector<double>(12));
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            K2D[i][j] = result2D.estif[i * 12 + j];
        }
    }

    std::vector<std::vector<double>> temp = SIFEG::multiply(K2D, T);
    std::vector<std::vector<double>> T_trans = SIFEG::transpose(T);
    std::vector<std::vector<double>> K3D = SIFEG::multiply(T_trans, temp);

    // 将结果存回 _result.estif
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            _result.estif[i * 12 + j] = K3D[i][j];
        }
    }

    // Step 5: 转换载荷向量：F_3D = T^T * F_2D
    std::vector<double> F2D(12);
    for (int i = 0; i < 12; ++i) {
        F2D[i] = result2D.eload[i];
    }
    std::vector<double> F3D = SIFEG::multiply(T_trans, F2D);

    for (int i = 0; i < 12; ++i) {
        _result.eload[i] = F3D[i];
    }

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

// 后处理函数
SIFEG::uResult a3lq4::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    SIFEG::uResult res;

    // 计算位移
    auto u = coef.at("u");
    auto v = coef.at("v");
    auto w = coef.at("w");

    // 计算应变（简化）
    std::vector<double> strain(6);
    strain[0] = 0.0;  // ε_xx
    strain[1] = 0.0;  // ε_yy
    strain[2] = 0.0;  // ε_zz
    strain[3] = 0.0;  // γ_yz
    strain[4] = 0.0;  // γ_zx
    strain[5] = 0.0;  // γ_xy

    // 计算应力（假设各向同性线弹性）
    double E = 200000;  // 简化
    double nu = 0.3;

    std::vector<double> stress(6);
    // 平面应力/应变关系
    // 这里简化处理

    // 保存结果
    res.eleResult["von_mises"] = 0.0;
    res.nodeResult["displacement"] = {
        u[0], u[1], u[2], u[3],
        v[0], v[1], v[2], v[3],
        w[0], w[1], w[2], w[3]
    };

    return res;
}
