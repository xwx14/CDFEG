#ifndef ELB_FIELD_DATA_H
#define ELB_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class elbFieldData : public CDFEG::PhyFieldData {
public:
    elbFieldData(CDFEG::FEMData* femData);
    ~elbFieldData();


};

#endif