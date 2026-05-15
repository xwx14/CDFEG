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

#include "Truss3DData.h"
#include "Truss3DDispFieldData.h"

Truss3DData::Truss3DData() {
    _dim = 3;
    _phyDatas.push_back(new Truss3DDispFieldData(this));
}

Truss3DData::~Truss3DData() {

}

int Truss3DData::caculate() {
    Truss3DDispFieldData* f = static_cast<Truss3DDispFieldData*>(_phyDatas[0]);
    f->initMatrix();
    f->eProgram_el();
    f->solve();
    f->uPhy();
    f->_equSys.calRightVals();
    return 1;
}

int Truss3DData::main() {
    caculate();
    return 1;
}
