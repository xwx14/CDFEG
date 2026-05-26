#ifndef ELDISP_FIELD_DATA_H
#define ELDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class ElDispFieldData : public CDFEG::PhyFieldData {
public:
    ElDispFieldData(CDFEG::FEMData* femData);
    ~ElDispFieldData();

    virtual int uPhy() override;
};

#endif