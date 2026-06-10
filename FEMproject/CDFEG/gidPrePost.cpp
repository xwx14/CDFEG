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
#include <sstream>
#include <map>
#include <iomanip>
#include "gidPrePost.h"
#include "FemData.h"
#include "PhyFieldData.h"
#include "ElementBase.h"
namespace CDFEG {

	static std::string vtkCellTypeToGidElemType(VTKCellType type)
	{
		switch (type)
		{
		case VTK_VERTEX:
			return "Point";
		case VTK_LINE:
		case VTK_POLY_LINE:
		case VTK_QUADRATIC_EDGE:
		case VTK_CUBIC_LINE:
		case VTK_HIGHER_ORDER_EDGE:
		case VTK_PARAMETRIC_CURVE:
		case VTK_LAGRANGE_CURVE:
		case VTK_BEZIER_CURVE:
			return "Line";
		case VTK_TRIANGLE:
		case VTK_TRIANGLE_STRIP:
		case VTK_QUADRATIC_TRIANGLE:
		case VTK_BIQUADRATIC_TRIANGLE:
		case VTK_HIGHER_ORDER_TRIANGLE:
		case VTK_PARAMETRIC_TRI_SURFACE:
		case VTK_LAGRANGE_TRIANGLE:
		case VTK_BEZIER_TRIANGLE:
			return "Triangle";
		case VTK_QUAD:
		case VTK_PIXEL:
		case VTK_QUADRATIC_QUAD:
		case VTK_BIQUADRATIC_QUAD:
		case VTK_QUADRATIC_LINEAR_QUAD:
		case VTK_QUADRATIC_POLYGON:
		case VTK_HIGHER_ORDER_QUAD:
		case VTK_PARAMETRIC_QUAD_SURFACE:
		case VTK_LAGRANGE_QUADRILATERAL:
		case VTK_BEZIER_QUADRILATERAL:
			return "Quadrilateral";
		case VTK_TETRA:
		case VTK_QUADRATIC_TETRA:
		case VTK_HIGHER_ORDER_TETRAHEDRON:
		case VTK_PARAMETRIC_TETRA_REGION:
		case VTK_LAGRANGE_TETRAHEDRON:
		case VTK_BEZIER_TETRAHEDRON:
			return "Tetrahedra";
		case VTK_HEXAHEDRON:
		case VTK_VOXEL:
		case VTK_QUADRATIC_HEXAHEDRON:
		case VTK_TRIQUADRATIC_HEXAHEDRON:
		case VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON:
		case VTK_HIGHER_ORDER_HEXAHEDRON:
		case VTK_PARAMETRIC_HEX_REGION:
		case VTK_LAGRANGE_HEXAHEDRON:
		case VTK_BEZIER_HEXAHEDRON:
			return "Hexahedra";
		case VTK_WEDGE:
		case VTK_QUADRATIC_WEDGE:
		case VTK_QUADRATIC_LINEAR_WEDGE:
		case VTK_BIQUADRATIC_QUADRATIC_WEDGE:
		case VTK_HIGHER_ORDER_WEDGE:
		case VTK_LAGRANGE_WEDGE:
		case VTK_BEZIER_WEDGE:
		case VTK_PENTAGONAL_PRISM:
		case VTK_HEXAGONAL_PRISM:
			return "Prism";
		case VTK_PYRAMID:
		case VTK_QUADRATIC_PYRAMID:
		case VTK_TRIQUADRATIC_PYRAMID:
		case VTK_HIGHER_ORDER_PYRAMID:
		case VTK_LAGRANGE_PYRAMID:
		case VTK_BEZIER_PYRAMID:
			return "Pyramid";
		case VTK_POLYGON:
		case VTK_HIGHER_ORDER_POLYGON:
			return "Polygon";
		default:
			return "Triangle";
		}
	}
	GidPrePost::GidPrePost(FEMData* data) :Processor(data,nullptr)
	{
		for (PhyFieldData* f : _femData->_phyDatas)
		{
			for (ElementBase* e : f->_eleSubs)
			{
				if (e->_bOutMsh)_mshOutEle.push_back(e);
			}
		}
	}
	GidPrePost::~GidPrePost()
	{
	}



