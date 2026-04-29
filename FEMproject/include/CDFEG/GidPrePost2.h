#ifndef GID_PREPOST2_H
#define GID_PREPOST2_H
// 将用于读取旧文件
#include "Processor.h"
namespace CDFEG{
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