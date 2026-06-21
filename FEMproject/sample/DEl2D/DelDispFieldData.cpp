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
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>

#include "DelDispFieldData.h"
#include "del2dData.h"
#include "DelQ4g.h"
#include "CDFEG/ElementBase.h"
#include "CDFEG/EquationSystem.h"
#include <cmath>

DelDispFieldData::DelDispFieldData(CDFEG::FEMData* femData)
    : CDFEG::PhyFieldData(2, femData) {
    _name = "DelDisp";
    _dispNames = { "u", "v" };
    _dof2 = 2;
    // 注册动力单元（含集中质量/阻尼输出）
    _eleSubs.push_back(new DelQ4g(this));
    // 应力恢复结果名
    _eleResNames = { "sigmaXX", "sigmaYY", "sigmaXY", "volume" };
    _resForm = "Vector OnNodes";
    // 声明从前处理读取的 Newmark 积分参数组（gamma/beta）
    _addParams = { {"newmark","gamma","beta"} };
}

DelDispFieldData::~DelDispFieldData() {

}

void DelDispFieldData::ensureHistorySize()
{
    if ((int)_u1.size() != _kVar)
    {
        _u1.assign(_kVar, 0.0);
        _v1.assign(_kVar, 0.0);
        _w1.assign(_kVar, 0.0);
        _u.assign(_kVar, 0.0);
        _v.assign(_kVar, 0.0);
        _w.assign(_kVar, 0.0);
    }
}

void DelDispFieldData::setNewmarkParams(double gamma, double beta, double dt)
{
    _gamma = gamma;
    _beta = beta;
    _dt = dt;
    calcNewmarkConstants();
}

void DelDispFieldData::calcNewmarkConstants()
{
    // 标准 Newmark-β 积分常数（对应旧 newmarka.nfe 的 @begin 段）
    double o = _gamma;
    double aa = _beta;
    _a0 = 1.0 / (_dt * _dt * aa);
    _a1 = o / (aa * _dt);
    _a2 = 1.0 / (aa * _dt);
    _a3 = 1.0 / (2.0 * aa) - 1.0;
    _a4 = o / aa - 1.0;
    _a5 = _dt / 2.0 * (o / aa - 2.0);
    _a6 = _dt * (1.0 - o);
    _a7 = _dt * o;
}

void DelDispFieldData::setInitialDisp(int nodeId, double u, double v)
{
    ensureHistorySize();
    _u1[nodeId * _dof + 0] = u;
    _u1[nodeId * _dof + 1] = v;
}

void DelDispFieldData::setInitialVel(int nodeId, double vu, double vv)
{
    ensureHistorySize();
    _v1[nodeId * _dof + 0] = vu;
    _v1[nodeId * _dof + 1] = vv;
}

void DelDispFieldData::setInitialAcc(int nodeId, double au, double av)
{
    ensureHistorySize();
    _w1[nodeId * _dof + 0] = au;
    _w1[nodeId * _dof + 1] = av;
}

