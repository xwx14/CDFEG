#include "hel2dData.h"
#include "HeatFieldData.h"
#include "DelDispFieldData.h"

hel2dData::hel2dData() {
    _dim = 2;
    _phyDatas.push_back(new HeatFieldData(this));
    _phyDatas.push_back(new DelDispFieldData(this));
    _mateConstitutive["HelQ4g"] = { "ek", "ec", "q", "pe", "pv", "fu", "fv", "rou", "alpha", "alfa" };
}

hel2dData::~hel2dData() {

}

int hel2dData::caculate() {
	HeatFieldData* phy0 = static_cast<HeatFieldData*>(_phyDatas[0]);
	phy0->initMatrix();
	phy0->eProgram();
	phy0->solve();
	phy0->uPhy();
	phy0->_equSys.calRightVals();
	DelDispFieldData* phy1 = static_cast<DelDispFieldData*>(_phyDatas[1]);
	phy1->initMatrix();
	phy1->eProgram();
	phy1->solve();
	phy1->uPhy();
	phy1->_equSys.calRightVals();

    return 1;
}

int hel2dData::main() {
    caculate();
    return 1;
}