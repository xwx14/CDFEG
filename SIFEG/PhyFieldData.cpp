#include "PhyFieldData.h"
#include "EleSub.h"
#include "FemData.h"
#include <set>
#include <iostream>
#include <map>
namespace SIFEG {

	PhyFieldData::PhyFieldData(int dof, FEMData* femData)
	{
		_dof = dof;
		_femData = femData;
	}

	PhyFieldData::~PhyFieldData()
	{
		for (EleSubBase* eleSub : _eleSubs)
		{
			if (eleSub)delete eleSub;
		}
	}

	int PhyFieldData::eProgram_el()
	{
		int dim = _femData->_dim;
		int kNode = _femData->_nPts;
		int nEleSub = _eleSubs.size();
		// 填充刚度矩阵
		for (int iEleSub = 0; iEleSub < nEleSub; ++iEleSub)
		{
			SIFEG::EleSubBase* eleSub = _eleSubs[iEleSub];
			int nNode = eleSub->getnNodesPerEle();
			int k = nNode * _dof;
			const std::vector<int>& eleIds = eleSub->getEleIds();
			for (int eleID : eleSub->_eleIds)
			{
				std::vector<double> r;
				std::vector<int> nodeIds;
				for (int i = _femData->_elePt[eleID]; i < _femData->_elePt[eleID + 1]; ++i)
				{
					int nodeId = _femData->_eleNodes[i];
					nodeIds.push_back(nodeId);
					// 节点坐标赋值
					int iCoor = dim * nodeId;
					for (int iDim = 0; iDim < dim; ++iDim)
					{
						r.push_back(_femData->_nodes[iCoor + iDim]);
					}
				}
				const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
				std::map<std::string, std::vector<double>> coef;
				//coef = getCoef(nodeIds);
				SIFEG::EleSubResult& outData = eleSub->run(r, coef, matParams);
				std::vector<double>& estifn = outData.estif;
				// ida和u中的序号
				int iDof1;
				// 方程号
				int inv;
				std::vector<int> lm;
				for (int nodi:nodeIds)
				{
					int iStart = nodi * _dof;
					for (int iDof = 0; iDof < _dof; ++iDof)
					{
						iDof1 = iDof + iStart;
						inv = _ida[iDof1];
						lm.push_back(inv);
					}
				}
				_equSys.adda(estifn, lm);
			}
		}

		// 边界条件添加
		_equSys.applyFirstBCs(_nodeBC1s, _ida);
		_equSys.applySecondBCs(_nodeBC2s, _ida);
		return 0;
	}

	std::map<std::string, std::vector<double>> PhyFieldData::getCoef1(std::vector<int> nodeIds)
	{
		std::map<std::string, std::vector<double>> coef;
		return coef;
	}

	std::map<std::string, std::vector<double>> PhyFieldData::getCoef(std::vector<int> nodeIds)
	{
		std::map<std::string, std::vector<double>> coef;
		return coef;
	}

	void PhyFieldData::setNPts(int n)
	{
		_kVar = n * _dof;
		//_nodeRes.resize(_kVar);
		_ida.resize(_kVar, -1);
		_equSys._f.resize(_kVar);
		//int dof2 = n * _dof2;
	}


	int PhyFieldData::addEleSub(EleSubBase* eleSub)
	{
		_eleSubs.push_back(eleSub);
		return _eleSubs.size() - 1;
	}

	void PhyFieldData::addBoundary(int nodeId, int typeId, double val, int iDof)
	{
		int i = _femData->_nodeIdMap[nodeId] * _dof + iDof;
		if (typeId == 1)
			_nodeBC1s[i] = val;
		else if (typeId == 2)
			_nodeBC2s[i] = val;
	}

	void PhyFieldData::setFirstBoundry(int nodeId, double val/*=0*/, int iDof /*= 0*/)
	{
		int i = _femData->_nodeIdMap[nodeId] * _dof + iDof;
		_nodeBC1s[i] = val;
	}

	void PhyFieldData::setSecondBoundry(int nodeId, double val, int iDof)
	{
		int i = _femData->_nodeIdMap[nodeId] * _dof + iDof;
		_nodeBC2s[i] = val;
	}

	int PhyFieldData::solve()
	{
		return _equSys.solve();
	}

