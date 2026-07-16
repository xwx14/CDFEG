#ifndef ELASTICT3_DATA_H
#define ELASTICT3_DATA_H
#include <string>
#include "CDFEG/DomainData.h"

class ElasticT3Data : public CDFEG::DomainData {
public:
    ElasticT3Data();
    ~ElasticT3Data();
    virtual int caculate() override;
    virtual int main() override;
};

#endif