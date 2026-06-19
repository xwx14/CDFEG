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
// 
#ifndef GID_PREPOST_H
#define GID_PREPOST_H
#include "GidResItem.h"
#include "Processor.h"
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include "TextReader.h"
#include <utility>
#include <set>
/**
 * @class GidPrePost
 * @brief Gid前后处理器
 * 该类用于处理gid的网格文件，以及输出结果文件。
 * 用于新式文件
 * @author xwx
 * @date 2025-3-27
 */
namespace CDFEG {

	class ElementBase;
	class CDFEG_API GidPrePost :public Processor
	{
	public:
		GidPrePost(FEMData* data);
		~GidPrePost();
		void setFilePath(const std::string& parentPath, const std::string& name);
		virtual int pre();
		int readMate(const std::map<std::string, std::string>& params);
		int readTime(const std::string& line);
		int readPreParams(const std::string& group, const std::string& line);
		void collectPreParamDecls();
		int readBaseData(const std::string& line);
		int readCoord(const std::map<std::string, std::string>& params);
		int readElement(const std::map<std::string, std::string>& params);
		int readID(const std::map<std::string, std::string>& params);
		int readUBF(const std::map<std::string, std::string>& params);
		int gidMsh();
		int writeNodes(std::ofstream& outFile, int dim);
		virtual int post(int it = 0);
		virtual int post2(int it = 0);
	public:
		// 网格文件
		std::string _datFn;
		TextReader _datReader;
		// 后处理网格文件
		std::string _gidMshFn;
		// 后处理结果文件
		std::string _gidResFn;
		std::vector<std::vector<int>> _matStartID;
		std::vector<ElementBase*> _mshOutEle;
		std::map< ElementBase*, int> _matStartID2;
		std::vector<ElementBase*> _mshOutEle2;
		std::vector<GidResItem> _resItems;
		// 额外参数声明表：组名(小写) → {目标层 _paramValues 指针, 该组 _addParams 行(首元素为原始组名)}
		// 注意：持有的 _paramValues 裸指针仅在 pre() 执行期间有效（owner 对象生命周期长于 GidPrePost）；
		//       pre() 结束时会 clear()，避免常驻悬垂指针。
		std::map<std::string, std::pair<std::map<std::string,std::vector<double>>*, std::vector<std::string>>> _preParamDecls;
	};
}
#endif
