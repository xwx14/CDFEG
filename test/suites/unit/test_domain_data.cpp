// DomainData 机制断言；首批守护 getElemMatParams 的两个分支
#include "third/catch.hpp"
#include "CDFEG/DomainData.h"
#include "CDFEG/PhyFieldData.h"
#include "CDFEG/ElementBase.h"
#include <map>
#include <string>

using namespace CDFEG;

// ElementBase 构造会解引用 _phyData->_femData（见 ElementBase.cpp:25），
// 故需先构造有效 PhyFieldData(_femData=&dd) 再传给 ElementBase。

// getElemMatParams 第一分支：ElementBase._eleMatIDMap 含 eleID（旧版本数据兼容），
// 应返回 _mateParams[_eleMatIDMap[eleID]]。
// 历史 bug：判断写成 find(eleID)!=find(eleID) 恒 false，此分支永不触发、一律走 else。
// 本测试在 bug 下 FAIL（走 else 返回 E=1.0，期望 E=2.0）。
TEST_CASE("getElemMatParams: eleMatIDMap 含 eleID 走第一分支", "[domain][getElemMatParams]") {
    DomainData dd;
    dd._mateParams = { {{"E", 1.0}}, {{"E", 2.0}} };  // 材料0=E1，材料1=E2
    dd._eleMateIds = {0};                               // else 路径：eleID0 -> 材料0(E1)
    PhyFieldData pf(2, &dd);
    ElementBase ele(4, &pf);
    ele._eleMatIDMap[0] = 1;                            // 第一分支：eleID0 -> 材料1(E2)

    const auto& p = dd.getElemMatParams(0, &ele);
    REQUIRE(p.at("E") == 2.0);  // 第一分支返回材料1(E2)；bug 下走 else 返回 E1 -> FAIL
}

// getElemMatParams 第二分支（else）：_eleMatIDMap 不含 eleID，
// 返回 _mateParams[_eleMateIds[eleID]]。bug 前后均通过，守护 else 正确。
TEST_CASE("getElemMatParams: eleMatIDMap 不含 eleID 走 else", "[domain][getElemMatParams]") {
    DomainData dd;
    dd._mateParams = { {{"E", 1.0}}, {{"E", 2.0}} };
    dd._eleMateIds = {1};  // eleID0 -> 材料1(E2)
    PhyFieldData pf(2, &dd);
    ElementBase ele(4, &pf);
    // _eleMatIDMap 空（不含 0）-> else 分支

    const auto& p = dd.getElemMatParams(0, &ele);
    REQUIRE(p.at("E") == 2.0);  // _mateParams[_eleMateIds[0]=1] = E2
}
