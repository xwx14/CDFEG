#include "Truss2DData.h"
#include "Truss2DDispFieldData.h"

Truss2DData::Truss2DData() {
    _dim = 2;
    _phyDatas.push_back(new Truss2DDispFieldData(this));
}

Truss2DData::~Truss2DData() {

}

int Truss2DData::caculate() {
    Truss2DDispFieldData* f = static_cast<Truss2DDispFieldData*>(_phyDatas[0]);
    f->initMatrix();
    f->eProgram_el();
    f->solve();
    f->uPhy();
    f->_equSys.calRightVals();
    return 1;
    return 1;
}

int Truss2DData::main() {
    caculate();
    return 1;
}