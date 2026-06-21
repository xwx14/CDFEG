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

#ifndef DEL2D_DATA_H
#define DEL2D_DATA_H
#include <string>
#include "CDFEG/FemData.h"

namespace CDFEG {
	class GidPrePost;
}

// 二维弹性动力学 Newmark-β 数据类
// caculate() 内实现时间步主循环：每步 eProgram(有效矩阵+载荷) → solve → uPhy(更新u/v/w历史+应力)
class del2dData : public CDFEG::FEMData {
public:
    del2dData();
    ~del2dData();
    virtual int caculate() override;
    virtual int main() override;

    // 设置后处理器与结果项，每步按 it 输出 GiD 结果
    void setPost(CDFEG::GidPrePost* prePost);
private:
    CDFEG::GidPrePost* _prePost = nullptr;
};

#endif
