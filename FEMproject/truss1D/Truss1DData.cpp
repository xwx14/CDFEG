#include "Truss1DData.h"
#include "Truss1DDispFieldData.h"

Truss1DData::Truss1DData() {
    _dim = 1;
    _phyDatas.push_back(new Truss1DDispFieldData(this));
}

Truss1DData::~Truss1DData() {

}

int Truss1DData::caculate() {
    Truss1DDispFieldData* f = static_cast<Truss1DDispFieldData*>(_phyDatas[0]);
    f->initMatrix();
    f->eProgram_el();
    f->solve();
    f->uPhy();
    f->_equSys.calRightVals();
    return 1;
}

int Truss1DData::main() {
    caculate();
    return 1;
}