#include "Elastic2DData.h"
#include "Elastic2DDispFieldData.h"

Elastic2DData::Elastic2DData() {
    _dim = 2;
    _phyDatas.push_back(new Elastic2DDispFieldData(this));
}

Elastic2DData::~Elastic2DData() {

}

int Elastic2DData::caculate() {

    return 1;
}

int Elastic2DData::main() {
    caculate();
    return 1;
}