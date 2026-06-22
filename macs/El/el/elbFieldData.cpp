#include "elbFieldData.h"
#include "elData.h"
#include "beq4g2.h"

elbFieldData::elbFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(3, femData) {
    _name="elb";
    _dispNames = { "dxx", "dyy", "dxy" };
    _dof2 = 3;
    _eleSubs.push_back(new beq4g2(this));
    _eleResNames = {  };
}

elbFieldData::~elbFieldData() {

}
