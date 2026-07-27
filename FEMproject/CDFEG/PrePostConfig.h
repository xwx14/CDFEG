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
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.
#ifndef PRE_POST_CONFIG_H
#define PRE_POST_CONFIG_H
#include <string>
#include <vector>
#include "CDFEG.h"
#include "ResItem.h"
namespace CDFEG {
    // 前后处理配置：由 DomainData 持有，所有 Processor 共享遍历
    class CDFEG_API PrePostConfig
    {
    public:
        // 节点结果项（OnNodes，post 取各物理场 _nodeRes）
        std::vector<ResItem> _nodeResItems;
        // 单元结果项（OnElements，post 取各物理场 _elemRes）
        std::vector<ResItem> _eleResItems;
        // GiD 结果 analysis 名（gidPrePost 写 Result 头用，默认沿用原硬编码值）
        std::string _analysisName = "Load Analysis";
        // 预留：processor 是否需要时间信息（当前未启用）
        bool _bNeedTime = false;
    };
}
#endif
