// SPDX-License-Identifier: GPL-3.0
#include "third/catch.hpp"
#include "CDFEG/EquationSystem.h"
#include <vector>
#include <set>
#include <map>
#include <cmath>

using namespace CDFEG;

// ---- 辅助：构造指定方程数的满稀疏骨架 mht（每行包含所有列） ----
static std::vector<std::set<int>> makeFullMht(int nEq) {
    std::vector<std::set<int>> mht(nEq);
    for (int i = 0; i < nEq; ++i)
        for (int j = 0; j < nEq; ++j)
            mht[i].insert(j);
    return mht;
}

// ======================================================================
// TEST 1: init 基本结构
// ======================================================================
TEST_CASE("EquationSystem.init 构造满稀疏骨架", "[equ][init]") {
    EquationSystem equ;
    auto mht = makeFullMht(3);
    int ret = equ.init(mht);

    REQUIRE(ret == 1);
    REQUIRE(equ._numCol.size() == 4);        // 3行 + 末尾哨兵 = 4
    REQUIRE(equ._colId.size() == 9);          // 3x3 = 9 个非零元
    REQUIRE(equ._data.size() == 9);
    REQUIRE(equ._f.size() == 3);
    REQUIRE(equ._colMap.size() == 3);

    // _numCol 行指针：每行起始 = 行号*3，末尾 = 总非零元
    REQUIRE(equ._numCol[0] == 0);
    REQUIRE(equ._numCol[1] == 3);
    REQUIRE(equ._numCol[2] == 6);
    REQUIRE(equ._numCol[3] == 9);
}

// ======================================================================
// TEST 2: adda 按行主序累加
// ======================================================================
TEST_CASE("EquationSystem.adda 行主序累加", "[equ][adda]") {
    EquationSystem equ;
    auto mht = makeFullMht(2);
    equ.init(mht);

    // 2x2 单刚，行主序：[[1, 2], [3, 4]]
    std::vector<double> estifn = {1.0, 2.0, 3.0, 4.0};
    std::vector<int> equIds = {0, 1};
    equ.adda(estifn, equIds);

    REQUIRE(equ._data[0] == 1.0);  // (0,0)
    REQUIRE(equ._data[1] == 2.0);  // (0,1)
    REQUIRE(equ._data[2] == 3.0);  // (1,0)
    REQUIRE(equ._data[3] == 4.0);  // (1,1)

    // 累加验证：再加一次 [[1,1],[1,1]]，应变为 [[2,3],[4,5]]
    std::vector<double> estifn2 = {1.0, 1.0, 1.0, 1.0};
    equ.adda(estifn2, equIds);
    REQUIRE(equ._data[0] == 2.0);
    REQUIRE(equ._data[1] == 3.0);
    REQUIRE(equ._data[2] == 4.0);
    REQUIRE(equ._data[3] == 5.0);
}

// ======================================================================
// TEST 3: adda 跳过 equId < 0
// ======================================================================
TEST_CASE("EquationSystem.adda 跳过负方程号", "[equ][adda]") {
    EquationSystem equ;
    auto mht = makeFullMht(2);
    equ.init(mht);

    // equIds 含 -1，对应行/列不应被修改
    std::vector<double> estifn = {1.0, 2.0, 3.0, 4.0};
    std::vector<int> equIds = {0, -1};
    equ.adda(estifn, equIds);

    REQUIRE(equ._data[0] == 1.0);  // (0,0) 写入
    REQUIRE(equ._data[1] == 0.0);  // (0,1) 未被写入（equIds[1]<0 跳过）
    REQUIRE(equ._data[2] == 0.0);  // (1,0) 未被写入（equIds[1]<0 跳过）
    REQUIRE(equ._data[3] == 0.0);  // (1,1) 未被写入
}

// ======================================================================
// TEST 4: addFirstBC 划行列法幂等性
// ======================================================================
TEST_CASE("EquationSystem.addFirstBC 划行列法幂等", "[equ][bc]") {
    EquationSystem equ;
    auto mht = makeFullMht(3);
    equ.init(mht);

    // 总刚: 对称正定 3x3
    // [4 1 1]
    // [1 5 1]
    // [1 1 6]
    std::vector<double> K = {4,1,1, 1,5,1, 1,1,6};
    // 用 equIds={0,1,2} 装配
    std::vector<int> equIds = {0,1,2};
    equ.adda(K, equIds);

    // 右端项
    equ._f = {10.0, 20.0, 30.0};

    // 施加一类边界: equId=1, val=7.0
    equ.addFirstBC(1, 7.0);

    // 快照第一次结果
    std::vector<double> data_after_first = equ._data;
    std::vector<double> f_after_first = equ._f;

    // 再次施加相同边界（幂等性验证）
    equ.addFirstBC(1, 7.0);

    REQUIRE(equ._data == data_after_first);
    REQUIRE(equ._f == f_after_first);
}

