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

// 取某组某参数的值；组或参数不存在、值未读到时返回 0.0
double CDFEG::ElementBase::getParam(const std::string& group, const std::string& param) const {
	for (const auto& g : _addParams) {
		if (g.size() < 2 || g[0] != group) continue;
		for (size_t i = 1; i < g.size(); ++i) {
			if (g[i] == param) {
				auto it = _paramValues.find(group);
				if (it == _paramValues.end() || (i - 1) >= it->second.size()) return 0.0;
				return it->second[i - 1];
			}
		}
		return 0.0;
	}
	return 0.0;
}
