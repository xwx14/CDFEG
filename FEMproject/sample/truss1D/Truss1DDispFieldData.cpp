// SPDX-License-Identifier: GPL-3.0
// This file is part of CDFEG.
//
// CDFEG is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDFEG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.

#include "Truss1DDispFieldData.h"
#include "Truss1DData.h"
#include "Truss1D.h"

Truss1DDispFieldData::Truss1DDispFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(1, femData) {
    _name="Truss1DDisp";
    _dispNames = { "u"};
    _dof2 = 0;
    _eleSubs.push_back(new Truss1D(this));
    _eleResNames = { "T","sigma" };
}

Truss1DDispFieldData::~Truss1DDispFieldData() {

}