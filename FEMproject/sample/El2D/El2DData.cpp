#include "El2DData.h"
#include "ElDispFieldData.h"

El2DData::El2DData() {
    _dim = 2;
    _phyDatas.push_back(new ElDispFieldData(this));
    _mateConstitutive["El"] = { "pe", "pv", "fu", "fv", "rou", "alpha" };
    _mateConstitutive["StressBL2g"] = { "fu", "fv" };
}

El2DData::~El2DData() {

}

int El2DData::caculate() {
	ElDispFieldData* phy0 = static_cast<ElDispFieldData*>(_phyDatas[0]);
	phy0->initMatrix();
	phy0->eProgram();
	phy0->solve();
	phy0->uPhy();
	phy0->_equSys.calRightVals();

    return 1;
}

int El2DData::main() {
    caculate();
    return 1;
}