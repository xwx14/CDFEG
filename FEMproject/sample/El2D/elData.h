#ifndef EL_DATA_H
#define EL_DATA_H
#include <string>
#include "SIFEG/FemData.h"

class elData : public SIFEG::FEMData {
public:
    elData();
    ~elData();
    virtual int caculate() override;
};

#endif