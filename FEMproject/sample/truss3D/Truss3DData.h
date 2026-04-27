#ifndef TRUSS3D_DATA_H
#define TRUSS3D_DATA_H
#include <string>
#include "CDFEG/FemData.h"

class Truss3DData : public CDFEG::FEMData {
public:
    Truss3DData();
    ~Truss3DData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif
