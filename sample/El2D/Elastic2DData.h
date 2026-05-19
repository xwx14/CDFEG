#ifndef ELASTIC2D_DATA_H
#define ELASTIC2D_DATA_H
#include <string>
#include "CDFEG/FemData.h"

class Elastic2DData : public CDFEG::FEMData {
public:
    Elastic2DData();
    ~Elastic2DData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif