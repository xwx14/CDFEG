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
#include "DomainData.h"
#include "PhyFieldData.h"
#include "ElementBase.h"
#include <set>
namespace CDFEG {

	// 按 ResItem 分量配对收集结果列指针：fromElem=true 取各场 _elemRes（单元结果），否则取 _nodeRes（节点结果）。
	// 单元结果项分量通常同场（iFields 全等），逐分量按 iFields[iv] 取，与节点分支一致。
	static std::vector<std::vector<double>*> collectResValPtrs(const DomainData* fem, const ResItem& item, bool fromElem)
	{
		std::vector<std::vector<double>*> ptrs;
		ptrs.reserve(item._ValNames.size());
		for (size_t iv = 0; iv < item._ValNames.size(); ++iv)
		{
			PhyFieldData* phy = fem->_phyDatas[item._iFields[iv]];
			ptrs.push_back(fromElem ? &phy->_elemRes[item._ValNames[iv]] : &phy->_nodeRes[item._ValNames[iv]]);
		}
		return ptrs;
	}

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
	GidPrePost::GidPrePost(DomainData* data) :Processor(data)
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
		collectPreParamDecls();
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
				else if (_preParamDecls.count(nameLower)) {
					_datReader.readNextLine();
					readPreParams(nameLower, line);
				}
			}
		}
		// 额外参数声明表用完即清：裸指针仅在 pre() 内有效，避免常驻悬垂指针
		_preParamDecls.clear();
		return 0;
	}


	int GidPrePost::readMate(const std::map<std::string, std::string>& params)
	{
		std::string name = params.at("name");
		auto itType = _femData->_mateConstitutive.find(name);
		if (itType == _femData->_mateConstitutive.end()) return -1;
		const std::vector<std::string>& mateparams = itType->second;
		const std::string& line = _datReader.getCurrentLine();
		int i = 0;
		while (_datReader.readNextLine()) {
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<double> vals = TextReader::splitDoubles(line, " ,");
			int len1 = vals.size();
			if (len1 > (int)mateparams.size())len1 = mateparams.size();
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

	void GidPrePost::collectPreParamDecls() {
		static const std::set<std::string> reserved = { "time","basedata","coord","id","ubf","elem" };
		auto addDecl = [&](auto* owner) {
			for (auto& g : owner->_addParams) {
				if (g.size() < 2) continue;
				const std::string& groupName = g[0];
				std::string gl = TextReader::toLowerCase(groupName);
				if (reserved.count(gl)) {
					std::cerr << "[GidPrePost] 跳过保留名参数组: " << groupName << std::endl;
					continue;
				}
				if (_preParamDecls.count(gl))
					std::cerr << "[GidPrePost] 参数组名重复(后者覆盖): " << groupName << std::endl;
				_preParamDecls[gl] = { &(owner->_paramValues), g };
			}
		};
		addDecl(_femData);
		for (PhyFieldData* f : _femData->_phyDatas) {
			addDecl(f);
			for (ElementBase* e : f->_eleSubs) addDecl(e);
		}
	}

	int GidPrePost::readPreParams(const std::string& group, const std::string& line) {
		auto it = _preParamDecls.find(group);
		if (it == _preParamDecls.end()) return -1;
		std::vector<double> vals = TextReader::splitDoubles(line, " ,");
		int nParams = (int)it->second.second.size() - 1;   // 参数名数 = 组行长度 - 1(组名)
		int n = (int)vals.size();
		if (n > nParams) n = nParams;
		std::vector<double> trimmed(vals.begin(), vals.begin() + n);
		const std::string& origName = it->second.second[0];   // 用原始组名回填，与 getParam 查询一致
		(*it->second.first)[origName] = trimmed;
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
			if (line.empty()) continue;                 // 容错：跳过空行
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<double> vals = TextReader::splitDoubles(line, " ,");
			if (vals.empty()) continue;                 // 容错：跳过空行/无效行
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
			if (line.empty()) continue;                 // 容错：跳过空行（避免 line[0] 越界）
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<int> vals = TextReader::splitInts(line, " ,");
			if (vals.size() < 2) continue;              // 容错：跳过空行/无效行（至少需 id + mateId）
			id = vals[0];
			vals.erase(vals.begin());
			mateId = vals.back();
			vals.pop_back();
			int internalId = _femData->addEle(id, vals, name);
			_femData->setEleMateByInternal(internalId, name+"_" + std::to_string(mateId));
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
			if (line.empty()) continue;                 // 容错：跳过空行
			if (line[0] == '*') {
				_datReader.preLine();
				break;
			}
			std::vector<double> vals = TextReader::splitDoubles(line, " ,");
			if ((int)vals.size() < dof + 1) continue;   // 容错：跳过空行/无效行（至少需节点 + dof 个边界值）
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
				auto matIt = eleSub->_eleMatIDMap.find(id);
				iMat = ((matIt != eleSub->_eleMatIDMap.end()) ? matIt->second : 0) - iMatStart + 1;
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
		const std::string& analysis = _femData->_prePostConfig._analysisName;

		// —— 单元结果（OnElements）：遍历 _eleResItems ——
		for (ResItem& item : _femData->_prePostConfig._eleResItems)
		{
			if (item._iFields.empty()) continue;
			PhyFieldData* phy = _femData->_phyDatas[item._iFields[0]];
			std::string typeStr = resTypeToStr(item._type);
			auto valPtrs = collectResValPtrs(_femData, item, true);
			for (ElementBase* eleSub : phy->_eleSubs)
			{
				std::string gpName = "GP_" + eleSub->_name;
				std::string gidType = vtkCellTypeToGidElemType(eleSub->_vtkCellType);
				if (it == 0)
				{
					outFile << "GaussPoints \"" << gpName << "\" ElemType " << gidType << std::endl;
					outFile << "Number Of Gauss Points: 1" << std::endl;
					outFile << "Natural Coordinates: Internal" << std::endl;
					outFile << "End GaussPoints" << std::endl;
				}
				outFile << "Result \"" << item._name << "\" \"" << analysis << "\"  ";
				outFile << std::setw(10) << it + 1 << " ";
				outFile << typeStr << " OnGaussPoints \"" << gpName << "\"" << std::endl;
				outFile << "ComponentNames ";
				for (const std::string& vn : item._ValNames)
					outFile << "\"" << vn << "\" ";
				outFile << std::endl;
				outFile << "Values" << std::endl;
				for (int id : eleSub->_eleIds)
				{
					outFile << std::setw(10) << id + 1;
					outFile << std::setw(16) << std::scientific << std::setprecision(7);
					for (auto* pv : valPtrs)
					{
						if (id >= 0 && id < (int)pv->size()) outFile << " " << (*pv)[id];
						else outFile << " " << 0.0;
					}
					outFile << std::endl;
				}
				outFile << "End Values" << std::endl;
			}
		}

		// —— 节点结果（OnNodes）：遍历 _nodeResItems ——
		for (ResItem& item : _femData->_prePostConfig._nodeResItems)
		{
			outFile << "Result \"" << item._name << "\" \"" << analysis << "\"  ";
			outFile << std::setw(10) << it + 1 << " ";
			outFile << resTypeToStr(item._type) << " OnNodes" << std::endl;
			outFile << "ComponentNames ";
			for (const std::string& vn : item._ValNames)
			{
				outFile << "\"" << vn << "\" ";
			}
			outFile << std::endl;
			outFile << "Values" << std::endl;
			auto valPtrs = collectResValPtrs(_femData, item, false);
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