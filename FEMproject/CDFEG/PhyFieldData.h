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
#ifndef PHY_DATA_H
#define PHY_DATA_H
#include <vector>
#include <string>
#include <map>
#include "CDFEG.h"
#include "EquationSystem.h"
#define DOF_ID(nodeId, iDof) ((nodeId)*_dof+(iDof))

namespace CDFEG {
	class ElementBase;
	class DomainData;
	class CDFEG_API  PhyFieldData {
	public:
		PhyFieldData(int dof, DomainData* femData);
		virtual ~PhyFieldData();
		/**
		 * @brief E程序，计算总刚矩阵和右端项
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		virtual int eProgram() { return eProgram_el(); };
		/**
		 * @brief 求解线性椭圆方程的算法
		 * @author Xie Wenxi
		 * @date 2025-3-21
		 */
		int eProgram_el();

		virtual std::map<std::string, std::vector<double>>  getCoef(const std::vector<int>& nodeIds);
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
		void setDomainData(DomainData* femData) { _femData = femData; };
		/**
		 * @brief 添加单元类型
		 * @param eleSub 单元类型
		 * @return 此单元的序号
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int addEleSub(ElementBase* eleSub);
		
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
		// 取某组某参数的值；组或参数不存在、值未读到时返回 0.0
		double getParam(const std::string& group, const std::string& param) const;
	public:
		std::string _name;
		std::string _resForm;
		std::vector<std::string> _dispNames;
		std::vector<std::string> _nodeResNames;
		std::vector<std::string> _eleResNames;
		// 需节点外推的结果名（uEle 经 weight 加权最小二乘外推到 _nodeRes）
		std::vector<std::string> _nodeExtrapNames;
		// uPhy 末尾是否用 _nodeExtrapNames 前 3 个分量算 von Mises
		bool _bVonMises = false;
		// 总自由度数，节点数*场自由度数
		int _kVar;
		// 第一类边界条件 <dofId, val>
		std::map<int, double> _nodeBC1s;
		// 第二类边界条件 <dofId, val>
		std::map<int, double> _nodeBC2s;
		// 关联的场
		std::vector<PhyFieldData*> _assPhys;
		// 有限元空间数据
		DomainData* _femData = nullptr;
		// 单元子程序
		std::vector<CDFEG::ElementBase*> _eleSubs;
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
		// 需要从前处理输入的参数，每组参数第一个为此参数的组名，后面为参数名
		std::vector<std::vector<std::string>> _addParams;
		// 从前处理读回的参数值，key=组名，value 按该组 _addParams 参数名顺序对齐
		std::map<std::string, std::vector<double>> _paramValues;
		// 记录所需的其他场的数据，key为场号，value为场内数据名序列
		std::map<int,std::vector<std::string>> _coefNames;
	protected:
		// 单元结果 + 节点结果加权外推：遍历 _eleSubs×单元调 uEle，
		// eleResult 写 _elemRes；nodeResult 含 "weight" 时按权重最小二乘外推到 _nodeRes
		void extrapolateNodeResults(const std::vector<std::string>& nodeResNames);
		// 由 3 个应力分量算 von Mises 等效应力，写 _nodeRes[outName]
		void computeVonMises(const std::string& sXX, const std::string& sYY,
		                     const std::string& sXY, const std::string& outName = "vonMises");
	};
}
#endif
