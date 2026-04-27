#ifndef TRUSS1D_H
#define TRUSS1D_H
#include "CDFEG/EleSubBase.h"

class Truss1D : public CDFEG::EleSubBase {
public:
    Truss1D(CDFEG::PhyFieldData* pData);
    ~Truss1D();

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