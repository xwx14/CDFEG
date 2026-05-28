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
#ifndef GID_RES_ITEM_H
#define GID_RES_ITEM_H
#include "CDFEG.h"
#include <string>
#include <vector>
namespace CDFEG {

	enum class GidLocation {
		OnNodes,
		OnGaussPoints
	};

	enum class GidResultType {
		Scalar,
		Vector2,
		Vector3,
		Vector4,
		Vector,
		Matrix3,
		Matrix6,
		Matrix,
		PlainDeformationMatrix,
		MainMatrix,
		LocalAxes,
		ComplexScalar,
		ComplexVector4,
		ComplexVector6,
		ComplexVector,
		ComplexMatrix3,
		ComplexMatrix6,
		ComplexMatrix
	};

	CDFEG_API std::string gidLocationToStr(GidLocation loc);
	CDFEG_API GidLocation strToGidLocation(const std::string& str);
	CDFEG_API std::string gidResultTypeToStr(GidResultType type);
	CDFEG_API GidResultType strToGidResultType(const std::string& str);
	CDFEG_API int gidResultTypeComponents(GidResultType type);

	class CDFEG_API GidResItem
	{
	public:
		GidResItem(const std::string& name, GidResultType type);
		~GidResItem();
		void addVal(int iField, const std::string& valName);

		std::string _name;
		GidResultType _type;
		std::vector<int> _iFields;
		std::vector<std::string> _ValNames;
	};
}
#endif
