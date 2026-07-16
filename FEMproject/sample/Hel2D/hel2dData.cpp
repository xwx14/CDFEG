#include "hel2dData.h"
#include "HeatFieldData.h"
#include "DelDispFieldData.h"

hel2dData::hel2dData() {
    _dim = 2;
    _phyDatas.push_back(new HeatFieldData(this));
    _phyDatas.push_back(new DelDispFieldData(this));
}

hel2dData::~hel2dData() {

}

int hel2dData::caculate() {

    return 1;
}

int hel2dData::main() {
    caculate();
    return 1;
}