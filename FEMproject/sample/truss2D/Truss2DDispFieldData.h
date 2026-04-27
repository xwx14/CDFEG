#ifndef TRUSS2DDISP_FIELD_DATA_H
#define TRUSS2DDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class Truss2DDispFieldData : public CDFEG::PhyFieldData {
public:
    Truss2DDispFieldData(CDFEG::FEMData* femData);
    ~Truss2DDispFieldData();


};

#endif