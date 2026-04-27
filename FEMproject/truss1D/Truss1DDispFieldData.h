#ifndef TRUSS1DDISP_FIELD_DATA_H
#define TRUSS1DDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class Truss1DDispFieldData : public CDFEG::PhyFieldData {
public:
    Truss1DDispFieldData(CDFEG::FEMData* femData);
    ~Truss1DDispFieldData();


};

#endif