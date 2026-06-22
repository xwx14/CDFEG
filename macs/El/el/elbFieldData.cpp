#include "elbFieldData.h"
#include "elData.h"
#include "beq4g2.h"

elbFieldData::elbFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(3, femData) {
    _name="elb";
    _dispNames = { "dxx", "dyy", "dxy" };
    _dof2 = 3;
    _eleSubs.push_back(new beq4g2(this));
    _eleResNames = {  };
}

elbFieldData::~elbFieldData() {

}

int elbFieldData::eProgram()
{
    // elb 为应力场，采用显式最小二乘法（参照旧项目 eelb.c）：
    // 不组装总刚、不施加边界、不求解方程组；beq4g2 返回 lumped mass(emass) 与应力
    // 载荷(eload)，节点级累加后直接 stress = load / mass。
    int nPts = _femData->_nPts;
    int dim = _femData->_dim;
    // 节点级累加器，索引 = nodeId*_dof + iDof
    std::vector<double> nodeMass(nPts * _dof, 0.0);
    std::vector<double> nodeLoad(nPts * _dof, 0.0);

    int nEleSub = _eleSubs.size();
    for (int iEleSub = 0; iEleSub < nEleSub; ++iEleSub)
    {
        CDFEG::ElementBase* eleSub = _eleSubs[iEleSub];
        int nNode = eleSub->getnNodesPerEle();
        for (int eleID : eleSub->_eleIds)
        {
            std::vector<double> r;
            std::vector<int> nodeIds;
            for (int i = _femData->_elePt[eleID]; i < _femData->_elePt[eleID + 1]; ++i)
            {
                int nodeId = _femData->_eleNodes[i];
                nodeIds.push_back(nodeId);
                int iCoor = dim * nodeId;
                for (int iDim = 0; iDim < dim; ++iDim)
                {
                    r.push_back(_femData->_nodes[iCoor + iDim]);
                }
            }
            const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
            // 跨场取 ela 位移（ela::u / ela::v），供 beq4g2 计算应变/应力
            std::map<std::string, std::vector<double>> coef = getCoef(nodeIds);
            CDFEG::EleSubResult& outData = eleSub->run(r, coef, matParams);
            // 单元 emass/eload 累加到节点级（按 节点×自由度）
            const std::vector<double>& emass = outData.emass;
            const std::vector<double>& eload = outData.eload;
            for (int i = 0; i < nNode; ++i)
            {
                int ivNode = nodeIds[i] * _dof;
                int ivEle = i * _dof;
                for (int iDof = 0; iDof < _dof; ++iDof)
                {
                    nodeMass[ivNode + iDof] += emass[ivEle + iDof];
                    nodeLoad[ivNode + iDof] += eload[ivEle + iDof];
                }
            }
        }
    }

    // lumping 防除零（eelb.c:164-170）：过小的质量充零保护
    double emmax = 0.0;
    for (double v : nodeMass)
    {
        if (v > emmax) emmax = v;
    }
    double emmin = emmax / 1e8;
    for (double& v : nodeMass)
    {
        if (v < emmin) v = emmin;
    }

    // 显式最小二乘：stress = load / mass，直接写入节点结果
    for (const std::string& disp : _dispNames)
    {
        _nodeRes[disp].resize(nPts);
    }
    for (int iNode = 0; iNode < nPts; ++iNode)
    {
        int iv = iNode * _dof;
        for (int iDof = 0; iDof < _dof; ++iDof)
        {
            _nodeRes[_dispNames[iDof]][iNode] = nodeLoad[iv + iDof] / nodeMass[iv + iDof];
        }
    }
    return 1;
}
