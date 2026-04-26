#ifndef PHY_DATA_H
#define PHY_DATA_H
#include <vector>
#include <string>
#include <map>
#include "SIFEG.h"
#include "EquationSystem.h"
#define DOF_ID(nodeId, iDof) ((nodeId)*_dof+(iDof))

namespace SIFEG {
	class EleSubBase;
	class FEMData;
	class SIFEG_API  PhyFieldData {
	public:
		PhyFieldData(int dof, FEMData* femData);
		virtual ~PhyFieldData();
		/**
		 * @brief E程序，计算总刚矩阵和右端项
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		virtual int eProgram() { return -1; };
		/**
		 * @brief 求解线性椭圆方程的算法
		 * @author Xie Wenxi
		 * @date 2025-3-21
		 */
		int eProgram_el();

		std::map<std::string, std::vector<double>>  getCoef1(std::vector<int> nodeIds);
		std::map<std::string, std::vector<double>>  getCoef(std::vector<int> nodeIds);
		/**
		 * @brief 设置节点个数，并初始化相关数值
		 * @param n 节点个数
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void setNPts(int n);
		/**
		 * @brief 设置有限元空间数据
		 * @param femData 有限元空间数据
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void setFEMData(FEMData* femData) { _femData = femData; };
		/**
		 * @brief 添加单元类型
		 * @param eleSub 单元类型
		 * @return 此单元的序号
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int addEleSub(EleSubBase* eleSub);
		
		void addBoundary(int nodeId,int typeId, double val = 0, int iDof = 0);
		/**
		 * @brief 设置第一类边界条件
		 * @param nodeId 节点ID
		 * @param val 边界值
		 * @param iDof 自由度序号
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		void setFirstBoundry(int nodeId, double val = 0, int iDof = 0);

		void setSecondBoundry(int nodeId, double val = 0, int iDof = 0);
		/**
		 * @brief 求解矩阵
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int solve();
		/*!
		 * @brief 根据计算结果，给_unoda赋值
		 * author xwx14
		 * date 2025/03/07
		 */
		virtual int uPhy();

		/*!
		 * @brief 根据边界条件及单元初始化矩阵
		 * author xwx14
		 * date 2025/03/06
		 */
		int initMatrix();
		/**
		 * @brief 获取
		 * @param
		 * @param
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-3-31
		 */
		std::vector<double> getNodeDisps(const std::vector<int>& nodeIds);
		std::vector<double> getNodeDisps(const std::vector<int>& nodeIds, int iDof);
	public:
		std::string _name;
		std::string _resForm;
		std::vector<std::string> _dispNames;
		std::vector<std::string> _nodeResNames;
		std::vector<std::string> _eleResNames;
		// 总自由度数，节点数*场自由度数
		int _kVar;
		// 第一类边界条件 <dofId, val>
		std::map<int, double> _nodeBC1s;
		// 第二类边界条件 <dofId, val>
		std::map<int, double> _nodeBC2s;
		// 关联的场
		std::vector<PhyFieldData*> _assPhys;
		// 有限元空间数据
		FEMData* _femData = nullptr;
		// 单元子程序
		std::vector<SIFEG::EleSubBase*> _eleSubs;
		// 节点规格数，start程序后会记录等式号
		//- 1：表示此节点自由度不存在
		// >=0 ：表示具有相同整型数的自由度对应相同的方程号（即代数方程组的同一个未知量）。
		std::vector<int> _ida;
		// 刚度矩阵
		EquationSystem _equSys;
		// 矩阵的右值，由矩阵与位移值相乘所得
		//std::vector<double> _rightVals;

		
		// 节点结果
		std::map<std::string,std::vector<double>> _nodeRes;
		// 单元结果
		std::map<std::string, std::vector<double>> _elemRes;
		// 等式个数
		int _neq = 0;
		// 自由度
		int _dof = 1;
		// 2类边值自由度
		int _dof2 = 1;
	};
}
#endif
