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
#include "DomainData.h"
#include "PhyFieldData.h"
#include "Processor.h"
CDFEG::DomainData::DomainData() {
	_elePt.push_back(0);
}
CDFEG::DomainData::~DomainData() {
	for (PhyFieldData* p : _phyDatas)
	{
		delete p;
	}
}

void CDFEG::DomainData::post(int it) {
	for (Processor* p : _processors) {
		p->post(it);
	}
}
void CDFEG::DomainData::setNPts(int n)
{
	_nPts = n;
	for (PhyFieldData* p : _phyDatas)
	{
		p->setNPts(n);
	}
}

void CDFEG::DomainData::addNode(int id, double x, double y/*=0.0*/, double z/*=0.0*/)
{
	int n = _nodeIdMap.size();
	_nodeIdMap[id] = n;
	_nodes.push_back(x);
	if (_dim == 1)return;
	_nodes.push_back(y);
	if (_dim == 2)return;
	_nodes.push_back(z);
}

void CDFEG::DomainData::addNodeEnd()
{
	int n = _nodeIdMap.size();
	setNPts(n);
}

int CDFEG::DomainData::addEle(int id, const std::vector<int>& nodeIds, const std::string& eleType)
{
	int i = _elePt.size() - 1;
	auto it = _eleIdMap.find(id);
	if (it == _eleIdMap.end()) {
		// 新 id：作为体单元添加
		_eleIdMap[id] = i;
		_nElem = _eleIdMap.size();
		for (int nid : nodeIds)
		{
			_eleNodes.push_back(_nodeIdMap[nid]);
		}
		_elePt.push_back(_eleNodes.size());
	}
	else
	{
		// id 已存在（同 id 的体单元已登记）：判断本单元是否为该体单元的「边」
		int ownerIdx = it->second;
		int ownerNodeCnt = _elePt[ownerIdx + 1] - _elePt[ownerIdx];
		if ((int)nodeIds.size() < ownerNodeCnt)
		{
			// 节点更少 → 作为边单元（边 id 即所属体单元 id），追加节点但不计入 _nElem
			_edgeIdMap[i] = ownerIdx;
			for (int nid : nodeIds)
			{
				_eleNodes.push_back(_nodeIdMap[nid]);
			}
			_elePt.push_back(_eleNodes.size());
		}
		// else 节点数 >= 已有体单元：忽略，保持原体单元不变
	}

	VTKCellType iEleType = VTKCellType::VTK_EMPTY_CELL;
	if (eleType != "")
	{
		for (PhyFieldData* p : _phyDatas)
		{
			for (ElementBase* pEle : p->_eleSubs)
			{
				if (nodeIds.size() == pEle->getnNodesPerEle() && pEle->_types.find(eleType) != pEle->_types.end())
				{
					pEle->_eleIds.push_back(i);
					iEleType = pEle->_vtkCellType;
					break;
				}
			}
		}
	}
	_eleTypes.push_back(iEleType);
	return i;
}

void CDFEG::DomainData::addEdge(int id, const std::vector<int>& nodeIds, const std::string& eleType)
{
	int i = _elePt.size() - 1;
	_edgeIdMap[i]=_eleIdMap[id];
	_nElem = _eleIdMap.size();
	for (int id : nodeIds)
	{
		_eleNodes.push_back(_nodeIdMap[id]);
	}
	_elePt.push_back(_eleNodes.size());
	VTKCellType iEleType = VTKCellType::VTK_EMPTY_CELL;
	if (eleType != "")
	{
		for (PhyFieldData* p : _phyDatas)
		{
			for (ElementBase* pEle : p->_eleSubs)
			{
				if (nodeIds.size()==pEle->getnNodesPerEle()&&pEle->_types.find(eleType) != pEle->_types.end())
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

void CDFEG::DomainData::setEleMateId(int eleId, int id)
{
	int interId = _eleIdMap[eleId];
	int nPt = _elePt.size() - 1;
	if (_eleMateIds.size() < nPt)_eleMateIds.resize(nPt);
	_eleMateIds[interId] = id;
}

void CDFEG::DomainData::setEleMateByName(int eleId, const std::string& name)
{
	// 在_mateNames中查找材料名称对应的序号
	int mateId = -1;
	for (size_t i = 0; i < _mateNames.size(); i++)
	{
		if (_mateNames[i] == name)
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

void CDFEG::DomainData::setEleMateByInternal(int internalId, const std::string& name)
{
	// 在 _mateNames 中查找材料名称对应的序号
	int mateId = -1;
	for (size_t i = 0; i < _mateNames.size(); i++)
	{
		if (_mateNames[i] == name)
		{
			mateId = static_cast<int>(i);
			break;
		}
	}
	if (mateId < 0) return;
	// 直接按程序内部单元索引设置材料号（不经 _eleIdMap，避免边/体单元 id 冲突）
	int nPt = _elePt.size() - 1;
	if ((int)_eleMateIds.size() < nPt) _eleMateIds.resize(nPt, 0);
	_eleMateIds[internalId] = mateId;
}

int CDFEG::DomainData::addMate(const std::map<std::string, double>& matParam, const std::string& name)
{
	_mateParams.push_back(matParam);
	_mateNames.push_back(name);
	return _mateParams.size() - 1;
}

void CDFEG::DomainData::addMateType(const std::string& name, const std::vector<std::string>& params)
{
	_mateConstitutive[name] = params;
}



const std::map<std::string, double>& CDFEG::DomainData::getElemMatParams(int eleID, ElementBase* ele) const
{
	// 为适应旧版本数据而设
	if (ele->_eleMatIDMap.find(eleID) != ele->_eleMatIDMap.end()) {
		int iMat = ele->_eleMatIDMap[eleID];
		return _mateParams[iMat];
	}
	return _mateParams[_eleMateIds[eleID]];
}

// 取某组某参数的值；组或参数不存在、值未读到时返回 0.0
double CDFEG::DomainData::getParam(const std::string& group, const std::string& param) const {
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

std::map<std::string, std::vector<double>> CDFEG::DomainData::getCoef(const std::vector<int>& nodeIds, const std::map<int, std::vector<std::string>>& datas)
{
	std::map<std::string, std::vector<double>> coef;
	for (const auto& d : datas) 
	{
		PhyFieldData* fd= _phyDatas[d.first];
		for (const std::string& name : d.second) 
		{
            if (fd->_nodeRes.find(name) == fd->_nodeRes.end())continue;
			std::vector<double> v;
			const std::vector<double>& kv = fd->_nodeRes[name];
			v.reserve(nodeIds.size());
			for (int nid : nodeIds)
				v.push_back((nid >= 0 && nid < (int)kv.size()) ? kv[nid] : 0.0);
			coef[fd->_name + "::" + name] = std::move(v);
		}
	}
	return coef;
}