// ======================================================================
// TEST 5: addFirstBC 划行列法正确性
// ======================================================================
TEST_CASE("EquationSystem.addFirstBC 划行列法正确性", "[equ][bc]") {
    EquationSystem equ;
    auto mht = makeFullMht(3);
    equ.init(mht);

    // 总刚
    // [4 1 1]
    // [1 5 1]
    // [1 1 6]
    std::vector<double> K = {4,1,1, 1,5,1, 1,1,6};
    equ.adda(K, {0,1,2});

    equ._f = {10.0, 20.0, 30.0};

    // 施加 equId=1, val=7.0
    equ.addFirstBC(1, 7.0);

    // 满矩阵 _colId = {0,1,2, 0,1,2, 0,1,2}
    // 行1(索引3..5): (1,0)=0, (1,1)=1, (1,2)=0
    REQUIRE(equ._data[3] == 0.0);  // (1,0) 清零
    REQUIRE(equ._data[4] == 1.0);  // (1,1) 对角=1
    REQUIRE(equ._data[5] == 0.0);  // (1,2) 清零

    // 行0的列1: _data0[1]=1, _f[0] -= 1*7 => 10-7=3
    REQUIRE(equ._data[1] == 0.0);
    REQUIRE(equ._f[0] == 3.0);

    // 行2的列1: _data0[7]=1, _f[2] -= 1*7 => 30-7=23
    REQUIRE(equ._data[7] == 0.0);
    REQUIRE(equ._f[2] == 23.0);

    // 右端项边界值
    REQUIRE(equ._f[1] == 7.0);
}

// ======================================================================
// TEST 6: _bSavedData0 刷新机制
// ======================================================================
TEST_CASE("EquationSystem._bSavedData0 刷新机制", "[equ][baseline]") {
    EquationSystem equ;
    auto mht = makeFullMht(2);
    equ.init(mht);

    // 总刚 [[2,1],[1,3]]
    equ.adda({2,1, 1,3}, {0,1});
    equ._f = {5.0, 6.0};

    // 第一次施加边界 -> 保存基线
    equ.addFirstBC(0, 1.0);
    REQUIRE(equ._bSavedData0 == true);
    REQUIRE(equ._data0[0] == 2.0);  // 基线保留原始值

    // 修改总刚（模拟动力学每步重装）
    equ._bSavedData0 = false;  // 标记需刷新
    equ._data = {4,2, 2,6};    // 新总刚
    equ._f = {10.0, 12.0};

    // 再次施加边界 -> 应从新总刚保存基线
    equ.addFirstBC(0, 2.0);
    REQUIRE(equ._bSavedData0 == true);
    REQUIRE(equ._data0[0] == 4.0);  // 基线已更新为新值

    // 行0: 对角=1
    REQUIRE(equ._data[0] == 1.0);
    // 右端项
    REQUIRE(equ._f[0] == 2.0);
}

// ======================================================================
// TEST 7: 端到端求解 2x2 已知系统
// ======================================================================
TEST_CASE("EquationSystem 端到端求解 2x2", "[equ][solve]") {
    // 方程组:
    // 2*x + 1*y = 5
    // 1*x + 3*y = 7
    // 解: x=1.6, y=1.8
    EquationSystem equ;
    auto mht = makeFullMht(2);
    equ.init(mht);

    equ.adda({2.0, 1.0, 1.0, 3.0}, {0, 1});
    equ._f = {5.0, 7.0};

    int ret = equ.solve();
    REQUIRE(ret == 1);

    REQUIRE(std::abs(equ._rhs[0] - 1.6) < 1e-10);
    REQUIRE(std::abs(equ._rhs[1] - 1.8) < 1e-10);
}

// ======================================================================
// TEST 8: 带边界条件的端到端求解
// ======================================================================
TEST_CASE("EquationSystem 带边界条件求解", "[equ][solve][bc]") {
    // 方程组:
    // 4*x + 1*y + 1*z = 10
    // 1*x + 5*y + 1*z = 20
    // 1*x + 1*y + 6*z = 30
    // 施加 y=7 (equId=1) 后划行列:
    // 行0: [4, 0, 1], f[0]=10-1*7=3
    // 行1: [0, 1, 0], f[1]=7
    // 行2: [1, 0, 6], f[2]=30-1*7=23
    // 化简: 4x+z=3, y=7, x+6z=23
    // => z=3-4x, x+6(3-4x)=23 => x=-5/23, z=89/23
    EquationSystem equ;
    auto mht = makeFullMht(3);
    equ.init(mht);

    equ.adda({4,1,1, 1,5,1, 1,1,6}, {0,1,2});
    equ._f = {10.0, 20.0, 30.0};
    equ.addFirstBC(1, 7.0);

    int ret = equ.solve();
    REQUIRE(ret == 1);

    REQUIRE(std::abs(equ._rhs[0] - (-5.0/23.0)) < 1e-10);
    REQUIRE(std::abs(equ._rhs[1] - 7.0) < 1e-10);
    REQUIRE(std::abs(equ._rhs[2] - (89.0/23.0)) < 1e-10);
}

