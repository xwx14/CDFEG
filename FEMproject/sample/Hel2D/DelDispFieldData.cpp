#include "DelDispFieldData.h"
#include "hel2dData.h"
#include "DelQ4g.h"

DelDispFieldData::DelDispFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="DelDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new DelQ4g(this));
    _eleResNames = { "sigmaXX", "sigmaYY", "sigmaXY", "volume" };
}

DelDispFieldData::~DelDispFieldData() {

}
