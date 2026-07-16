#ifndef DELQ4G_H
#define DELQ4G_H
#include "CDFEG/IsoEleBase.h"

class DelQ4g : public CDFEG::IsoEleBase {
public:
    DelQ4g(CDFEG::PhyFieldData* pData);
    ~DelQ4g();

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