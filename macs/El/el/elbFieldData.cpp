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
    int dim = _femData->_dim;
    int nEleSub = _eleSubs.size();
    // 组装 lumped mass（estif）+ 应力载荷（eload）到总刚/右端项
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
            // 跨场取 ela 位移（ela::u / ela::v），供 beq4g2 计算应力载荷
            std::map<std::string, std::vector<double>> coef = getCoef(nodeIds);
            CDFEG::EleSubResult& outData = eleSub->run(r, coef, matParams);
            // 构建定位向量 lm
            std::vector<int> lm;
            for (int nodi : nodeIds)
            {
                int iStart = nodi * _dof;
                for (int iDof = 0; iDof < _dof; ++iDof)
                {
                    lm.push_back(_ida[iDof + iStart]);
                }
            }
            // estif（lumped mass 对角）组装到总刚
            _equSys.adda(outData.estif, lm);
            // eload（应力载荷）累加到右端项
            const std::vector<double>& eload = outData.eload;
            if (!eload.empty())
            {
                for (int i = 0; i < (int)lm.size(); ++i)
                {
                    if (lm[i] >= 0)
                    {
                        _equSys._f[lm[i]] += eload[i];
                    }
                }
            }
        }
    }

    // 边界条件
    _equSys.applyFirstBCs(_nodeBC1s, _ida);
    _equSys.applySecondBCs(_nodeBC2s, _ida);
    return 1;
}
