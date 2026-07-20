#ifndef DELDISP_FIELD_DATA_H
#define DELDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class DelDispFieldData : public CDFEG::PhyFieldData {
public:
    DelDispFieldData(CDFEG::DomainData* femData);
    ~DelDispFieldData();

    // 重写 eProgram：组装弹性总刚 + 体力/热载荷（取 Heat 场温度）→ 右端项
    virtual int eProgram() override;
    // 重写 uPhy：回填位移 + 应力恢复（需 Heat 场温度参与热项扣除）
    virtual int uPhy() override;

};

#endif