#include "a2ll2.h"
#include "elData.h"
#include "aFieldData.h"
#include "SIFEG/MatrixFun.h"

a2ll2::a2ll2(SIFEG::PhyFieldData* pData)
    : SIFEG::L2G(2, pData) {
    _name = "a2ll2";
    _dispNames = { "u", "v" };
    _paramNames = { "fu", "fv" };
    _types.insert("a2ll2");
    _vtkCellType = ::VTK_LINE;
}

a2ll2::~a2ll2() {

}

SIFEG::EleSubResult& a2ll2::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    _result.nodeIds.clear();
    std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
    std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
    std::fill(_result.edamp.begin(), _result.edamp.end(), 0.0);
    std::fill(_result.eload.begin(), _result.eload.end(), 0.0);

    double fu = matParams.at("fu");
    double fv = matParams.at("fv");

    // 构建 2D 节点坐标矩阵 r (k × m) = (_femData->_dim × _nNode)
    // r[0] = {x1, x2}, r[1] = {y1, y2}
    std::vector<std::vector<double>> r_coord(_femData->_dim, std::vector<double>(_nNode));
    for (int i = 0; i < _femData->_dim; ++i) {
        for (int j = 0; j < _nNode; ++j) {
            r_coord[i][j] = r[i * _nNode + j];
        }
    }

    // 调用 Schmidt 正交化方法计算坐标变换矩阵 z
    // z[i][j] 表示全局坐标轴 j 对应的局部坐标轴 i
    std::vector<std::vector<double>> z =
        SIFEG::computeTransformMatrix(_femData->_dim, _dim, _nNode, r_coord);

    // 计算局部坐标系下的节点坐标 y (nldim × nnode)
    // 由于 _dim=1, 只需要 y[0] 一行
    std::vector<std::vector<double>> y(_dim, std::vector<double>(_nNode, 0.0));
    for (int j = 0; j < _nNode; ++j) {
        y[0][j] = r_coord[0][j] * z[0][0] + r_coord[1][j] * z[1][0];
    }

    // 构建局部坐标向量 yy (nldim × nnode) = 1 × 2
    std::vector<double> yy(_nNode);
    for (int j = 0; j < _nNode; ++j) {
        yy[j] = y[0][j];
    }

    // 调用 run1D 在 1D 局部坐标系下计算矩阵
    // els: 刚度矩阵 (4×4)
    // elm: 质量矩阵 (4×4)
    // eld: 阻尼矩阵 (4×4)
    // ell: 荷载向量 (4×1)
    std::vector<std::vector<double>> els(_nVar, std::vector<double>(_nVar, 0.0));
    std::vector<std::vector<double>> elm(_nVar, std::vector<double>(_nVar, 0.0));
    std::vector<std::vector<double>> eld(_nVar, std::vector<double>(_nVar, 0.0));
    std::vector<double> ell(_nVar, 0.0);

    run1D(yy, fu, fv, els, elm, eld, ell);

    // 构建变换矩阵 t (ngvar × nlvar) = 4×4
    // t[iv][jv] 表示局部自由度 jv 对应的全局自由度 iv
    // 根据自由度顺序：节点1(u,v), 节点2(u,v)
    // 索引映射：u1->0, v1->1, u2->2, v2->3
    std::vector<std::vector<double>> t(_nVar, std::vector<double>(_nVar, 0.0));

    // 局部 u 方向 -> 全局投影
    t[0][0] = z[0][0];  // u1 = z[0] * u_local
    t[1][0] = z[1][0];  // v1 = z[1] * u_local
    t[2][2] = z[0][0];  // u2 = z[0] * u_local
    t[3][2] = z[1][0];  // v2 = z[1] * u_local

    // 局部 v 方向 -> 全局投影
    t[0][1] = z[0][1];  // u1 = z[0]' * v_local
    t[1][1] = z[1][1];  // v1 = z[1]' * v_local
    t[2][3] = z[0][1];  // u2 = z[0]' * v_local
    t[3][3] = z[1][1];  // v2 = z[1]' * v_local

    // tkt: T^T * els * T
    // 将局部刚度矩阵转换到全局坐标系
    auto estifTransformed = SIFEG::multiplyTAT(t, els);
    for (int i = 0; i < _nVar; ++i) {
        for (int j = 0; j < _nVar; ++j) {
            _result.estif[i * _nVar + j] = estifTransformed[i][j];
        }
    }

    // tmt: T^T * elm * T
    // 将局部质量矩阵转换到全局坐标系
    auto egsMass = SIFEG::multiplyTAT(t, elm);
    for (int i = 0; i < _nVar; ++i) {
        for (int j = 0; j < _nVar; ++j) {
            _result.emass[i * _nVar + j] = egsMass[i][j];
        }
    }

    // tl: T^T * ell
    // 将局部荷载向量转换到全局坐标系
    _result.eload = SIFEG::multiplyT(t, ell);

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

void a2ll2::run1D(
    const std::vector<double>& yy,
    double fu,
    double fv,
    std::vector<std::vector<double>>& els,
    std::vector<std::vector<double>>& elm,
    std::vector<std::vector<double>>& eld,
    std::vector<double>& ell
) {
    // 初始化矩阵为0
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            els[i][j] = 0.0;
            elm[i][j] = 0.0;
            eld[i][j] = 0.0;
        }
        ell[i] = 0.0;
    }

    // 自由度映射 (kvord)
    // u1->0, v1->1, u2->2, v2->3
    int kvord[] = {0, 2, 1, 3};

    // 遍历高斯积分点
    for (int igaus = 0; igaus < _nGaus; ++igaus) {
        // 参考坐标
        double refc = _refc[igaus];

        // 计算形函数 cu[2], cv[2] (2个节点)
        std::vector<double> refcVec = {refc};
        std::vector<double> shpr = shapeFun(refcVec);
        double cu[2] = {shpr[0], shpr[1]};  // u方向形函数
        double cv[2] = {shpr[0], shpr[1]};  // v方向形函数

        // 计算坐标雅可比矩阵
        std::vector<double> fx = coordTransFun(yy, refcVec);
        std::vector<std::vector<double>> dfdx(1, std::vector<double>(2));
        dcoor(yy, igaus, fx, dfdx, 1);

        // 计算雅可比行列式 (det = dx/dξ)
        double det = dfdx[0][0];

        // 积分权重
        double weigh = det * _gaus[igaus];

        // 刚度矩阵计算（线单元无刚度）
        for (int i = 0; i < 2; ++i) {
            int iv = kvord[i];
            for (int j = 0; j < 2; ++j) {
                int jv = kvord[j];
                els[iv][jv] += cu[i] * cu[j] * 0.0 * weigh;
            }
        }

        // 荷载向量计算
        // u 方向
        for (int i = 0; i < 2; ++i) {
            int iv = kvord[i];
            ell[iv] += cu[i] * fu * weigh;
        }
        // v 方向
        for (int i = 0; i < 2; ++i) {
            int iv = kvord[i + 2];
            ell[iv] += cv[i] * fv * weigh;
        }
    }
}

SIFEG::uResult a2ll2::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    SIFEG::uResult res;

    // 线单元通常不需要后处理计算
    // 后处理结果通常由其他单元类型提供

    return res;
}