// ======================================================================
// TEST 9: calRightVals 节点力/反力
// ======================================================================
TEST_CASE("EquationSystem.calRightVals 节点力计算", "[equ][calright]") {
    // 与 "带边界条件求解" 相同的系统，验证 calRightVals
    EquationSystem equ;
    auto mht = makeFullMht(3);
    equ.init(mht);

    equ.adda({4,1,1, 1,5,1, 1,1,6}, {0,1,2});
    equ._f = {10.0, 20.0, 30.0};
    equ.addFirstBC(1, 7.0);
    equ.solve();
    equ.calRightVals();

    // 节点力 = _data0 * _rhs（原始总刚乘位移解）
    // _data0 = [4,1,1, 1,5,1, 1,1,6]
    // _rhs = [-5/23, 7, 89/23]
    double x = -5.0/23.0, y = 7.0, z = 89.0/23.0;
    double expected0 = 4*x + 1*y + 1*z;  // = 10 (原始 f[0])
    double expected1 = 1*x + 5*y + 1*z;  // = 20 (原始 f[1])
    double expected2 = 1*x + 1*y + 6*z;  // = 30 (原始 f[2])

    REQUIRE(std::abs(equ._rightVals[0] - expected0) < 1e-10);
    REQUIRE(std::abs(equ._rightVals[1] - expected1) < 1e-10);
    REQUIRE(std::abs(equ._rightVals[2] - expected2) < 1e-10);
}

// ======================================================================
// TEST 10: addSecondBC 仅修改右端项
// ======================================================================
TEST_CASE("EquationSystem.addSecondBC 仅修改右端项", "[equ][bc2]") {
    EquationSystem equ;
    auto mht = makeFullMht(2);
    equ.init(mht);

    equ._f = {10.0, 20.0};

    equ.addSecondBC(0, 5.0);
    REQUIRE(equ._f[0] == 15.0);
    REQUIRE(equ._f[1] == 20.0);

    // 累加
    equ.addSecondBC(0, 3.0);
    REQUIRE(equ._f[0] == 18.0);

    // 无效 equId 不崩溃
    equ.addSecondBC(-1, 100.0);
    equ.addSecondBC(99, 100.0);
    REQUIRE(equ._f[1] == 20.0);
}

// ======================================================================
// TEST 11: 稀疏骨架（非满矩阵）adda + solve
// ======================================================================
TEST_CASE("EquationSystem 稀疏三对角矩阵装配", "[equ][sparse]") {
    // 三对角 4x4:
    // [2 1 0 0]
    // [1 2 1 0]
    // [0 1 2 1]
    // [0 0 1 2]
    std::vector<std::set<int>> mht(4);
    mht[0] = {0,1};
    mht[1] = {0,1,2};
    mht[2] = {1,2,3};
    mht[3] = {2,3};

    EquationSystem equ;
    equ.init(mht);

    REQUIRE(equ._data.size() == 10);  // 2+3+3+2=10 非零元
    REQUIRE(equ._numCol.size() == 5);  // 4+1

    // 用 2x2 单刚装配 off-diagonal: [[0,1],[1,0]] 行主序
    // 对角用 1x1 单刚: {2}
    equ.adda({2.0}, {0});
    equ.adda({2.0}, {1});
    equ.adda({2.0}, {2});
    equ.adda({2.0}, {3});
    // off-diagonal: 1x1 单刚值为1，但 adda 要求 estifn 长度=nd^2
    // 用2x2单刚 [[0,1],[1,0]] 装配，对角位置+0不影响
    equ.adda({0.0, 1.0, 1.0, 0.0}, {0, 1});
    equ.adda({0.0, 1.0, 1.0, 0.0}, {1, 2});
    equ.adda({0.0, 1.0, 1.0, 0.0}, {2, 3});

    // 验证装配结果
    REQUIRE(equ._data[equ._colMap[0][0]] == 2.0);  // (0,0)=2
    REQUIRE(equ._data[equ._colMap[0][1]] == 1.0);  // (0,1)=1
    REQUIRE(equ._data[equ._colMap[1][0]] == 1.0);  // (1,0)=1
    REQUIRE(equ._data[equ._colMap[1][1]] == 2.0);  // (1,1)=2
    REQUIRE(equ._data[equ._colMap[1][2]] == 1.0);  // (1,2)=1
    REQUIRE(equ._data[equ._colMap[2][1]] == 1.0);  // (2,1)=1
    REQUIRE(equ._data[equ._colMap[2][2]] == 2.0);  // (2,2)=2
    REQUIRE(equ._data[equ._colMap[2][3]] == 1.0);  // (2,3)=1
    REQUIRE(equ._data[equ._colMap[3][2]] == 1.0);  // (3,2)=1
    REQUIRE(equ._data[equ._colMap[3][3]] == 2.0);  // (3,3)=2
}
