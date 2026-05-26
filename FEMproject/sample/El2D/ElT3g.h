#ifndef ELT3G_H
#define ELT3G_H
#include "CDFEG/IsoEleBase.h"

class ElT3g : public CDFEG::IsoEleBase {
public:
    ElT3g(CDFEG::PhyFieldData* pData);
    ~ElT3g();

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