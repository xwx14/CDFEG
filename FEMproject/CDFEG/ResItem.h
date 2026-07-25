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
#ifndef RES_ITEM_H
#define RES_ITEM_H
#include "CDFEG.h"
#include <string>
#include <vector>
namespace CDFEG {

    enum class ResLocation {
        OnNodes,
        OnGaussPoints
    };

    enum class ResType {
        Scalar,
        Vector,
        Matrix
    };

    CDFEG_API std::string resLocationToStr(ResLocation loc);
    CDFEG_API ResLocation strToResLocation(const std::string& str);
    CDFEG_API std::string resTypeToStr(ResType type);
    CDFEG_API ResType strToResType(const std::string& str);

    class CDFEG_API ResItem
    {
    public:
        ResItem(const std::string& name, ResType type, ResLocation loc = ResLocation::OnNodes);
        ~ResItem();
        void addVal(int iField, const std::string& valName);

        std::string _name;
        ResType _type;
        // 结果位置：OnNodes=节点结果(取_nodeRes)，OnGaussPoints=单元结果(取_elemRes)
        ResLocation _loc = ResLocation::OnNodes;
        std::vector<int> _iFields;
        std::vector<std::string> _ValNames;
    };
}
#endif
