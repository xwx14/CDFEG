#include "DelDispFieldData.h"
#include "hel2dData.h"
#include "DelQ4g.h"
#include "CDFEG/ElementBase.h"
#include "CDFEG/EquationSystem.h"
#include <cmath>

DelDispFieldData::DelDispFieldData(CDFEG::DomainData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="DelDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new DelQ4g(this));
    _eleResNames = { "sigmaXX", "sigmaYY", "sigmaXY", "volume" };
    _nodeExtrapNames = { "sigmaXX", "sigmaYY", "sigmaXY" };
    _bVonMises = true;
    _resForm = "Vector OnNodes";
    _coefNames[0] = { "T" }; // 需要从 Heat 场取温度
}

DelDispFieldData::~DelDispFieldData() {

}