	int PhyFieldData::uPhy()
	{
		for (const std::string& disp : _dispNames)
		{
			_nodeRes[disp].resize( _femData->_nPts);
		}
		for (const std::string& str:_eleResNames)
		{
			_elemRes[str].resize(_femData->_nElem);
		}
		for (size_t iDof = 0; iDof < _dof; iDof++)
		{
			std::string& dispName = _dispNames[iDof];
			for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode) {
				int id = _ida[DOF_ID(iNode, iDof)];
				if (id != -1)_nodeRes[dispName][iNode] = _equSys._rhs[id];
			}
		}
		int dim = _femData->_dim;
		int nEleSub = _eleSubs.size();
		for (int iEleSub = 0; iEleSub < nEleSub; ++iEleSub)
		{
			SIFEG::EleSubBase* eleSub = _eleSubs[iEleSub];
			int nNode = eleSub->getnNodesPerEle();
			int k = nNode * _dof;
			const std::vector<int>& eleIds = eleSub->getEleIds();
			for (int eleID : eleSub->_eleIds)
			{
				std::vector<double> r;
				std::vector<int> nodeIds;
				std::map<std::string, std::vector<double>> coef;
				for (int i = _femData->_elePt[eleID]; i < _femData->_elePt[eleID + 1]; ++i)
				{
					int nodeId = _femData->_eleNodes[i];
					for (size_t iDof = 0; iDof < _dof; iDof++)
					{
						std::string& dispName = _dispNames[iDof];
						coef[dispName].push_back(_nodeRes[dispName][nodeId]);
					}
					nodeIds.push_back(nodeId);
					// 节点坐标赋值
					int iCoor = dim * nodeId;
					for (int iDim = 0; iDim < dim; ++iDim)
					{
						r.push_back(_femData->_nodes[iCoor + iDim]);
					}
				}
				const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
				uResult res=eleSub->uEle(r, coef, matParams);
				for (const auto& it : res.eleResult)
				{
					const std::string& resName = it.first;
					_elemRes[resName][eleID] = it.second;
				}
				for (const auto& it : res.nodeResult)
				{
					const std::string& resName = it.first;
					const std::vector<double>& resVals = it.second;
					if (resVals.size() == 1)_elemRes[resName][eleID] = resVals[0];
				}
			}
		}
		return 1;
	}


	//确定矩阵的维度和非零元位置
	int PhyFieldData::initMatrix()
	{
		int knode= _femData->_nPts;
		_kVar= knode * _dof;
		int iDof = -1;
		std::vector<int>& elePts = _femData->_elePt;
		std::vector<int>& eleNodes = _femData->_eleNodes;
		std::vector<std::set<int>> mht0(_kVar);
		int nElement;
		int iNode = -1;
		int nNodesPerEle;
		int nMaxNoZero;
		int nEq = -1;
		std::vector<std::set<int>> mht(_kVar);
		// 根据节点所在单元确定节点自由度状况，-1代表无此自由度，>=0代表相应的方程编号
		for (SIFEG::EleSubBase* eleSub:_eleSubs)
		{
			nNodesPerEle = eleSub->getnNodesPerEle();
			std::vector<int>& eleIds = eleSub->_eleIds;
			nElement = eleIds.size();
			for (int iEle : eleIds)
			{
				// 记录在此单元中的自由度对应的方程号
				std::vector<int> lm;
				for (int iNode1 = elePts[iEle]; iNode1 < elePts[iEle + 1]; ++iNode1)
				{
					iNode = eleNodes[iNode1];
					for (int i = 0; i < _dispNames.size(); ++i) {
						std::string& dispName = _dispNames[i];
						iDof = iNode * _dof + i;
						
						if (std::find(eleSub->_dispNames.begin(), eleSub->_dispNames.end(), dispName) != eleSub->_dispNames.end())
						{
							// 根据节点所在单元确定节点自由度状况，-1代表无此自由度，>=0代表相应的方程编号
							if (_ida[iDof] == -1)_ida[iDof] = ++nEq;
							if (_ida[iDof]>=0)lm.push_back(_ida[iDof]);
						}
					}
				}
				for (int ni : lm)
				{
					for (int nj : lm)
					{
						mht[ni].insert(nj);
					}
				}
			}
		}
		_neq =++nEq;
		mht.resize(_neq);
		_equSys.init(mht);
		return 1;
	}

	std::vector<double> PhyFieldData::getNodeDisps(const std::vector<int>& nodeIds)
	{
		std::vector<double> rt;
		return rt;
	}

	std::vector<double> PhyFieldData::getNodeDisps(const std::vector<int>& nodeIds, int iDof)
	{
		std::vector<double> rt;

		return rt;
	}

}
