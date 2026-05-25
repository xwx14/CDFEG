#include "elData.h"
#include "aFieldData.h"

elData::elData() {
    _dim = 2;
    _phyDatas.push_back(new aFieldData(this));
}

elData::~elData() {

}

int elData::caculate() {
    aFieldData* aField = static_cast<aFieldData*>(_phyDatas[0]);
    aField->initMatrix();
    aField->eProgram_el();
    aField->solve();
    aField->uPhy();
    aField->_equSys.calRightVals();
    return 1;
}