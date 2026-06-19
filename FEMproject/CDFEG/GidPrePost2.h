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
#ifndef GID_PREPOST2_H
#define GID_PREPOST2_H
// 将用于读取旧文件格式的GidPrePost类
#include "Processor.h"
#include <string>
#include <vector>
#include <map>
namespace CDFEG{
	class ElementBase;
	class GidPrePost2 :
		public Processor
	{
	public:
		GidPrePost2(FEMData* data);
		~GidPrePost2();
		virtual int pre();
		virtual int post(int it = 0);
		bool readMatFile();
		bool readDatFile();
	public:
		//材料信息文件
		std::string _matFn;
		// 时间文件
		std::string _timeFn = "time0";
		// 网格文件
		std::string _datFn;
		// 后处理网格文件
		std::string _gidMshFn;
		// 后处理结果文件
		std::string _gidResFn;
		std::vector<std::vector<int>> _matStartID;
		std::vector<ElementBase*> _mshOutEle;
		std::map< ElementBase*, int> _matStartID2;
		std::vector<ElementBase*> _mshOutEle2;
	};
}


#endif