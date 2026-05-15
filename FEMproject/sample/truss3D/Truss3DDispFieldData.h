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

#ifndef TRUSS3DDISP_FIELD_DATA_H
#define TRUSS3DDISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"

class Truss3DDispFieldData : public CDFEG::PhyFieldData {
public:
    Truss3DDispFieldData(CDFEG::FEMData* femData);
    ~Truss3DDispFieldData();
};

#endif
