// 验证 _paramValues / getParam 取值逻辑（不依赖 dat 读取，直接手工填值）
#include "CDFEG/FemData.h"
#include "CDFEG/PhyFieldData.h"
#include "CDFEG/ElementBase.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CDFEG;
    FEMData* fd = new FEMData();
    PhyFieldData* f = new PhyFieldData(2, fd);
    ElementBase* e = new ElementBase(4, f);

    f->_addParams = { {"newmark","gamma","beta"} };
    f->_paramValues["newmark"] = { 0.5, 0.25 };
    e->_addParams = { {"euler","theta"} };
    e->_paramValues["euler"] = { 1.5 };

    bool ok = true;
    ok &= (f->getParam("newmark","gamma") == 0.5);
    ok &= (f->getParam("newmark","beta")  == 0.25);
    ok &= (f->getParam("newmark","nope")  == 0.0);   // 组存在但无此参数名
    ok &= (f->getParam("other","x")       == 0.0);   // 组不存在
    ok &= (e->getParam("euler","theta")   == 1.5);

    std::cout << (ok ? "TEST PASS" : "TEST FAIL") << std::endl;
    return ok ? 0 : 1;
}
