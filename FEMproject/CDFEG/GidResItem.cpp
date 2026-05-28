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

#include "GidResItem.h"
namespace CDFEG {

	std::string gidLocationToStr(GidLocation loc)
	{
		switch (loc)
		{
		case GidLocation::OnNodes: return "OnNodes";
		case GidLocation::OnGaussPoints: return "OnGaussPoints";
		default: return "OnNodes";
		}
	}

	GidLocation strToGidLocation(const std::string& str)
	{
		std::string lower = str;
		for (auto& c : lower) c = tolower(c);
		if (lower == "ongausspoints") return GidLocation::OnGaussPoints;
		return GidLocation::OnNodes;
	}

	std::string gidResultTypeToStr(GidResultType type)
	{
		switch (type)
		{
		case GidResultType::Scalar: return "Scalar";
		case GidResultType::Vector2: return "Vector:2";
		case GidResultType::Vector3: return "Vector:3";
		case GidResultType::Vector4: return "Vector:4";
		case GidResultType::Vector: return "Vector";
		case GidResultType::Matrix3: return "Matrix:3";
		case GidResultType::Matrix6: return "Matrix:6";
		case GidResultType::Matrix: return "Matrix";
		case GidResultType::PlainDeformationMatrix: return "PlainDeformationMatrix";
		case GidResultType::MainMatrix: return "MainMatrix";
		case GidResultType::LocalAxes: return "LocalAxes";
		case GidResultType::ComplexScalar: return "ComplexScalar";
		case GidResultType::ComplexVector4: return "ComplexVector:4";
		case GidResultType::ComplexVector6: return "ComplexVector:6";
		case GidResultType::ComplexVector: return "ComplexVector";
		case GidResultType::ComplexMatrix3: return "ComplexMatrix:3";
		case GidResultType::ComplexMatrix6: return "ComplexMatrix:6";
		case GidResultType::ComplexMatrix: return "ComplexMatrix";
		default: return "Scalar";
		}
	}

	GidResultType strToGidResultType(const std::string& str)
	{
		std::string lower = str;
		for (auto& c : lower) c = tolower(c);
		if (lower == "vector:2") return GidResultType::Vector2;
		if (lower == "vector:3") return GidResultType::Vector3;
		if (lower == "vector:4") return GidResultType::Vector4;
		if (lower == "vector") return GidResultType::Vector;
		if (lower == "matrix:3") return GidResultType::Matrix3;
		if (lower == "matrix:6") return GidResultType::Matrix6;
		if (lower == "matrix") return GidResultType::Matrix;
		if (lower == "plaindeformationmatrix") return GidResultType::PlainDeformationMatrix;
		if (lower == "mainmatrix") return GidResultType::MainMatrix;
		if (lower == "localaxes") return GidResultType::LocalAxes;
		if (lower == "complexscalar") return GidResultType::ComplexScalar;
		if (lower == "complexvector:4") return GidResultType::ComplexVector4;
		if (lower == "complexvector:6") return GidResultType::ComplexVector6;
		if (lower == "complexvector") return GidResultType::ComplexVector;
		if (lower == "complexmatrix:3") return GidResultType::ComplexMatrix3;
		if (lower == "complexmatrix:6") return GidResultType::ComplexMatrix6;
		if (lower == "complexmatrix") return GidResultType::ComplexMatrix;
		return GidResultType::Scalar;
	}

	int gidResultTypeComponents(GidResultType type)
	{
		switch (type)
		{
		case GidResultType::Scalar: return 1;
		case GidResultType::Vector2: return 2;
		case GidResultType::Vector3: return 3;
		case GidResultType::Vector4: return 4;
		case GidResultType::Vector: return 3;
		case GidResultType::Matrix3: return 3;
		case GidResultType::Matrix6: return 6;
		case GidResultType::Matrix: return 6;
		case GidResultType::PlainDeformationMatrix: return 4;
		case GidResultType::MainMatrix: return 12;
		case GidResultType::LocalAxes: return 3;
		case GidResultType::ComplexScalar: return 2;
		case GidResultType::ComplexVector4: return 4;
		case GidResultType::ComplexVector6: return 6;
		case GidResultType::ComplexVector: return 6;
		case GidResultType::ComplexMatrix3: return 6;
		case GidResultType::ComplexMatrix6: return 12;
		case GidResultType::ComplexMatrix: return 12;
		default: return 1;
		}
	}

	GidResItem::GidResItem(const std::string& name, GidResultType type)
		: _name(name), _type(type)
	{
	}

	GidResItem::~GidResItem()
	{
	}

	void GidResItem::addVal(int iField, const std::string& valName)
	{
		_iFields.push_back(iField);
		_ValNames.push_back(valName);
	}
}
