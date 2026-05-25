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

#include "aFieldData.h"
#include "elData.h"
#include "a1eq4g2.h"
#include "a2ll2.h"
#include <cmath>

aFieldData::aFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _dispNames = { "u", "v" };
    _eleResNames = { "sigmaXX" ,"sigmaYY","sigmaXY","vonMises","volume"};
    _dof2 = 2;
    _eleSubs.push_back(new a1eq4g2(this));
    _eleSubs.push_back(new a2ll2(this));
    _name = "a";
    _resForm = "Vector OnNodes";
}

aFieldData::~aFieldData() {

}

int aFieldData::uPhy() {
    for (const std::string& disp : _dispNames)
    {
        _nodeRes[disp].resize(_femData->_nPts);
    }
    for (const std::string& str : _eleResNames)
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

    std::vector<std::string> stressNames = { "sigmaXX", "sigmaYY", "sigmaXY" };
    std::map<std::string, std::vector<double>> nodeStressSum;
    std::vector<double> nodeWeightSum(_femData->_nPts, 0.0);
    for (const std::string& name : stressNames)
    {
        nodeStressSum[name].resize(_femData->_nPts, 0.0);
    }

    int dim = _femData->_dim;
    int nEleSub = _eleSubs.size();
    for (int iEleSub = 0; iEleSub < nEleSub; ++iEleSub)
    {
        CDFEG::ElementBase* eleSub = _eleSubs[iEleSub];
        int nNode = eleSub->getnNodesPerEle();
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
                int iCoor = dim * nodeId;
                for (int iDim = 0; iDim < dim; ++iDim)
                {
                    r.push_back(_femData->_nodes[iCoor + iDim]);
                }
            }
            const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
            CDFEG::uResult res = eleSub->uEle(r, coef, matParams);

            for (const auto& it : res.eleResult)
            {
                const std::string& resName = it.first;
                _elemRes[resName][eleID] = it.second;
            }

            if (res.nodeResult.find("weight") != res.nodeResult.end())
            {
                const std::vector<double>& weights = res.nodeResult.at("weight");
                for (size_t i = 0; i < nodeIds.size(); ++i)
                {
                    int nodeId = nodeIds[i];
                    nodeWeightSum[nodeId] += weights[i];
                    for (const std::string& name : stressNames)
                    {
                        if (res.nodeResult.find(name) != res.nodeResult.end())
                        {
                            nodeStressSum[name][nodeId] += res.nodeResult.at(name)[i];
                        }
                    }
                }
            }
        }
    }

    for (const std::string& name : stressNames)
    {
        _nodeRes[name].resize(_femData->_nPts);
        for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode)
        {
            if (nodeWeightSum[iNode] > 0.0)
            {
                _nodeRes[name][iNode] = nodeStressSum[name][iNode] / nodeWeightSum[iNode];
            }
            else
            {
                _nodeRes[name][iNode] = 0.0;
            }
        }
    }

    _nodeRes["vonMises"].resize(_femData->_nPts);
    for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode)
    {
        double sXX = _nodeRes["sigmaXX"][iNode];
        double sYY = _nodeRes["sigmaYY"][iNode];
        double sXY = _nodeRes["sigmaXY"][iNode];
        _nodeRes["vonMises"][iNode] = sqrt(sXX * sXX - sXX * sYY + sYY * sYY + 3.0 * sXY * sXY);
    }

    return 1;
}
