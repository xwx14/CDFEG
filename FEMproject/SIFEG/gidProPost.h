#ifndef GID_PROPOST_H
#define GID_PROPOST_H
#include "SIFEG.h"
#include "Processor.h"
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include "TextReader.h"
/**
 * @class GidProPost
 * @brief Gid前后处理器
 * 该类用于处理gid的网格文件，以及输出结果文件。
 * 用于新式文件
 * @author xwx
 * @date 2025-3-27
 */
namespace SIFEG {
	class EleSubBase;
	class SIFEG_API GidProPost :public Processor
	{
	public:
		GidProPost(FEMData* data);
		~GidProPost();
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
		std::vector<EleSubBase*> _mshOutEle;
		std::map< EleSubBase*, int> _matStartID2;
		std::vector<EleSubBase*> _mshOutEle2;
	};
}
#endif
