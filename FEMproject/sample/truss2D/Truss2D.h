#ifndef TRUSS2D_H
#define TRUSS2D_H
#include "CDFEG/EleSubBase.h"

class Truss2D : public CDFEG::EleSubBase {
public:
    Truss2D(CDFEG::PhyFieldData* pData);
    ~Truss2D();

    virtual CDFEG::EleSubResult& run(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    virtual CDFEG::uResult uEle(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;
};

#endif