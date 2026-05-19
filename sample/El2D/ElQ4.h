#ifndef ELQ4_H
#define ELQ4_H
#include "CDFEG/IsoEleBase.h"

class ElQ4 : public CDFEG::IsoEleBase {
public:
    ElQ4(CDFEG::PhyFieldData* pData);
    ~ElQ4();

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

    virtual std::vector<double> coordTransFun(
        const std::vector<double>& refc,
        const std::vector<double>& coords
    ) override;
};

#endif