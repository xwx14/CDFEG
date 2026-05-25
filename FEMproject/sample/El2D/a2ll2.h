#ifndef A2LL2_H
#define A2LL2_H
#include "SIFEG/L2G.h"
#include <vector>
#include <map>

class a2ll2 : public SIFEG::L2G {
public:
    a2ll2(SIFEG::PhyFieldData* pData);
    ~a2ll2();

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

private:
    void run1D(
        const std::vector<double>& yy,
        double fu,
        double fv,
        std::vector<std::vector<double>>& els,
        std::vector<std::vector<double>>& elm,
        std::vector<std::vector<double>>& eld,
        std::vector<double>& ell
    );
};

#endif