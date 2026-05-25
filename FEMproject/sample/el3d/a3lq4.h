#ifndef A3LQ4_H
#define A3LQ4_H
#include "SIFEG/Q4g.h"

class a3lq4 : public SIFEG::Q4g {
public:
    a3lq4(SIFEG::PhyFieldData* pData);
    ~a3lq4();

    // 坐标转换函数：从 3D 投影到 2D 平面
    void coord3DTo2D(
        const std::vector<double>& r,
        std::vector<double>& y2d,
        std::vector<std::vector<double>>& transMat
    );

    // 构造 12×12 的转换矩阵
    void buildTransformMatrix(
        const std::vector<std::vector<double>>& transMat,
        std::vector<std::vector<double>>& T
    );

    // 2D 单元计算（内部函数）
    SIFEG::EleSubResult& run2D(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    );

    // 3D 单元主函数（重写基类）
    virtual SIFEG::EleSubResult& run(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    // 后处理函数
    virtual SIFEG::uResult uEle(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;
};

#endif
