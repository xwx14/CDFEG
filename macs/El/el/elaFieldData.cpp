#include "elaFieldData.h"
#include "elData.h"
#include "a1eq4g2.h"
#include "a2ll2.h"

elaFieldData::elaFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name="ela";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    _eleSubs.push_back(new a1eq4g2(this));
    _eleSubs.push_back(new a2ll2(this));
    _eleResNames = { "exx", "eyy", "exy" };
}

elaFieldData::~elaFieldData() {

}

int elaFieldData::eProgram()
{
    int dim = _femData->_dim;
    int kNode = _femData->_nPts;
    int nEleSub = _eleSubs.size();
    // 组装刚度矩阵 + eload 累加到右端项
    for (int iEleSub = 0; iEleSub < nEleSub; ++iEleSub)
    {
        CDFEG::ElementBase* eleSub = _eleSubs[iEleSub];
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
            CDFEG::EleSubResult& outData = eleSub->run(r, coef, matParams);
            std::vector<double>& estifn = outData.estif;
            // 构建定位向量 lm
            int iDof1;
            int inv;
            std::vector<int> lm;
            for (int nodi : nodeIds)
            {
                int iStart = nodi * _dof;
                for (int iDof = 0; iDof < _dof; ++iDof)
                {
                    iDof1 = iDof + iStart;
                    inv = _ida[iDof1];
                    lm.push_back(inv);
                }
            }
            // 组装单刚到总刚
            _equSys.adda(estifn, lm);
            // eload 累加到右端项（a1eq4g2 体力 / a2ll2 面力）
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