	void GidPrePost::setFilePath(const std::string& parentPath, const std::string& name)
	{
		//std::string path= parentPath + "\\" + name + ".gid\\";
		std::string path = parentPath+"\\";
		_datFn =path +name + ".dat";
		_gidMshFn = path + name + ".post.msh";
		_gidResFn = path + name + ".post.res";
	}

	int GidPrePost::pre()
	{
		_datReader.setFilePath(_datFn);
		if (!_datReader.open()) return -1;
		const std::string& line = _datReader.getCurrentLine();
		while (_datReader.readNextLine()) {
			if (line[0] == '*') {
				std::string structure = line.substr(1);
				std::map<std::string, std::string> params=TextReader::parseInfoLine(structure,true);
				std::string nameLower = TextReader::toLowerCase(params["name"]);
				std::string typeLower = TextReader::toLowerCase(params["type"]);
				if (typeLower=="mat") {
					readMate(params);
				}
				else if(nameLower=="time") {
					_datReader.readNextLine();
					readTime(line);
				}
				else if(nameLower=="basedata") {
					_datReader.readNextLine();
					readBaseData(line);
				}
				else if (nameLower == "coord") {
					readCoord(params);
				}
				else if (typeLower == "id") {
					readID(params);
				}
				else if(typeLower=="ubf"){
					readUBF(params);
				}
				else if (typeLower == "elem") {
					readElement(params);
				}
			}
		}
		return 0;
	}


	int GidPrePost::readMate(const std::map<std::string, std::string>& params)
	{
		std::string name = params.at("name");
		ElementBase* curEle = nullptr;
		for (PhyFieldData* f : _femData->_phyDatas)
		{
			for (ElementBase* e : f->_eleSubs)
			{
				if ("mat_" + e->_name == name)curEle = e;
			}
		}
		if (!curEle)return -1;
		std::vector<std::string>& mateparams = curEle->_paramNames;
		const std::string& line = _datReader.getCurrentLine();
		int i = 0;
		while (_datReader.readNextLine()) {
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<double> vals = TextReader::splitDoubles(line, " ,");
			int len1 = vals.size();
			if (len1 > mateparams.size())len1 = mateparams.size();
			std::map<std::string, double> paramMap;
			for (int j = 0; j < len1; ++j) {
				paramMap[mateparams[j]] = vals[j];
			}
			_femData->addMate(paramMap, name+"_" + std::to_string(++i));
		}
		return 0;
	}

	int GidPrePost::readTime(const std::string& line)
	{
		std::vector<double> vals = TextReader::splitDoubles(line, " ,");
		switch (vals.size())
		{
		case 2:
			_femData->_dt = vals[0];
			_femData->_tMax = vals[1];
			break;
		case 3:
			_femData->_dt = vals[0];
			_femData->_tMax = vals[2]-vals[1];
			break;
		default:
			break;
		}
		return 0;
	}

	int GidPrePost::readBaseData(const std::string& line)
	{
		std::vector<int> vals = TextReader::splitInts(line, " ,");
		_nPts = vals[0];
		_nEles = vals[1];
		return 0;
	}

	int GidPrePost::readCoord(const std::map<std::string, std::string>& params)
	{
		int id = -1;
		const std::string& line = _datReader.getCurrentLine();
		while (_datReader.readNextLine()) {
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<double> vals = TextReader::splitDoubles(line, " ,");
			id = vals[0]+0.1;
			switch (vals.size())
			{
			case 2:
				_femData->addNode(id, vals[1]);
				break;
			case 3:
				_femData->addNode(id, vals[1], vals[2]);
				break;
			case 4:
				_femData->addNode(id, vals[1], vals[2], vals[3]);
				break;
			default:
				break;
			}
			
		}
		_femData->addNodeEnd();
		return 0;
	}

