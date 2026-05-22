#ifndef ELT3_H
#define ELT3_H
#include "CDFEG/ElementBase.h"

class ElT3 : public CDFEG::ElementBase {
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
};

#endif