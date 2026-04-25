#ifndef TRUSS1D_H
#define TRUSS1D_H
#include "SIFEG/EleSubBase.h"

class Truss1D : public SIFEG::EleSubBase {
public:
    Truss1D(SIFEG::PhyFieldData* pData);
    ~Truss1D();

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