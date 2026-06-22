#include "elData.h"
#include "elaFieldData.h"
#include "elbFieldData.h"

elData::elData() {
    _dim = 2;
    _phyDatas.push_back(new elaFieldData(this));
    _phyDatas.push_back(new elbFieldData(this));
}

elData::~elData() {

}

int elData::caculate() {

    return 1;
}

int elData::main() {
    caculate();
    return 1;
}