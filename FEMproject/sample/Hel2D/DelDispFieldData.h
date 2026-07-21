#ifndef DELDISP_FIELD_DATA_H
#define DELDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class DelDispFieldData : public CDFEG::PhyFieldData {
public:
    DelDispFieldData(CDFEG::DomainData* femData);
    ~DelDispFieldData();

    // 重写 uPhy：回填位移 + 应力恢复
    virtual int uPhy() override;

};

#endif