#ifndef A1EQ4G2_H
#define A1EQ4G2_H
#include "SIFEG/Q4g.h"

class a1eq4g2 : public SIFEG::Q4g {
public:
    a1eq4g2(SIFEG::PhyFieldData* pData);
    ~a1eq4g2();

    virtual SIFEG::EleSubResult& run(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    virtual SIFEG::uResult uEle(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;
};

#endif