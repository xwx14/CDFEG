#include "HeatFieldData.h"
#include "hel2dData.h"
#include "HeatQ4g.h"
#include "CDFEG/ElementBase.h"
#include "CDFEG/EquationSystem.h"

HeatFieldData::HeatFieldData(CDFEG::DomainData* femData)
    : CDFEG::PhyFieldData(1, femData) {
    _name="Heat";
    _dispNames = { "T" };
    _dof2 = 1;
    _eleSubs.push_back(new HeatQ4g(this));
    _eleResNames = {  };
}

HeatFieldData::~HeatFieldData() {

}
