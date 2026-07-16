#include "HeatFieldData.h"
#include "hel2dData.h"
#include "HeatQ4g.h"
#include "CDFEG/ElementBase.h"
#include "CDFEG/EquationSystem.h"

HeatFieldData::HeatFieldData(CDFEG::DomainData* femData)
    : CDFEG::PhyFieldData(1, femData) {
    _name="Heat";
    _dispNames = { "T" };
    _dof2 = 1;
    _eleSubs.push_back(new HeatQ4g(this));
    _eleResNames = {  };
}

HeatFieldData::~HeatFieldData() {

}

// E 程序：组装热传导总刚 + 热源载荷 → 右端项
int HeatFieldData::eProgram()
{
    std::fill(_equSys._data.begin(), _equSys._data.end(), 0.0);
    std::fill(_equSys._f.begin(), _equSys._f.end(), 0.0);

    int dim = _femData->_dim;
    int nEleSub = _eleSubs.size();
    for (int iEleSub = 0; iEleSub < nEleSub; ++iEleSub)
    {
        CDFEG::ElementBase* eleSub = _eleSubs[iEleSub];
        int nNode = eleSub->getnNodesPerEle();
        int k = nNode * _dof;

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
                    r.push_back(_femData->_nodes[iCoor + iDim]);
            }
            const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
            std::map<std::string, std::vector<double>> coef;   // 热场不依赖其它场
            CDFEG::EleSubResult& outData = eleSub->run(r, coef, matParams);

            // 单元定位向量 lm（与 eProgram_el 一致）
            std::vector<int> lm;
            for (int nodi : nodeIds)
            {
                int iStart = nodi * _dof;
                for (int iDof = 0; iDof < _dof; ++iDof)
                    lm.push_back(_ida[iStart + iDof]);
            }

            _equSys.adda(outData.estif, lm);

            // eload → 右端项 _f（热源载荷）
            for (int i = 0; i < k; ++i)
            {
                int inv = lm[i];
                if (inv >= 0) _equSys._f[inv] += outData.eload[i];
            }
        }
    }

    // 边界条件：_data 已重置，强制 applyFirstBCs 重建基线缓存
    _equSys._bSavedData0 = false;
    _equSys.applyFirstBCs(_nodeBC1s, _ida);
    _equSys.applySecondBCs(_nodeBC2s, _ida);
    return 1;
}
