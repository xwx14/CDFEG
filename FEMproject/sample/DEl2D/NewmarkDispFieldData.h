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

#ifndef NEWMARK_DISP_FIELD_DATA_H
#define NEWMARK_DISP_FIELD_DATA_H
#include "CDFEG/PhyFieldData.h"
#include <vector>
#include <string>

// 二维弹性动力学位移物理场（Newmark-β 逐步积分）
// 相比静力场 ElDispFieldData：
//   1) 新增上一步位移 _u1 / 速度 _v1 / 加速度 _w1（按节点存储）
//   2) 新增当前步位移 _u / 速度 _v / 加速度 _w
//   3) 重写 eProgram：装配 Newmark 有效矩阵 K+a0·M+a1·C 与有效载荷
//   4) 重写 uPhy：由解向量更新加速度/速度/位移，并保存为下一步初值，再做应力恢复
class NewmarkDispFieldData : public CDFEG::PhyFieldData {
public:
    NewmarkDispFieldData(CDFEG::FEMData* femData);
    ~NewmarkDispFieldData();

    // 设置 Newmark 积分参数（dt/tmax 由 FEMData 持有，gamma/beta 默认 0.5/0.25 平均加速度法）
    void setNewmarkParams(double gamma, double beta, double dt);
    // 设置初值（按节点号设置该节点 u/v/w 初值，未设置的节点默认 0）
    void setInitialDisp(int nodeId, double u, double v);
    void setInitialVel(int nodeId, double vu, double vv);
    void setInitialAcc(int nodeId, double au, double av);

    // 重写 E 程序：装配有效矩阵 + 有效载荷（Newmark 算法核心）
    virtual int eProgram() override;
    // 重写 u 程序：更新加速度/速度/位移历史 + 应力恢复
    virtual int uPhy() override;

public:
    // 当前步位移/速度/加速度（按节点存储，长度 = 节点数，下标为程序内节点号）
    std::vector<double> _u, _v, _w;
    // 上一步位移/速度/加速度（Newmark 积分历史量）
    std::vector<double> _u1, _v1, _w1;

    // Newmark-β 积分常数
    double _gamma = 0.5;   // γ
    double _beta = 0.25;   // β
    double _dt = 0.0;      // 时间步长
    // a0..a7 见 newmarka.nfe 标准公式
    double _a0, _a1, _a2, _a3, _a4, _a5, _a6, _a7;

private:
    // 由 _gamma/_beta/_dt 计算积分常数 a0..a7
    void calcNewmarkConstants();
    // 确保历史向量(_u/_v/_w/_u1/_v1/_w1)与 _kVar 一致，未初始化则填零
    void ensureHistorySize();
};

#endif