int DelDispFieldData::eProgram()
{
    ensureHistorySize();
    int dim = _femData->_dim;

    // 每步重新装配有效矩阵与有效载荷，先清零
    std::fill(_equSys._data.begin(), _equSys._data.end(), 0.0);
    std::fill(_equSys._f.begin(), _equSys._f.end(), 0.0);

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
                {
                    r.push_back(_femData->_nodes[iCoor + iDim]);
                }
            }
            const std::map<std::string, double>& matParams = _femData->getElemMatParams(eleID, eleSub);
            std::map<std::string, std::vector<double>> coef;
            CDFEG::EleSubResult& outData = eleSub->run(r, coef, matParams);
            std::vector<double>& estif = outData.estif;
            std::vector<double>& emass = outData.emass;
            std::vector<double>& edamp = outData.edamp;
            std::vector<double>& eload = outData.eload;

            // 局部方程号列表 lm（与 eProgram_el 一致）
            std::vector<int> lm;
            for (int nodi : nodeIds)
            {
                int iStart = nodi * _dof;
                for (int iDof = 0; iDof < _dof; ++iDof)
                {
                    lm.push_back(_ida[iStart + iDof]);
                }
            }

            // 有效单元矩阵 K_eff = K + a0·M + a1·C（M、C 为集中形式，仅加对角）
            std::vector<double> effMat(k * k, 0.0);
            for (int i = 0; i < k; ++i)
            {
                for (int j = 0; j < k; ++j)
                {
                    effMat[i * k + j] = estif[i * k + j];
                }
                effMat[i * k + i] += emass[i] * _a0 + edamp[i] * _a1;
            }
            _equSys.adda(effMat, lm);

            // 有效载荷：F_eff = eload + M·u1·a0 + M·v1·a2 + M·w1·a3 + C·u1·a1 + C·v1·a4 + C·w1·a5
            // emass[i]/edamp[i] 对应节点 nodeIds[i/_dof]、自由度 i%_dof
            for (int i = 0; i < k; ++i)
            {
                int inv = lm[i];
                if (inv < 0) continue;
                int node = nodeIds[i / _dof];
                int iDof = i % _dof;
                int idx = node * _dof + iDof;
                double u1 = _u1[idx];
                double v1 = _v1[idx];
                double w1 = _w1[idx];
                double feff = eload[i]
                    + emass[i] * (u1 * _a0 + v1 * _a2 + w1 * _a3)
                    + edamp[i] * (u1 * _a1 + v1 * _a4 + w1 * _a5);
                _equSys._f[inv] += feff;
            }
        }
    }

    // 边界条件：每步 _data/_f 已重置，强制 applyFirstBCs 重建基线缓存
    _equSys._bSavedData0 = false;
    _equSys.applyFirstBCs(_nodeBC1s, _ida);
    _equSys.applySecondBCs(_nodeBC2s, _ida);
    return 1;
}

int DelDispFieldData::uPhy()
{
    ensureHistorySize();
    // 1) 由解向量 _rhs 读取当前步位移
    for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode)
    {
        for (int iDof = 0; iDof < _dof; ++iDof)
        {
            int idx = iNode * _dof + iDof;
            int id = _ida[idx];
            _u[idx] = (id != -1) ? _equSys._rhs[id] : 0.0;
        }
    }

    // 2) Newmark 加速度/速度更新（对应 unewmarka.c）
    for (size_t idx = 0; idx < _kVar; ++idx)
    {
        // w = (u - u1)·a0 - v1·a2 - w1·a3
        _w[idx] = (_u[idx] - _u1[idx]) * _a0 - _v1[idx] * _a2 - _w1[idx] * _a3;
        // v = v1 + w·a7 + w1·a6
        _v[idx] = _v1[idx] + _w[idx] * _a7 + _w1[idx] * _a6;
    }

    // 3) 节点结果输出（位移/速度/加速度）
    _nodeRes["u"].resize(_femData->_nPts);
    _nodeRes["v"].resize(_femData->_nPts);
    _nodeRes["velU"].resize(_femData->_nPts);
    _nodeRes["velV"].resize(_femData->_nPts);
    _nodeRes["accU"].resize(_femData->_nPts);
    _nodeRes["accV"].resize(_femData->_nPts);
    for (size_t iNode = 0; iNode < _femData->_nPts; ++iNode)
    {
        _nodeRes["u"][iNode] = _u[iNode * _dof + 0];
        _nodeRes["v"][iNode] = _u[iNode * _dof + 1];
        _nodeRes["velU"][iNode] = _v[iNode * _dof + 0];
        _nodeRes["velV"][iNode] = _v[iNode * _dof + 1];
        _nodeRes["accU"][iNode] = _w[iNode * _dof + 0];
        _nodeRes["accV"][iNode] = _w[iNode * _dof + 1];
    }

    // 4) 应力恢复（与 ElDispFieldData 一致：uEle 由当前位移反求应力）
    for (const std::string& str : _eleResNames)
    {
        _elemRes[str].resize(_femData->_nElem);
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
        for (int eleID : eleSub->_eleIds)
        {
            std::vector<double> r;
            std::vector<int> nodeIds;
            std::map<std::string, std::vector<double>> coef;
            for (int i = _femData->_elePt[eleID]; i < _femData->_elePt[eleID + 1]; ++i)
            {
                int nodeId = _femData->_eleNodes[i];
                for (int iDof = 0; iDof < _dof; ++iDof)
                {
                    std::string dispName = _dispNames[iDof];
                    coef[dispName].push_back(_u[nodeId * _dof + iDof]);
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
                _elemRes[it.first][eleID] = it.second;
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

    // 5) 当前步结果保存为下一步初值
    _u1 = _u;
    _v1 = _v;
    _w1 = _w;

    return 1;
}
