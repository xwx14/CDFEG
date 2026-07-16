#ifndef HEL2D_DATA_H
#define HEL2D_DATA_H
#include <string>
#include "CDFEG/FemData.h"

class hel2dData : public CDFEG::FEMData {
public:
    hel2dData();
    ~hel2dData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif