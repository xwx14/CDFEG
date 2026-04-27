#include "Truss3DData.h"
#include "Truss3DDispFieldData.h"

Truss3DData::Truss3DData() {
    _dim = 3;
    _phyDatas.push_back(new Truss3DDispFieldData(this));
}

Truss3DData::~Truss3DData() {

}

int Truss3DData::caculate() {
    Truss3DDispFieldData* f = static_cast<Truss3DDispFieldData*>(_phyDatas[0]);
    f->initMatrix();
    f->eProgram_el();
    f->solve();
    f->uPhy();
    f->_equSys.calRightVals();
    return 1;
}

int Truss3DData::main() {
    caculate();
    return 1;
}
