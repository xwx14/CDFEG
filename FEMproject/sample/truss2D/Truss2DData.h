#ifndef TRUSS2D_DATA_H
#define TRUSS2D_DATA_H
#include <string>
#include "CDFEG/FemData.h"

class Truss2DData : public CDFEG::FEMData {
public:
    Truss2DData();
    ~Truss2DData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif