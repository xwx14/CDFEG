#ifndef SIFEG_FEM_DATA_H
#define SIFEG_FEM_DATA_H
#include <vector>
#include <string>
#include "SIFEG.h"
#include "EleSubBase.h"
#include "EquationSystem.h"

namespace SIFEG {
	class PhyFieldData;
	class EleSubBase;
	// 有限元空间数据
	class SIFEG_API FEMData {
	public:
		FEMData();
		virtual ~FEMData();
		/**
		 * @brief 设置网格点的个数
		 * @param n 网格点个数
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void setNPts(int n);
		/**
		 * @brief 添加节点
		 * @param id 节点id
		 * @param x x坐标
		 * @param y y坐标
		 * @param z z坐标
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void addNode(int id, double x, double y = 0.0, double z = 0.0);
		/**
		 * @brief 节点添加结束后，调用此函数
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void addNodeEnd();
		/**
		 * @brief 添加单元
		 * @param id 单元id
		 * @param nodeIds 单元节点id列表
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void addEle(int id, const std::vector<int>& nodeIds, const std::string& eleType);
		/**
		 * @brief 设置单元材料号
		 * @param eleId 单元id
		 * @param id 材料号
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void setEleMateId(int eleId, int id);
		/**
		 * @brief 通过材料名称设置单元材料号
		 * @param eleId 单元id
		 * @param name 材料名称
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void setEleMateByName(int eleId, const std::string& name);
		/**
		 * @brief 添加材料参数
		 * @param matParam 材料参数
		 * @return 此材料号
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int addMate(const std::map<std::string, double>& matParam,const std::string& name="");
		/**
		 * @brief 获取单元的材料参数
		 * @param eleID 单元id
		 * @param
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		const std::map<std::string, double>& getElemMatParams(int eleID, EleSubBase* ele) const;
		// 计算程序
		virtual int caculate() { return -1; };
		// main程序
		virtual int main() { return -1; };
	public:
		int _nPts;
		double _dt = 0.0;
		double _tMax = 0.0;
		// 物理场
		std::vector<PhyFieldData*> _phyDatas;
		// 材料参数
		std::vector<std::map<std::string, double>> _mateParams;
		std::vector<std::string> _mateNames;
		// 记录每个单元的材料号
		std::vector<int> _eleMateIds;
		// 维度
		int _dim = 1;
		// 节点坐标
		std::vector<double> _nodes;
		// 记录每个单元的节点号（程序中的节点号）
		std::vector<int> _eleNodes;
		// 记录每种单元的vtk类型
		std::vector<VTKCellType> _eleTypes;
		// 记录每种单元的数量
		std::vector<int> _nEles;
		// 每个单元初始的节点号在_eleNodes的位置，最后一个值为_eleNodes的长度，此序列长度为单元数+1
		std::vector<int> _elePt;
		// first为文件中节点号，second为程序中节点号
		std::map<int, int> _nodeIdMap;
		// 总单元数
		int _nElem = 0;
		// first为文件中单元号，second为程序中单元号
		std::map<int, int> _eleIdMap;

	};
};
#endif
