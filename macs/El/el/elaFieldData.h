#ifndef ELA_FIELD_DATA_H
#define ELA_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class elaFieldData : public CDFEG::PhyFieldData {
public:
    elaFieldData(CDFEG::FEMData* femData);
    ~elaFieldData();


};

#endif