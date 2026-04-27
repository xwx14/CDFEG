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
#include "PhyFieldData.h"
#include "FemData.h"
#endif
