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

#include "Truss2DData.h"
#include "Truss2DDispFieldData.h"

Truss2DData::Truss2DData() {
    _dim = 2;
    _phyDatas.push_back(new Truss2DDispFieldData(this));
}

Truss2DData::~Truss2DData() {

}

int Truss2DData::caculate() {
    Truss2DDispFieldData* f = static_cast<Truss2DDispFieldData*>(_phyDatas[0]);
    f->initMatrix();
    f->eProgram();
    f->solve();
    f->uPhy();
    f->_equSys.calRightVals();
    return 1;
}

int Truss2DData::main() {
    caculate();
    return 1;
}