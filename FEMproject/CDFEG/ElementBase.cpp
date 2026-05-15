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

#include "ElementBase.h"
#include "PhyFieldData.h"
#include "FemData.h"

CDFEG::ElementBase::ElementBase(int nNode, PhyFieldData* pData)
{
	_nNode = nNode;
	_phyData = pData;
	_femData = _phyData->_femData;
}

CDFEG::EleSubResult& CDFEG::ElementBase::run(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams)
{
	return _result;
}

CDFEG::uResult CDFEG::ElementBase::uEle(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams)
{
	uResult res;
	return res;
}
