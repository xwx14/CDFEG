#include "gidProPost.h"
#include <iostream>
#include <sstream>
#include <map>
#include <iomanip>
#include "FemData.h"
#include "PhyFieldData.h"
#include "EleSubBase.h"
namespace SIFEG {
	GidProPost::GidProPost(FEMData* data) :Processor(data,nullptr)
	{

	}
	GidProPost::~GidProPost()
	{
	}



	void GidProPost::setFilePath(const std::string& parentPath, const std::string& name)
	{
		std::string path= parentPath + "\\" + name + ".gid\\";
		_datFn =path +name + ".dat";
		_gidMshFn = path + name + ".post.msh";
		_gidResFn = path + name + ".post.res";
	}

	int GidProPost::pre()
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


	int GidProPost::readMate(const std::map<std::string, std::string>& params)
	{
		std::string name = params.at("name");
		EleSubBase* curEle = nullptr;
		for (PhyFieldData* f : _femData->_phyDatas)
		{
			for (EleSubBase* e : f->_eleSubs)
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

	int GidProPost::readTime(const std::string& line)
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

	int GidProPost::readBaseData(const std::string& line)
	{
		std::vector<int> vals = TextReader::splitInts(line, " ,");
		_nPts = vals[0];
		_nEles = vals[1];
		return 0;
	}

	int GidProPost::readCoord(const std::map<std::string, std::string>& params)
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

	int GidProPost::readElement(const std::map<std::string, std::string>& params)
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

	int GidProPost::readID(const std::map<std::string, std::string>& params)
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

	int GidProPost::readUBF(const std::map<std::string, std::string>& params)
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

	int GidProPost::gidMsh()
	{
		// 将网格数据写入"./heat.gid/heat.post.msh"
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
		for (EleSubBase* eleSub : _mshOutEle)
		{
			iMatStart = _matStartID2[eleSub];
			outFile << "Mesh \"" << eleSub->_name << "\" Dimension " << dim << " Elemtype Quadrilateral Nnode  " << eleSub->_nNode << std::endl;
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

	int GidProPost::writeNodes(std::ofstream& outFile, int dim)
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

	int GidProPost::post(int it)
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
			outFile << std::setw(10) << it << " ";
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




}