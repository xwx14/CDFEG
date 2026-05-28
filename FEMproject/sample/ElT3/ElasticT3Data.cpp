#include "ElasticT3Data.h"
#include "Elastic2DDispFieldData.h"

ElasticT3Data::ElasticT3Data() {
    _dim = 2;
    _phyDatas.push_back(new Elastic2DDispFieldData(this));
}

ElasticT3Data::~ElasticT3Data() {

}

int ElasticT3Data::caculate() {

    return 1;
}

int ElasticT3Data::main() {
    caculate();
    return 1;
}