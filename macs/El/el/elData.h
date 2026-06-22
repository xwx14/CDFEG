#ifndef EL_DATA_H
#define EL_DATA_H
#include <string>
#include "CDFEG/FemData.h"

class elData : public CDFEG::FEMData {
public:
    elData();
    ~elData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif