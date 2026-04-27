#ifndef TRUSS3D_H
#define TRUSS3D_H
#include "CDFEG/EleSubBase.h"

class Truss3D : public CDFEG::EleSubBase {
public:
    Truss3D(CDFEG::PhyFieldData* pData);
    ~Truss3D();

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
