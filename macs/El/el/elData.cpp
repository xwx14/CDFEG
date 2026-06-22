#include "elData.h"
#include "elaFieldData.h"
#include "elbFieldData.h"

elData::elData() {
    _dim = 2;
    _phyDatas.push_back(new elaFieldData(this));
    _phyDatas.push_back(new elbFieldData(this));
}

elData::~elData() {

}

int elData::caculate() {
    // ela 先求解（位移场），保证 elb getCoef 时 ela._nodeRes 已填
    elaFieldData* ela = static_cast<elaFieldData*>(_phyDatas[0]);
    ela->initMatrix();
    ela->eProgram();
    ela->solve();
    ela->uPhy();

    // elb 后处理（应力场）：显式最小二乘法，不组装方程组、不 solve
    // eProgram 内部完成 mass/load 累加 → stress=load/mass，结果写入 _nodeRes
    elbFieldData* elb = static_cast<elbFieldData*>(_phyDatas[1]);
    elb->eProgram();

    return 1;
}

int elData::main() {
    caculate();
    return 1;
}
