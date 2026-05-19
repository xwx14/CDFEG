#ifndef ELASTIC2DDISP_FIELD_DATA_H
#define ELASTIC2DDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class Elastic2DDispFieldData : public CDFEG::PhyFieldData {
public:
    Elastic2DDispFieldData(CDFEG::FEMData* femData);
    ~Elastic2DDispFieldData();


};

#endif