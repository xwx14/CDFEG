#include "hel2dData.h"
#include "HeatFieldData.h"
#include "DelDispFieldData.h"

hel2dData::hel2dData() {
    _dim = 2;
    _phyDatas.push_back(new HeatFieldData(this));
    _phyDatas.push_back(new DelDispFieldData(this));
}

hel2dData::~hel2dData() {

}

int hel2dData::caculate() {
    // 顺序耦合（对应旧 hel.c）：先解温度场 a，再以温度为热载荷解位移场 b
    HeatFieldData* heatField = static_cast<HeatFieldData*>(_phyDatas[0]);
    DelDispFieldData* delField = static_cast<DelDispFieldData*>(_phyDatas[1]);

    // 各场方程编号 + 稀疏骨架
    heatField->initMatrix();
    delField->initMatrix();

    // a 场（Heat）：组装热传导总刚 + 热源 → 求解温度 → 回填 _nodeRes["T"]
    heatField->eProgram();
    heatField->solve();
    heatField->uPhy();

    // b 场（DelDisp）：组装弹性总刚 + 体力/热载荷（取温度）→ 求解位移 → 回填位移 + 应力
    delField->eProgram();
    delField->solve();
    delField->uPhy();

    return 1;
}

int hel2dData::main() {
    caculate();
    return 1;
}