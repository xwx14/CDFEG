#ifndef ELASTICT3_DATA_H
#define ELASTICT3_DATA_H
#include <string>
#include "CDFEG/FemData.h"

class ElasticT3Data : public CDFEG::FEMData {
public:
    ElasticT3Data();
    ~ElasticT3Data();
    virtual int caculate() override;
    virtual int main() override;
};

#endif