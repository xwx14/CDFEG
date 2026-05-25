#ifndef A2EW4G2_H
#define A2EW4G2_H
#include "SIFEG/W4G.h"

class a2ew4g2 : public SIFEG::W4G {
public:
    a2ew4g2(SIFEG::PhyFieldData* pData);
    ~a2ew4g2();

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