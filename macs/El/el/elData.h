#ifndef EL_DATA_H
#define EL_DATA_H
#include <string>
#include "CDFEG/DomainData.h"

class elData : public CDFEG::DomainData {
public:
    elData();
    ~elData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif