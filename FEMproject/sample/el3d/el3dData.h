#ifndef EL3D_DATA_H
#define EL3D_DATA_H
#include <string>
#include "SIFEG/FemData.h"

class el3dData : public SIFEG::FEMData {
public:
    el3dData();
    ~el3dData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif