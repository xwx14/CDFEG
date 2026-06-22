#ifndef A1EQ4G2_H
#define A1EQ4G2_H
#include "CDFEG/IsoEleBase.h"

class a1eq4g2 : public CDFEG::IsoEleBase {
public:
    a1eq4g2(CDFEG::PhyFieldData* pData);
    ~a1eq4g2();

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

    virtual std::vector<double> shapeFun(
        const std::vector<double>& refc
    ) override;
};

#endif