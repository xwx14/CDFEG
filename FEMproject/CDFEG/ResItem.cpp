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
#include "ResItem.h"
#include <cctype>
namespace CDFEG {
    std::string resLocationToStr(ResLocation loc)
    {
        switch (loc)
        {
            case ResLocation::OnNodes: return "OnNodes";
            case ResLocation::OnElements: return "OnGaussPoints";
        }
        return "OnNodes";
    }

    ResLocation strToResLocation(const std::string& str)
    {
        std::string lower; lower.reserve(str.size());
        for (char c : str) lower.push_back((char)std::tolower((unsigned char)c));
        if (lower == "ongausspoints") return ResLocation::OnElements;
        return ResLocation::OnNodes;
    }

    std::string resTypeToStr(ResType type)
    {
        switch (type)
        {
            case ResType::Scalar: return "Scalar";
            case ResType::Vector: return "Vector";
            case ResType::Matrix: return "Matrix";
        }
        return "Scalar";
    }

    ResType strToResType(const std::string& str)
    {
        std::string lower; lower.reserve(str.size());
        for (char c : str) lower.push_back((char)std::tolower((unsigned char)c));
        if (lower == "vector") return ResType::Vector;
        if (lower == "matrix") return ResType::Matrix;
        return ResType::Scalar;
    }

    ResItem::ResItem(const std::string& name, ResType type, ResLocation loc)
        : _name(name), _type(type), _loc(loc)
    {
    }

    ResItem::~ResItem()
    {
    }

    void ResItem::addVal(int iField, const std::string& valName)
    {
        _iFields.push_back(iField);
        _ValNames.push_back(valName);
    }
}
