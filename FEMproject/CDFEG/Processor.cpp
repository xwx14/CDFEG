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
#include "Processor.h"
#include "PhyFieldData.h"
#include "FemData.h"
namespace CDFEG {
	Processor::Processor(FEMData* data, PhyFieldData* fieldData)
	{
		_femData = data;
		_phyFieldData = fieldData;
	}

	int Processor::pre()
	{
		return -1;
	}

	int Processor::post(int it /*= 0*/)
	{
		return -1;
	}

	Processor::~Processor()
	{

	}
}