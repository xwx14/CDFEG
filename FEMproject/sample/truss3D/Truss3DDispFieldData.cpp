#include "Truss3DDispFieldData.h"
#include "Truss3DData.h"
#include "Truss3D.h"

Truss3DDispFieldData::Truss3DDispFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(3, femData) {
    _name = "Truss3DDisp";
    _dispNames = { "u", "v", "w" };
    _dof2 = 3;
    _eleSubs.push_back(new Truss3D(this));
    _eleResNames = { "T","sigma" };
}

Truss3DDispFieldData::~Truss3DDispFieldData() {

}
