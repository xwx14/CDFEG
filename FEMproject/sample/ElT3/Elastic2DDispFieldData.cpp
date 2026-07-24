#include "Elastic2DDispFieldData.h"
#include "ElasticT3Data.h"
#include "ElT3.h"

Elastic2DDispFieldData::Elastic2DDispFieldData(CDFEG::DomainData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="Elastic2DDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new ElT3(this));
    _eleResNames = { "Sxx", "Syy", "Sxy" };
}

Elastic2DDispFieldData::~Elastic2DDispFieldData() {

}

int Elastic2DDispFieldData::uPhy() {
    for (const std::string& disp : _dispNames)
        _nodeRes[disp].resize(_femData->_nPts);
    for (const std::string& str : _eleResNames)
        _elemRes[str].resize(_femData->_nElem);

    // 回填节点位移
    for (size_t iDof = 0; iDof < _dof; ++iDof) {
        const std::string& dispName = _dispNames[iDof];
        for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode) {
            int id = _ida[DOF_ID(iNode, iDof)];
            if (id != -1) _nodeRes[dispName][iNode] = _equSys._rhs[id];
        }
    }

    // 节点应力外推（最小二乘加权平均：常应变单元应力外推到节点）
    const std::vector<std::string> stressNames = { "Sxx", "Syy", "Sxy" };
    std::map<std::string, std::vector<double>> nodeStressSum;
    std::vector<double> nodeWeightSum(_femData->_nPts, 0.0);
    for (const std::string& name : stressNames)
        nodeStressSum[name].resize(_femData->_nPts, 0.0);

    int dim = _femData->_dim;
    for (CDFEG::ElementBase* eleSub : _eleSubs) {
        for (int eleID : eleSub->_eleIds) {
            std::vector<double> r;
            std::vector<int> nodeIds;
            std::map<std::string, std::vector<double>> coef;
            for (int i = _femData->_elePt[eleID]; i < _femData->_elePt[eleID + 1]; ++i) {
                int nodeId = _femData->_eleNodes[i];
                for (size_t iDof = 0; iDof < _dof; ++iDof)
                    coef[_dispNames[iDof]].push_back(_nodeRes[_dispNames[iDof]][nodeId]);
                nodeIds.push_back(nodeId);
                int iCoor = dim * nodeId;
                for (int iDim = 0; iDim < dim; ++iDim)
                    r.push_back(_femData->_nodes[iCoor + iDim]);
            }
            const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
            CDFEG::uResult res = eleSub->uEle(r, coef, matParams);

            for (const auto& it : res.eleResult)
                _elemRes[it.first][eleID] = it.second;

            if (res.nodeResult.find("weight") != res.nodeResult.end()) {
                const std::vector<double>& weights = res.nodeResult.at("weight");
                for (size_t i = 0; i < nodeIds.size(); ++i) {
                    int nodeId = nodeIds[i];
                    nodeWeightSum[nodeId] += weights[i];
                    for (const std::string& name : stressNames)
                        if (res.nodeResult.find(name) != res.nodeResult.end())
                            nodeStressSum[name][nodeId] += res.nodeResult.at(name)[i];
                }
            }
        }
    }

    for (const std::string& name : stressNames) {
        _nodeRes[name].resize(_femData->_nPts);
        for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode)
            _nodeRes[name][iNode] = nodeWeightSum[iNode] > 0.0
                ? nodeStressSum[name][iNode] / nodeWeightSum[iNode] : 0.0;
    }
    return 1;
}
