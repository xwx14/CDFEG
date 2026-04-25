#ifndef SI_MATRIX_H
#define SI_MATRIX_H
#include <vector>
#include <map>
#include <array>
#include <Eigen/Sparse>
#include "SIFEG.h"
#include <set>
typedef Eigen::SparseMatrix<double, Eigen::RowMajor> SpMat;
namespace  SIFEG {

	// 矩阵类
	class SIFEG_API EquationSystem {
	public:
		EquationSystem() {};
		~EquationSystem() {};
		/**
		 * @brief 初始化矩阵
		 * @param mht 矩阵行指示
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int init(std::vector<std::set<int>>& mht);
		/**
		 * @brief 获取矩阵行数
		 * @param
		 * @param
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int getNRows()const { return _colMap.size(); };
		/**
		 * @brief 将矩阵转换为Eigen矩阵
		 * @return 转换后的矩阵
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		SpMat& convert2Eigen();
		/**
		 * @brief 求解矩阵
		 * @param f 右端项(非齐次项)
		 * @param x 求解结果
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		int solve();
		/*!
		 * @brief 将单元刚度矩阵添加至总刚矩阵
		 * @param StifMat 总刚矩阵
		 * @param estifn 单元刚度矩阵
		 * @param equIds 单元节点id
		 * author xwx14
		 * date 2025/03/07
		 */
		void adda(const std::vector <double>& estifn, const std::vector<int>& equIds);
		/**
		 * @brief 使用划行列法添加一类边界条件
		 * @param 等式号
		 * @param 边界值
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-12-29
		 */
		void addFirstBC(int equId, double val);

		void addSecondBC(int equId, double val);
		/**
		 * @brief 批量应用第一类边界条件
		 * @param bc1s 第一类边界条件 <dofId, val>
		 * @param ida 自由度序号到方程编号的映射
		 * @author Xie Wenxi
		 * @date 2026-1-22
		 */
		void applyFirstBCs(const std::map<int, double>& bc1s, const std::vector<int>& ida);
		/**
		 * @brief 批量应用第二类边界条件
		 * @param bc2s 第二类边界条件 <dofId, val>
		 * @param ida 自由度序号到方程编号的映射
		 * @author Xie Wenxi
		 * @date 2026-1-22
		 */
		void applySecondBCs(const std::map<int, double>& bc2s, const std::vector<int>& ida);
		/*!
		* @brief 计算右值
		* author xwx14
		* date 2025/03/07
		*/
		int calRightVals();
	public:
		// 矩阵行指示(每行开始的元素在_data中的序号，最后一个元素为非零元个数)
		std::vector<int> _numCol;
		// 矩阵非零元所在列号
		std::vector<int> _colId;
		//矩阵非零元数值
		std::vector< double> _data;
		// 未经划行列处理的数值
		std::vector< double> _data0;
		// 每行中，列号到非零元序号的映射
		std::vector <std::map<int, int>> _colMap;
		// 是否已经转换成Eigen矩阵
		bool _bEigenConverted = false;
		// Eigen矩阵
		SpMat _eigenMat;
		// 右值
		std::vector<double> _f;
		// 计算结果
		std::vector<double> _rhs;
		bool _bSavedData0 = false;
		// 未经划行列处理的右端项
		std::vector<double> _f0;
		// 已应用的一类边界条件 <equId, val>
		std::map<int, double> _firstBCMap;
		// 未经划行列的矩阵右端项(非齐次项)
		std::vector<double> _rightVals;
	};
}
#endif
