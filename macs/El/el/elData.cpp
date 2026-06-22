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

    // elb 后求解（应力场），依赖 ela 位移结果
    elbFieldData* elb = static_cast<elbFieldData*>(_phyDatas[1]);
    elb->initMatrix();
    elb->eProgram();
    elb->solve();
    elb->uPhy();

    return 1;
}

int elData::main() {
    caculate();
    return 1;
}