	int GidPrePost::readElement(const std::map<std::string, std::string>& params)
	{
		int id = -1;
		int mateId = -1;
		std::string name = params.at("name");
		const std::string& line = _datReader.getCurrentLine();
		while (_datReader.readNextLine()) {
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<int> vals = TextReader::splitInts(line, " ,");
			id = vals[0];
			vals.erase(vals.begin());
			mateId = vals[vals.size() - 1];
			vals.pop_back();
			_femData->addEle(id, vals, name);
			_femData->setEleMateByName(id,name+"_" + std::to_string(mateId));
		}
		return 0;
	}

	int GidPrePost::readID(const std::map<std::string, std::string>& params)
	{
		int nodeIdIn = -1;
		std::string FieldName = params.at("name").substr(2);
		PhyFieldData* curField=nullptr;
		for (PhyFieldData* f : _femData->_phyDatas)
		{
			if (FieldName == f->_name)curField = f;
		}
		if (curField == nullptr)return -1;
		int dof = curField->_dof;
		const std::string& line = _datReader.getCurrentLine();
		while (_datReader.readNextLine()) {
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<int> vals = TextReader::splitInts(line, " ,");
			
			
		}
		return 0;
	}

	int GidPrePost::readUBF(const std::map<std::string, std::string>& params)
	{
		std::string FieldName = params.at("name").substr(3);
		PhyFieldData* curField = nullptr;
		for (PhyFieldData* f : _femData->_phyDatas)
		{
			if (FieldName == f->_name)curField = f;
		}
		if (curField == nullptr)return -1;
		int dof = curField->_dof;
		const std::string& line = _datReader.getCurrentLine();
		while (_datReader.readNextLine()) {
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<double> vals = TextReader::splitDoubles(line, " ,");
			for (int i = 0; i < dof; ++i)
			{
				curField->setFirstBoundry(vals[0]+0.1, vals[i+1], i);
			}
		}
		return 0;
	}

	int GidPrePost::gidMsh()
	{
		// 将网格数据写入
		std::ofstream outFile;
		outFile.open(_gidMshFn, std::ios::out); // 使用ios::out模式打开文件
		int dim = _femData->_dim;
		if (!outFile.is_open())return 0;
		bool bFirst = true;
		int nEle;
		int iNode0 = -1;
		int iEle = 0;
		// 每个单元初始的节点号在_eleNodes的位置，最后一个值为_eleNodes的长度，此序列长度为单元数+1
		std::vector<int>& elePt = _femData->_elePt;
		int iMat;
		int iMatStart;
		for (ElementBase* eleSub : _mshOutEle)
		{
			iMatStart = _matStartID2[eleSub];
			outFile << "Mesh \"" << eleSub->_name << "\" Dimension " << dim << " Elemtype " << vtkCellTypeToGidElemType(eleSub->_vtkCellType) << " Nnode  " << eleSub->_nNode << std::endl;
			outFile << "Coordinates" << std::endl;
			if (bFirst)
			{
				bFirst = false;
				writeNodes(outFile, dim);
			}
			outFile << "End coordinates" << std::endl;
			std::vector<int>& eleIds = eleSub->_eleIds;
			outFile << "Elements" << std::endl;
			nEle = eleIds.size();
			for (int id : eleIds)
			{
				outFile << std::setw(6) << id + 1;
				for (int iNode1 = elePt[id]; iNode1 < elePt[id + 1]; ++iNode1) {
					outFile << std::setw(6) << _femData->_eleNodes[iNode1] + 1;
				}
				iMat = eleSub->_eleMatIDMap[id] - iMatStart + 1;
				outFile << "  " << iMat << std::endl;
			}
			outFile << "End elements" << std::endl;
		}
		outFile.close();
		return 1;
	}

