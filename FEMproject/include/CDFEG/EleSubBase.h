#ifndef SI_ELE_SUB_H
#define SI_ELE_SUB_H
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <array>
#include "CDFEG.h"
#include <functional>
#include "MatrixFun.h"
#include "vtkCellType.h"
#define H 0.02
#define H2 0.04

namespace CDFEG {
	class FEMData;
	class PhyFieldData;
	// 单元子程序计算结果
	struct EleSubResult
	{
		// 单元刚度矩阵
		std::vector<double> estif;
		// 单元质量矩阵
		std::vector<double> emass;
		// 单元阻尼矩阵
		std::vector<double> edamp;
		// 单元外力向量
		std::vector<double> eload;
		// 单元节点编号
		std::vector<int> nodeIds;
	};
	// 单元后处理结果
	struct uResult {
		std::map<std::string, double> eleResult;
		std::map<std::string, std::vector<double>> nodeResult;
	};
	// 单元类
	class CDFEG_API EleSubBase {
	public:
		EleSubBase(int nNode, PhyFieldData* pData = nullptr);
		virtual~EleSubBase() {};
		// 获取每个单元的节点个数
		int getnNodesPerEle()const { return _nNode; };
		/*!
		 * @brief 单元子程序
		 * @param r 节点坐标
		 * @param coef 系数
		 * @param iMate 材料编号
		 * @return 计算结果
		 * author xwx14
		 * date 2025/03/09
		 */
		virtual EleSubResult& run(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams);

		virtual uResult uEle(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams);

		const std::vector<int>& getEleIds()const { return _eleIds; }

	public:
		std::string _name;
		std::set<std::string> _types;
		// 控制是否保存每个单元的计算结果
		bool _bSaveResult = false;
		// 单元自由度
		std::vector<std::string> _dispNames;
		VTKCellType _vtkCellType;
		// 所属的物理场
		PhyFieldData* _phyData;
		// 所属的有限元空间
		FEMData* _femData;
		// 实时计算结果
		EleSubResult _result;
		// 每个单元的计算结果
		std::vector<EleSubResult> _results;
		// 单元编号
		std::vector<int> _eleIds;
		// 为适应旧版本输入数据而使用
		std::map<int, int> _eleMatIDMap;
		// 单元数量
		int _nElement = 0;
		// 未知量个数
		int _nVar;
		// 全局空间维度
		int _dim;
		// 广义位移个数
		int _nDisp;
		// 节点个数
		int _nNode;
		// 全局坐标维度
		int _nCoor;
		// 单元子程序所需的材料参数名称
		std::vector<std::string> _paramNames;

	};

}
#endif
