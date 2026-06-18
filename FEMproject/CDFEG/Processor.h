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
// 
// 
#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <string>
#include "CDFEG.h"
/**
 * @class Processor
 * @brief 前后处理器基类
 * @author xwx
 * @date 2025-3-27
 */
namespace CDFEG {
	class FEMData;
	class PhyFieldData;
	class CDFEG_API Processor
	{
	public:
		Processor(FEMData* data, PhyFieldData* fieldData);
		virtual~Processor();
		/**
		 * @brief 前处理
		 * @author Xie Wenxi
		 * @date 2025-3-27
		 */
		virtual int pre();
		/**
		 * @brief 后处理
		 * @author Xie Wenxi
		 * @date 2025-3-27
		 */
		virtual int post(int it = 0);

	public:
		FEMData* _femData;
		PhyFieldData* _phyFieldData;
		bool _bNeedTime = false;
		int _nPts = 0;
		int _nEles = 0;
	};
}

#endif
