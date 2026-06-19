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
#include "FemData.h"
#include "PhyFieldData.h"
CDFEG::FEMData::FEMData() {
	_elePt.push_back(0);
}
CDFEG::FEMData::~FEMData() {
	for (PhyFieldData* p : _phyDatas)
	{
		delete p;
	}
}
void CDFEG::FEMData::setNPts(int n)
{
	_nPts = n;
	for (PhyFieldData* p : _phyDatas)
	{
		p->setNPts(n);
	}
}

void CDFEG::FEMData::addNode(int id, double x, double y/*=0.0*/, double z/*=0.0*/)
{
	int n = _nodeIdMap.size();
	_nodeIdMap[id] = n;
	_nodes.push_back(x);
	if (_dim == 1)return;
	_nodes.push_back(y);
	if (_dim == 2)return;
	_nodes.push_back(z);
}

void CDFEG::FEMData::addNodeEnd()
{
	int n = _nodeIdMap.size();
	setNPts(n);
}

void CDFEG::FEMData::addEle(int id, const std::vector<int>& nodeIds, const std::string& eleType)
{
	int i = _elePt.size() - 1;
	if (_eleIdMap.find(id) == _eleIdMap.end()) {
		_eleIdMap[id] = i;
		_nElem = _eleIdMap.size();
		for (int id : nodeIds)
		{
			_eleNodes.push_back(_nodeIdMap[id]);
		}
		_elePt.push_back(_eleNodes.size());
	}
	//条件判断 _elePt[_eleIdMap[id] + 1] - _elePt[_eleIdMap[id]] 计算的是原单元的节点个数，
	//当 nodeIds.size() 与之不一致时才重新添加（递归调用）。如果节点数一致则作为edge边添加。
	else if (nodeIds.size() != _elePt[_eleIdMap[id] + 1] - _elePt[_eleIdMap[id]])
	{
		return addEle(id, nodeIds, eleType);
	}

	VTKCellType iEleType;
	if (eleType != "")
	{
		for (PhyFieldData* p : _phyDatas)
		{
			for (ElementBase* pEle : p->_eleSubs)
			{
				if (pEle->_types.find(eleType) != pEle->_types.end())
				{
					pEle->_eleIds.push_back(i);
					iEleType = pEle->_vtkCellType;
					break;
				}
			}
		}
	}
	_eleTypes.push_back(iEleType);
}

void CDFEG::FEMData::addEdge(int id, const std::vector<int>& nodeIds, const std::string& eleType)
{
	int i = _elePt.size() - 1;
	_edgeIdMap[i]=_eleIdMap[id];
	_nElem = _eleIdMap.size();
	for (int id : nodeIds)
	{
		_eleNodes.push_back(_nodeIdMap[id]);
	}
	_elePt.push_back(_eleNodes.size());
	VTKCellType iEleType;
	if (eleType != "")
	{
		for (PhyFieldData* p : _phyDatas)
		{
			for (ElementBase* pEle : p->_eleSubs)
			{
				if (pEle->_types.find(eleType) != pEle->_types.end())
				{
					pEle->_eleIds.push_back(i);
					iEleType = pEle->_vtkCellType;
					break;
				}
			}
		}
	}
	_eleTypes.push_back(iEleType);
}

void CDFEG::FEMData::setEleMateId(int eleId, int id)
{
	int interId = _eleIdMap[eleId];
	int nPt = _elePt.size() - 1;
	if (_eleMateIds.size() < nPt)_eleMateIds.resize(nPt);
	_eleMateIds[interId] = id;
}

void CDFEG::FEMData::setEleMateByName(int eleId, const std::string& name)
{
	// 在_mateNames中查找材料名称对应的序号
	int mateId = -1;
	for (size_t i = 0; i < _mateNames.size(); i++)
	{
		if (_mateNames[i] == "mat_"+name)
		{
			mateId = static_cast<int>(i);
			break;
		}
	}
	// 如果找到材料号，则设置单元的材料号
	if (mateId >= 0)
	{
		setEleMateId(eleId, mateId);
	}
}

int CDFEG::FEMData::addMate(const std::map<std::string, double>& matParam, const std::string& name)
{
	_mateParams.push_back(matParam);
	_mateNames.push_back(name);
	return _mateParams.size() - 1;
}



const std::map<std::string, double>& CDFEG::FEMData::getElemMatParams(int eleID, ElementBase* ele) const
{
	// 为适应旧版本数据而设
	if (ele->_eleMatIDMap.find(eleID) != ele->_eleMatIDMap.find(eleID)) {
		int iMat = ele->_eleMatIDMap[eleID];
		return _mateParams[iMat];
	}
	return _mateParams[_eleMateIds[eleID]];
}

// 取某组某参数的值；组或参数不存在、值未读到时返回 0.0
double CDFEG::FEMData::getParam(const std::string& group, const std::string& param) const {
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
