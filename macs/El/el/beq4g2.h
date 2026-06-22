#ifndef BEQ4G2_H
#define BEQ4G2_H
#include "CDFEG/IsoEleBase.h"

class beq4g2 : public CDFEG::IsoEleBase {
public:
    beq4g2(CDFEG::PhyFieldData* pData);
    ~beq4g2();

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