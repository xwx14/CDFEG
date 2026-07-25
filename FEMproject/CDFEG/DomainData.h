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
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>
// 
#ifndef CDFEG_DOMAIN_DATA_H
#define CDFEG_DOMAIN_DATA_H
#include <vector>
#include <string>
#include <map>
#include "CDFEG.h"
#include "ElementBase.h"
#include "EquationSystem.h"
namespace CDFEG {
	class PhyFieldData;
	class ElementBase;
	class Processor;
	// 有限元空间数据
	class CDFEG_API DomainData {
	public:
		DomainData();
		virtual ~DomainData();
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
		 * @param id 单元id（文件中编号）
		 * @param nodeIds 单元节点id列表
		 * @param eleType 单元类型名（与 ElementBase::_types 匹配）
		 * @return 该单元在程序中的内部索引（供绑定材料等使用）
		 * @note 当 id 已存在但 nodeIds 节点数更少时，按「边单元」处理：
		 *       记录到 _edgeIdMap（边所属体单元），追加节点但不增加 _nElem，
		 *       避免与同 id 的体单元冲突（边 id 即所属体单元 id）。
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int addEle(int id, const std::vector<int>& nodeIds, const std::string& eleType);
		/**
		* @brief 添加edge
		* @param id 单元id
		* @param nodeIds edge节点id列表
		* @return
		* @author Xie Wenxi
		* @date 2025-3-17
		*/
		void addEdge(int id, const std::vector<int>& nodeIds, const std::string& eleType);
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
		 * @brief 通过材料名称设置单元材料号（按程序内部单元索引）
		 * @param internalId 程序内部单元索引（addEle 的返回值）
		 * @param name 材料名称（不含 "mat_" 前缀，如 "DelQ4g_1"）
		 * @note 用于边单元等与体单元共享文件 id 的场景：setEleMateByName 按
		 *       文件 id 经 _eleIdMap 查内部索引，同 id 的边会被误绑到体单元；
		 *       本方法直接用内部索引绑定，体单元/边单元均正确。
		 */
		void setEleMateByInternal(int internalId, const std::string& name);
		/**
		 * @brief 添加材料参数
		 * @param matParam 材料参数
		 * @return 此材料号
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int addMate(const std::map<std::string, double>& matParam,const std::string& name="");
		/**
		 * @brief 注册材料本构类型（参数 schema）
		 * @param name 本构类型名（与材料段头 name 一致，如 "HelQ4g"）
		 * @param params 参数名列表（供 readMate 为材料值命名）
		 * @author xwx14
		 * @date 2026-07-20
		 */
		void addMateType(const std::string& name, const std::vector<std::string>& params);
		/**
		 * @brief 获取单元的材料参数
		 * @param eleID 单元id
		 * @param
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		const std::map<std::string, double>& getElemMatParams(int eleID, ElementBase* ele) const;
		// 计算程序
		virtual int caculate() { return -1; };
		// main程序
		virtual int main() { return -1; };
		// 统一后处理：遍历 _processors 调各自 post(it)
		void post(int it = 0);
		// 取某组某参数的值；组或参数不存在、值未读到时返回 0.0
		double getParam(const std::string& group, const std::string& param) const;
		/*!
		 * @brief 获取coef(其他场的数据)
		 * @param nodeIds 节点id
		 * @param dataNames 数据名称,key为场号，value为场内数据名序列
		 * @return coef
		 * author xwx14
		 * date 2026/07/16
		 */
		std::map<std::string, std::vector<double>>  getCoef(const std::vector<int>& nodeIds,const std::map<int,std::vector<std::string>>& dataNames);
	public:
		int _nPts;
		double _dt = 0.0;
		double _tMax = 0.0;
		// 物理场
		std::vector<PhyFieldData*> _phyDatas;
		// 材料参数
		std::vector<std::map<std::string, double>> _mateParams;
		std::vector<std::string> _mateNames;
		// 材料本构类型表：key=本构类型名，value=参数名列表（readMate 按此为材料值命名）
		std::map<std::string, std::vector<std::string>> _mateConstitutive;
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
		// first为edge单元号，second为edge所在的单元号（均为程序中）
		std::map<int, int> _edgeIdMap;
		// 前后处理
		std::vector<Processor*> _processors;
		// 需要从前处理输入的参数，每组参数第一个为此参数的组名，后面为参数名
		std::vector<std::vector<std::string>> _addParams;
		// 从前处理读回的参数值，key=组名，value 按该组 _addParams 参数名顺序对齐
		std::map<std::string, std::vector<double>> _paramValues;
		
	};
};
#endif