	int GidPrePost::writeNodes(std::ofstream& outFile, int dim)
	{
		int nNode = _femData->_nodeIdMap.size();
		int iVal = -1;
		for (int i = 0; i < nNode; ++i) {

			outFile << std::setw(6) << i + 1;
			outFile << std::setw(16) << std::scientific << std::setprecision(7);
			for (int j = 0; j < dim; ++j)
			{
				//outFile << std::setw(16) << std::scientific << std::setprecision(7) << _femData->_nodes[i * dim + j];
				outFile << _femData->_nodes[++iVal] << "  ";
			}
			outFile << std::endl;
		}
		return 1;
	}

	int GidPrePost::post(int it)
	{
		std::ofstream outFile;
		gidMsh();
		// 使用ios::out模式打开文件
		if (it == 0)
		{
			outFile.open(_gidResFn, std::ios::out);
			if (!outFile.is_open())return 0;
			outFile << "GID Post Results File 1.0" << std::endl;
		}
		else
		{
			outFile.open(_gidResFn, std::ios::app);
			if (!outFile.is_open())return 0;
		}
		int nPhy = _femData->_phyDatas.size();
		int nNodes = _femData->_nPts;
		int dof;
		for (int iPhy = 0; iPhy < nPhy; ++iPhy)
		{
			PhyFieldData* phyData = _femData->_phyDatas[iPhy];
			if (phyData->_nodeRes.size() == 0)continue;
			dof = phyData->_dof;
			outFile << "Result \""<<phyData->_name<<"\" \"Load Analysis\"  ";
			outFile << std::setw(10) << it+1 << " ";
			outFile << phyData->_resForm << std::endl;
			outFile << "ComponentNames ";
			for (const std::string& disp : phyData->_dispNames)
			{
				outFile << "\"" << disp << "\" ";
			}
			outFile << std::endl;
			outFile << "Values" << std::endl;
			int iValue = -1;
			for (int iNode = 0; iNode < nNodes; ++iNode)
			{
				outFile << std::setw(10) << iNode + 1;
				outFile << std::setw(16) << std::scientific << std::setprecision(7);
				for (const std::string& disp : phyData->_dispNames)
				{
					outFile <<" " << phyData->_nodeRes[disp][iNode];
				}
				for (int iDof = 0; iDof < dof; ++iDof)
				{
					//todo: 结果输出需修改为对应物理场的数据
					//outFile << std::setw(16) << std::scientific << std::setprecision(7) << phyData->_nodeRes[++iValue];
				}
				outFile << std::endl;
			}
			outFile << "End Values" << std::endl;
		}
		return 1;
	}

	int GidPrePost::post2(int it)
	{
		std::ofstream outFile;
		gidMsh();
		if (it == 0)
		{
			outFile.open(_gidResFn, std::ios::out);
			if (!outFile.is_open())return 0;
			outFile << "GID Post Results File 1.0" << std::endl;
		}
		else
		{
			outFile.open(_gidResFn, std::ios::app);
			if (!outFile.is_open())return 0;
		}
		int nNodes = _femData->_nPts;
		for (GidResItem& item : _resItems)
		{
			outFile << "Result \"" << item._name << "\" \"Load Analysis\"  ";
			outFile << std::setw(10) << it + 1 << " ";
			outFile << gidResultTypeToStr(item._type) << " OnNodes" << std::endl;
			outFile << "ComponentNames ";
			for (const std::string& vn : item._ValNames)
			{
				outFile << "\"" << vn << "\" ";
			}
			outFile << std::endl;
			outFile << "Values" << std::endl;
			std::vector<std::vector<double>*> valPtrs;
			valPtrs.reserve(item._iFields.size());
			for (size_t iv = 0; iv < item._iFields.size(); ++iv)
			{
				valPtrs.push_back(&(_femData->_phyDatas[item._iFields[iv]]->_nodeRes[item._ValNames[iv]]));
			}
			for (int iNode = 0; iNode < nNodes; ++iNode)
			{
				outFile << std::setw(10) << iNode + 1;
				outFile << std::setw(16) << std::scientific << std::setprecision(7);
				for (auto* pv : valPtrs)
				{
					outFile << " " << (*pv)[iNode];
				}
				outFile << std::endl;
			}
			outFile << "End Values" << std::endl;
		}
		return 1;
	}
}