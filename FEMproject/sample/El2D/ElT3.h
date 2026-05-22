#ifndef ELT3_H
#define ELT3_H
#include "CDFEG/IsoEleBase.h"

class ElT3 : public CDFEG::IsoEleBase {
public:
    ElT3(CDFEG::PhyFieldData* pData);
    ~ElT3();

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

    //virtual std::vector<double> coordTransFun(
    //    const std::vector<double>& refc,
    //    const std::vector<double>& coords
    //) override;
};

#endif