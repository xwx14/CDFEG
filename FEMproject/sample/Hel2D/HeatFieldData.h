#ifndef HEAT_FIELD_DATA_H
#define HEAT_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class HeatFieldData : public CDFEG::PhyFieldData {
public:
    HeatFieldData(CDFEG::FEMData* femData);
    ~HeatFieldData();


};

#endif