#ifndef A2LL2_H
#define A2LL2_H
#include "CDFEG/IsoEleBase.h"

class a2ll2 : public CDFEG::IsoEleBase {
public:
    a2ll2(CDFEG::PhyFieldData* pData);
    ~a2ll2();

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