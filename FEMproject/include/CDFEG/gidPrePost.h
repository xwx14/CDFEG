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
#include "CDFEG.h"
#include "Processor.h"
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include "TextReader.h"
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
		int readBaseData(const std::string& line);
		int readCoord(const std::map<std::string, std::string>& params);
		int readElement(const std::map<std::string, std::string>& params);
		int readID(const std::map<std::string, std::string>& params);
		int readUBF(const std::map<std::string, std::string>& params);
		int gidMsh();
		int writeNodes(std::ofstream& outFile, int dim);
		virtual int post(int it = 0);
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
	};
}
#endif
