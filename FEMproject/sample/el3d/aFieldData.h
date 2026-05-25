#ifndef A_FIELD_DATA_H
#define A_FIELD_DATA_H
#include "SIFEG/PhyFieldData.h"

class aFieldData : public SIFEG::PhyFieldData {
public:
    aFieldData(SIFEG::FEMData* femData);
    ~aFieldData();

    virtual int uPhy() override;
};

#endif