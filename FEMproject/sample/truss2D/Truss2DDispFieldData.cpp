#include "Truss2DDispFieldData.h"
#include "Truss2DData.h"
#include "Truss2D.h"

Truss2DDispFieldData::Truss2DDispFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="Truss2DDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new Truss2D(this));
    _eleResNames = { "T","sigma" };
}

Truss2DDispFieldData::~Truss2DDispFieldData() {

}
