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

#include <iostream>
#include "ElDispFieldData.h"
#include "elData.h"
#include "CDFEG/gidProPost.h"

int main() {
    elData data;
	CDFEG::GidProPost pre(&data);
	pre.setFilePath("E:\\myProject\\CDFEG\\FEMproject\\test\\el2dData\\el.gid", "el");
	pre.pre();
    data.caculate();
    pre.post();
    return 0;
}
