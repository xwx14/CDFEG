#include "FemData.h"
#include "PhyFieldData.h"
SIFEG::FEMData::FEMData() {
	_elePt.push_back(0);
}
SIFEG::FEMData::~FEMData() {
	for (PhyFieldData* p : _phyDatas)
	{
		delete p;
	}
}
void SIFEG::FEMData::setNPts(int n)
{
	_nPts = n;
	for (PhyFieldData* p : _phyDatas)
	{
		p->setNPts(n);
	}
}

void SIFEG::FEMData::addNode(int id, double x, double y/*=0.0*/, double z/*=0.0*/)
{
	int n = _nodeIdMap.size();
	_nodeIdMap[id] = n;
	_nodes.push_back(x);
	if (_dim == 1)return;
	_nodes.push_back(y);
	if (_dim == 2)return;
	_nodes.push_back(z);
}

void SIFEG::FEMData::addNodeEnd()
{
	int n = _nodeIdMap.size();
	setNPts(n);
}

void SIFEG::FEMData::addEle(int id, const std::vector<int>& nodeIds, const std::string& eleType)
{

	int i = _elePt.size() - 1;
	_eleIdMap[id] = i;
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
			for (EleSubBase* pEle : p->_eleSubs)
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

void SIFEG::FEMData::setEleMateId(int eleId, int id)
{
	int interId = _eleIdMap[eleId];
	int nPt = _elePt.size() - 1;
	if (_eleMateIds.size() < nPt)_eleMateIds.resize(nPt);
	_eleMateIds[interId] = id;
}

void SIFEG::FEMData::setEleMateByName(int eleId, const std::string& name)
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

int SIFEG::FEMData::addMate(const std::map<std::string, double>& matParam, const std::string& name)
{
	_mateParams.push_back(matParam);
	_mateNames.push_back(name);
	return _mateParams.size() - 1;
}



const std::map<std::string, double>& SIFEG::FEMData::getElemMatParams(int eleID, EleSubBase* ele) const
{
	// 为适应旧版本数据而设
	if (ele->_eleMatIDMap.find(eleID) != ele->_eleMatIDMap.find(eleID)) {
		int iMat = ele->_eleMatIDMap[eleID];
		return _mateParams[iMat];
	}
	return _mateParams[_eleMateIds[eleID]];
}
