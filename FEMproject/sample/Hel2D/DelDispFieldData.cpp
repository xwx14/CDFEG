#include "DelDispFieldData.h"
#include "hel2dData.h"
#include "DelQ4g.h"
#include "CDFEG/ElementBase.h"
#include "CDFEG/EquationSystem.h"
#include <cmath>

DelDispFieldData::DelDispFieldData(CDFEG::DomainData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="DelDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new DelQ4g(this));
    _eleResNames = { "sigmaXX", "sigmaYY", "sigmaXY", "volume" };
    _resForm = "Vector OnNodes";
    _coefNames[0] = { "T" }; // 需要从 Heat 场取温度
}

DelDispFieldData::~DelDispFieldData() {

}

// u 程序：回填位移 + 应力恢复（应力在位移场后处理计算）
int DelDispFieldData::uPhy()
{
    // 1) 回填节点位移
    for (const std::string& disp : _dispNames)
        _nodeRes[disp].resize(_femData->_nPts);
    for (const std::string& str : _eleResNames)
        _elemRes[str].resize(_femData->_nElem);
    for (size_t iDof = 0; iDof < _dof; iDof++)
    {
        std::string& dispName = _dispNames[iDof];
        for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode)
        {
            int id = _ida[DOF_ID(iNode, iDof)];
            if (id != -1) _nodeRes[dispName][iNode] = _equSys._rhs[id];
        }
    }

    // 2) 应力恢复：uEle 由位移+温度反求应力，N 加权外推到节点
    std::vector<std::string> stressNames = { "sigmaXX", "sigmaYY", "sigmaXY" };
    std::map<std::string, std::vector<double>> nodeStressSum;
    std::vector<double> nodeWeightSum(_femData->_nPts, 0.0);
    for (const std::string& name : stressNames)
        nodeStressSum[name].resize(_femData->_nPts, 0.0);

    int dim = _femData->_dim;
    int nEleSub = _eleSubs.size();
    for (int iEleSub = 0; iEleSub < nEleSub; ++iEleSub)
    {
        CDFEG::ElementBase* eleSub = _eleSubs[iEleSub];
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
                    std::string dispName = _dispNames[iDof];
                    coef[dispName].push_back(_nodeRes[dispName][nodeId]);
                }
                nodeIds.push_back(nodeId);
                int iCoor = dim * nodeId;
                for (int iDim = 0; iDim < dim; ++iDim)
                    r.push_back(_femData->_nodes[iCoor + iDim]);
            }
            const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
            CDFEG::uResult res = eleSub->uEle(r, coef, matParams);

            for (const auto& it : res.eleResult)
                _elemRes[it.first][eleID] = it.second;

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
                            nodeStressSum[name][nodeId] += res.nodeResult.at(name)[i];
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
            _nodeRes[name][iNode] = (nodeWeightSum[iNode] > 0.0)
                ? nodeStressSum[name][iNode] / nodeWeightSum[iNode]
                : 0.0;
        }
    }

    // von Mises 等效应力
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
