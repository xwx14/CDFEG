#ifndef A1EC8G2_H
#define A1EC8G2_H
#include "SIFEG/C8G.h"

class a1ec8g2 : public SIFEG::C8G {
public:
    a1ec8g2(SIFEG::PhyFieldData* pData);
    ~a1ec8g2();

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