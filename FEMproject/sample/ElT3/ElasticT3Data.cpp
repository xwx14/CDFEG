#include "ElasticT3Data.h"
#include "Elastic2DDispFieldData.h"

ElasticT3Data::ElasticT3Data() {
    _dim = 2;
    _phyDatas.push_back(new Elastic2DDispFieldData(this));
}

ElasticT3Data::~ElasticT3Data() {

}

int ElasticT3Data::caculate() {
	Elastic2DDispFieldData* f = static_cast<Elastic2DDispFieldData*>(_phyDatas[0]);
	f->initMatrix();
	f->eProgram();
	f->solve();
	f->uPhy();
	f->_equSys.calRightVals();
    return 1;
}

int ElasticT3Data::main() {
    caculate();
    return 1;
}