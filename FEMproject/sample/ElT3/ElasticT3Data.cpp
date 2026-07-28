#include "ElasticT3Data.h"
#include "Elastic2DDispFieldData.h"

ElasticT3Data::ElasticT3Data() {
    _dim = 2;
    _phyDatas.push_back(new Elastic2DDispFieldData(this));
    _mateConstitutive["ElT3"] = { "E", "nu", "t", "fx", "fy" };
}

ElasticT3Data::~ElasticT3Data() {

}

int ElasticT3Data::caculate() {
	Elastic2DDispFieldData* phy0 = static_cast<Elastic2DDispFieldData*>(_phyDatas[0]);
	phy0->initMatrix();
	phy0->eProgram();
	phy0->solve();
	phy0->uPhy();
	phy0->_equSys.calRightVals();

    return 1;
}

int ElasticT3Data::main() {
    caculate();
    return 1;
}