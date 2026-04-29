#ifndef ISO_ELE_BASE_H
#define ISO_ELE_BASE_H
#include "ElementBase.h"
namespace CDFEG {
	/**
	 * @class IsoEleBase
	 * @brief 等参单元基类
	 * @author xwx
	 * @date 2025-4-21
	 */
	class CDFEG_API IsoEleBase :public ElementBase {
	public:
		IsoEleBase(int nNode, PhyFieldData* pData = nullptr);
		virtual ~IsoEleBase();

		/**
		 * @brief 将坐标值带代入形函数
		 * @param crtr 坐标雅各比矩阵的逆矩阵
		 * @param cu 输出变量
		 每个节点一行，每一行第一个值为refShapCoef原值，第二个值为对x的偏导，第三个值为对y的偏导
		* @author Xie Wenxi
		* @date 2025-2-12
		*/
		int shapn(int iGause, std::vector<double>& coor, std::vector<std::vector<double>>& crtr, std::vector<std::vector<double>>& cu);
		/**
		 * @brief
		 * @param
		 * @param
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-4-17
		 */
		int shapc(int iGause, int nVal, std::vector<std::vector<double>>& dfdx, std::vector<std::vector<double>>& crtr, std::vector<std::vector<double>>& cu, int dord = 1);
		/**
		 * @brief 坐标转换函数
		 * @param x 节点全局坐标
		 * @param refc 积分点参考坐标
		 * @return 转换后的坐标值
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		virtual std::vector<double> coordTransFun(const std::vector<double>& x, const std::vector<double>& refc);

		/**
		 * @brief 形函数系数计算（由各种单元的类实现）
		 * @param refc 积分点参考坐标
		 * @return 形函数系数
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		virtual std::vector<double> shapeFun(const std::vector<double>& refc) = 0;

		/**
		 * @brief 求坐标雅各比矩阵
		 * @param r 节点坐标
		 * @param iGaus 积分点索引
		 * @param fx 转换后的坐标值
		 * @param dfdx 雅各比矩阵
		 * @return fx中原值的序号
		 * @author Xie Wenxi
		 * @date 2025-2-26
		 */
		virtual int dcoor(const std::vector<double>& r, int iGaus, std::vector<double>& fx, std::vector<std::vector<double>>& dfdx, int dord = 1);
		/*!
		 * @brief 计算系数雅各比矩阵
		 * @param x 参考点
		 * @param iGaus 积分点序号
		 * @param nVal 系数个数
		 * @param fx 转换后的系数数值
		 * @param dord 导数次数
		 * @return 是否成功
		 * author xwx14
		 * date 2025/04/01
		 */
		int dCoef(const std::vector<double>& x, int iGaus, int nVal, std::vector<double>& fx, std::vector<std::vector<double>>& dfdx, int dord = 1);
		/**
		 * @brief 求形函数雅各比矩阵
		 * @param x 参考点
		 * @param fx 转换后的形函数值
		 * @param dfdx 雅各比矩阵
		 * @param dord 导数次数
		 * @return 是否成功
		 *
		 * @author Xie Wenxi
		 * @date 2025-2-26
		 */
		virtual int dshap(int iGaus, std::vector<double>& fx, std::vector<std::vector<double>>& dfdx, int dord = 1);
		/**
		 * @brief 计算形函数系数
		 * @param dim 空间维度
		 * @return
		 * @author Xie Wenxi
		 * @date 2025-2-14
		 */
		int caculateShapeCoef(int dim);
		/**
		 * @brief 转换函数
		 * @param x 节点全局坐标
		 * @param nVal  全局坐标维数
		 * @param iGaus 积分点索引
		 * @param i 偏移索引值，见_refShapCoef下的注释
		 * @return 转换后的值
		 * @author Xie Wenxi
		 * @date 2025-3-17
		 */
		std::vector<double> coordTransFun(const std::vector<double>& x, int nVal, int iGaus, int i);

	public:
		// 导数最大阶数
		int _ndord = 1;
		// 积分点权重
		std::vector<double> _gaus;
		// 积分点个数，默认为节点个数
		int _nGaus;
		// 参考坐标维度（等参单元特有）
		int _nRefc;

		// 积分点坐标，二维为x1,y1,x2,y2...
		std::vector<double> _refc;

		// 在母单元中，积分点上的形函数值(包含偏移过的值)
		//_refShapCoef[iGaus][i][0] 代表第iGaus个积分点的N_i的值
		std::vector<std::vector<std::vector<double>>> _refShapCoef;
		//0代表坐标原值，1代表向正方向偏移H,-1代表向负方向偏移H
		//一维：0:0;1:1;2:-1
		//
		//二维：0:(0,0);1:(0,1);2:(0,-1);
			 // 3:(1,0);4:(1,1);5:(1,-1);
			 // 6:(-1,0);7:(-1,1);8:(-1,-1);
			 //
		//三维：0:(0,0,0);1:(0,0,1);2:(0,0,-1);
			// 3:(0,1,0);4:(0,1,1);5:(0,1,-1);
			// 6:(0,-1,0);7:(0,-1,1);8:(0,-1,-1);
			// 9:(1,0,0);10:(1,0,1);11:(1,0,-1);
			// 12:(1,1,0);13:(1,1,1);14:(1,1,-1);
			// 15:(1,-1,0);16:(-1,1);17:(1,-1,-1);
			// 18:(-1,0,0);19:(-1,0,1);20:(-1,0,-1);
			// 21:(-1,1,0);22:(-1,1,1);23:(-1,1,-1);
			// 24:(-1,-1,0);25:(-1,-1,1);26:(-1,-1,-1)
		int _iPow3;
		std::vector<int> _iPow3s;
		// 每个积分点的形函数雅各比矩阵
		std::vector<std::vector<std::vector<double>>> _dRefShapecoef;
	};
}
#endif
