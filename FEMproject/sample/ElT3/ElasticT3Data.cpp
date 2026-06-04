#include "ElasticT3Data.h"
#include "Elastic2DDispFieldData.h"

ElasticT3Data::ElasticT3Data() {
    _dim = 2;
    _phyDatas.push_back(new Elastic2DDispFieldData(this));
}

ElasticT3Data::~ElasticT3Data() {

}

int ElasticT3Data::caculate() {
	Elastic2DDispFieldData* aField = static_cast<Elastic2DDispFieldData*>(_phyDatas[0]);
	aField->initMatrix();
	aField->eProgram();
	aField->solve();
	aField->uPhy();
	aField->_equSys.calRightVals();
    return 1;
}

int ElasticT3Data::main() {
    caculate();
    return 1;
}