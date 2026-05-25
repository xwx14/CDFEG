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

#include "elData.h"
#include "ElDispFieldData.h"

elData::elData() {
    _dim = 2;
    _phyDatas.push_back(new ElDispFieldData(this));
}

elData::~elData() {

}

int elData::caculate() {
    ElDispFieldData* aField = static_cast<ElDispFieldData*>(_phyDatas[0]);
    aField->initMatrix();
    aField->eProgram_el();
    aField->solve();
    aField->uPhy();
    aField->_equSys.calRightVals();
    return 1;
}
