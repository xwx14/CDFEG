#ifndef HEATQ4G_H
#define HEATQ4G_H
#include "CDFEG/IsoEleBase.h"

class HeatQ4g : public CDFEG::IsoEleBase {
public:
    HeatQ4g(CDFEG::PhyFieldData* pData);
    ~HeatQ4g();

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