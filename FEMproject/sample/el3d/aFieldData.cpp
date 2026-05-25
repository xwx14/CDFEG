#include "aFieldData.h"
#include "el3dData.h"
#include "a1ec8g2.h"
#include "a2ew4g2.h"
#include "a3lq4.h"
#include <cmath>

aFieldData::aFieldData(SIFEG::FEMData* femData)
    : SIFEG::PhyFieldData(3, femData) {
    _name="a";
    _dispNames = { "u", "v", "w" };
    _eleResNames = { "sigmaXX" ,"sigmaYY","sigmaZZ","sigmaYZ","sigmaXZ", "sigmaXY","vonMises" };
    _dof2 = 3;
    _eleSubs.push_back(new a1ec8g2(this));
    _eleSubs.push_back(new a2ew4g2(this));
    _eleSubs.push_back(new a3lq4(this));
    _eleResNames = {  };
    _resForm = "Vector OnNodes";
}

aFieldData::~aFieldData() {

}

int aFieldData::uPhy() {
    // 初始化节点位移结果
    for (const std::string& disp : _dispNames)
    {
        _nodeRes[disp].resize(_femData->_nPts);
    }
    // 初始化单元结果
    for (const std::string& str : _eleResNames)
    {
        _elemRes[str].resize(_femData->_nElem);
    }
    // 提取节点位移
    for (size_t iDof = 0; iDof < _dof; iDof++)
    {
        std::string& dispName = _dispNames[iDof];
        for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode) {
            int id = _ida[DOF_ID(iNode, iDof)];
            if (id != -1)_nodeRes[dispName][iNode] = _equSys._rhs[id];
        }
    }

    // 节点应力累积（带权重）
    std::vector<std::string> stressNames = { "sigmaXX", "sigmaYY", "sigmaZZ", "sigmaYZ", "sigmaXZ", "sigmaXY" };
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
                int iCoor = dim * nodeId;
                for (int iDim = 0; iDim < dim; ++iDim)
                {
                    r.push_back(_femData->_nodes[iCoor + iDim]);
                }
            }
            const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
            SIFEG::uResult res = eleSub->uEle(r, coef, matParams);

            // 存储单元结果
            for (const auto& it : res.eleResult)
            {
                const std::string& resName = it.first;
                _elemRes[resName][eleID] = it.second;
            }

            // 累积节点应力（带权重）
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

    // 计算节点平均应力
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

    // 计算节点 Von Mises 应力
    _nodeRes["vonMises"].resize(_femData->_nPts);
    for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode)
    {
        double sXX = _nodeRes["sigmaXX"][iNode];
        double sYY = _nodeRes["sigmaYY"][iNode];
        double sZZ = _nodeRes["sigmaZZ"][iNode];
        double sYZ = _nodeRes["sigmaYZ"][iNode];
        double sXZ = _nodeRes["sigmaXZ"][iNode];
        double sXY = _nodeRes["sigmaXY"][iNode];
        _nodeRes["vonMises"][iNode] = sqrt(0.5 * ((sXX - sYY) * (sXX - sYY) +
                                                   (sYY - sZZ) * (sYY - sZZ) +
                                                   (sZZ - sXX) * (sZZ - sXX) +
                                                   6.0 * (sXY * sXY + sYZ * sYZ + sXZ * sXZ)));
    }

    return 1;
}
