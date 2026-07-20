#ifndef DELDISP_FIELD_DATA_H
#define DELDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class DelDispFieldData : public CDFEG::PhyFieldData {
public:
    DelDispFieldData(CDFEG::DomainData* femData);
    ~DelDispFieldData();
    int eProgram();
    int uPhy();

};

#endif