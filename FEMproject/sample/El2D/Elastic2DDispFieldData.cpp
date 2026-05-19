#include "Elastic2DDispFieldData.h"
#include "Elastic2DData.h"
#include "ElT3.h"
#include "ElQ4.h"

Elastic2DDispFieldData::Elastic2DDispFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="Elastic2DDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new ElT3(this));
    _eleSubs.push_back(new ElQ4(this));
    _eleResNames = { "Sxx", "Syy", "Sxy" };
}

Elastic2DDispFieldData::~Elastic2DDispFieldData() {

}
