// 验证 _paramValues / getParam 取值 + gidPrePost 按 _addParams 声明从 dat 回填
#include "CDFEG/FemData.h"
#include "CDFEG/PhyFieldData.h"
#include "CDFEG/ElementBase.h"
#include "CDFEG/gidPrePost.h"
#include <fstream>
#include <iostream>

int main() {
    using namespace CDFEG;
    FEMData* fd = new FEMData();
    fd->_addParams = { {"proj","dim"} };
    PhyFieldData* f = new PhyFieldData(2, fd);
    f->_name = "F";
    f->_addParams = { {"newmark","gamma","beta"}, {"empty"}, {"g","a","b"} };
    f->_paramValues["g"] = { 1.0 };  // 故意只填1个，使 b 越界
    ElementBase* e = new ElementBase(4, f);
    e->_addParams = { {"euler","theta"} };
    f->_eleSubs.push_back(e);
    fd->_phyDatas.push_back(f);

    // 写临时 dat（只含额外参数段，验证读取回填；不依赖完整网格）
    // 注：dat 段头按 GidPrePost.pre() 约定为单 '*' 前缀（与真实 GiD 导出格式一致）
    std::ofstream dat("t.dat");
    dat << "* name=proj,structure=\"F64\"\n2.0 3.0\n";
    dat << "* name=newmark,structure=\"F64\"\n0.5 0.25\n";
    dat << "* name=euler,structure=\"F64\"\n1.5\n";
    dat.close();

    GidPrePost pp(fd);
    pp.setFilePath(".", "t");
    pp.pre();

    bool ok = true;
    // dat 回填到三层
    ok &= (fd->getParam("proj","dim")     == 2.0);
    ok &= (f->getParam("newmark","gamma") == 0.5);
    ok &= (f->getParam("newmark","beta")  == 0.25);
    ok &= (e->getParam("euler","theta")   == 1.5);
    // getParam 防御分支
    ok &= (f->getParam("newmark","nope")  == 0.0);  // 组存在但无此参数名
    ok &= (f->getParam("empty","x")       == 0.0);  // 组行长度<2
    ok &= (f->getParam("g","b")           == 0.0);  // _paramValues 向量越界
    ok &= (f->getParam("other","x")       == 0.0);  // 组不存在

    std::cout << (ok ? "TEST PASS" : "TEST FAIL") << std::endl;
    return ok ? 0 : 1;
}
