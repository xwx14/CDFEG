#include "elData.h"
#include "ElDispFieldData.h"

elData::elData() {
    _dim = 2;
    _phyDatas.push_back(new ElDispFieldData(this));
}

elData::~elData() {

}

int elData::caculate() {
	ElDispFieldData* aField = static_cast<ElDispFieldData*>(_phyDatas[0]);
	aField->initMatrix();
	aField->eProgram_el();
	aField->solve();
	aField->uPhy();
	aField->_equSys.calRightVals();
	return 1;
}

int elData::main() {
    caculate();
    return 1;
}