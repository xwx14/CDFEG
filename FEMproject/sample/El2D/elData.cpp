#include "elData.h"
#include "ElDispFieldData.h"

elData::elData() {
    _dim = 2;
    _phyDatas.push_back(new ElDispFieldData(this));
    _mateConstitutive["ElQ4g"] = { "pe", "pv", "fu", "fv", "rou", "alpha" };
    _mateConstitutive["ElT3g"] = { "pe", "pv", "fu", "fv", "rou", "alpha" };
    _mateConstitutive["StressBL2g"] = { "fu", "fv" };
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