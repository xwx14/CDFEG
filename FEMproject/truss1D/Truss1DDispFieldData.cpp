#include "Truss1DDispFieldData.h"
#include "Truss1DData.h"
#include "Truss1D.h"

Truss1DDispFieldData::Truss1DDispFieldData(SIFEG::FEMData* femData)
    : SIFEG::PhyFieldData(1, femData) {
    _name="Truss1DDisp";
    _dispNames = { "u"};
    _dof2 = 0;
    _eleSubs.push_back(new Truss1D(this));
    _eleResNames = { "T"};
}

Truss1DDispFieldData::~Truss1DDispFieldData() {

}