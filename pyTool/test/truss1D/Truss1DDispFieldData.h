#ifndef TRUSS1DDISP_FIELD_DATA_H
#define TRUSS1DDISP_FIELD_DATA_H
#include "SIFEG/PhyFieldData.h"

class Truss1DDispFieldData : public SIFEG::PhyFieldData {
public:
    Truss1DDispFieldData(SIFEG::FEMData* femData);
    ~Truss1DDispFieldData();


};

#endif