#include "ElDispFieldData.h"
#include "El2DData.h"
#include "ElQ4g.h"
#include "ElT3g.h"
#include "StressBL2g.h"

ElDispFieldData::ElDispFieldData(CDFEG::DomainData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="ElDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new ElQ4g(this));
    _eleSubs.push_back(new ElT3g(this));
    _eleSubs.push_back(new StressBL2g(this));
    _eleResNames = { "sigmaXX", "sigmaYY", "sigmaXY" };
    _nodeExtrapNames = { "sigmaXX", "sigmaYY", "sigmaXY" };
}

ElDispFieldData::~ElDispFieldData() {

}
