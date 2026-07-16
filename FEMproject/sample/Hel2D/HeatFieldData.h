#ifndef HEAT_FIELD_DATA_H
#define HEAT_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class HeatFieldData : public CDFEG::PhyFieldData {
public:
    HeatFieldData(CDFEG::DomainData* femData);
    ~HeatFieldData();

    // 重写 eProgram：基类 eProgram_el 不处理 eload→右端，热源载荷需手动累加到 _f
    virtual int eProgram() override;

};

#endif