#ifndef HEL2D_DATA_H
#define HEL2D_DATA_H
#include <string>
#include "CDFEG/DomainData.h"

class hel2dData : public CDFEG::DomainData {
public:
    hel2dData();
    ~hel2dData();
    virtual int caculate() override;
    virtual int main() override;
};

#endif