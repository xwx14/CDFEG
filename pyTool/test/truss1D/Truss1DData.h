#ifndef TRUSS1D_DATA_H
#define TRUSS1D_DATA_H
#include <string>
#include "CDFEG/FemData.h"

class Truss1DData : public CDFEG::FEMData {
public:
    Truss1DData();
    ~Truss1DData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif