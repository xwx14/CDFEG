#include "el3dData.h"
#include "aFieldData.h"

el3dData::el3dData() {
    _dim = 3;
    _phyDatas.push_back(new aFieldData(this));
}

el3dData::~el3dData() {

}

int el3dData::caculate() {
    aFieldData* aField = static_cast<aFieldData*>(_phyDatas[0]);
    aField->initMatrix();
    aField->eProgram_el();
    aField->solve();
    aField->uPhy();
    aField->_equSys.calRightVals();
    return 1;
}

int el3dData::main() {
    caculate();
    return 1;
}
