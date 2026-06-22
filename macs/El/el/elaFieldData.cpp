#include "elaFieldData.h"
#include "elData.h"
#include "a1eq4g2.h"
#include "a2ll2.h"

elaFieldData::elaFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="ela";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new a1eq4g2(this));
    _eleSubs.push_back(new a2ll2(this));
    _eleResNames = { "exx", "eyy", "exy" };
}

elaFieldData::~elaFieldData() {

}
