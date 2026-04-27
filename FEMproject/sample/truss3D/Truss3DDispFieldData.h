#ifndef TRUSS3DDISP_FIELD_DATA_H
#define TRUSS3DDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class Truss3DDispFieldData : public CDFEG::PhyFieldData {
public:
    Truss3DDispFieldData(CDFEG::FEMData* femData);
    ~Truss3DDispFieldData();
};

#endif
