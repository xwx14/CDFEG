#ifndef EL2D_DATA_H
#define EL2D_DATA_H
#include <string>
#include "CDFEG/DomainData.h"

class El2DData : public CDFEG::DomainData {
public:
    El2DData();
    ~El2DData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif