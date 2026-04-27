#ifndef SHAPE_FUN_H
#define SHAPE_FUN_H
#include <vector>
#include <array>
#include <functional>
#include "EquationSystem.h"
#include "CDFEG.h"
typedef std::function<std::vector<double>(std::vector<double>&)> TranFun;
namespace CDFEG {
	extern std::vector<std::vector<std::vector<double>>> addArrays;

	/**
	 * @brief 计算行列式
	 * @param matrix 矩阵
	 * @return 行列式的值
	 * @author Xie Wenxi
	 * @date 2025-3-17
	 */
	CDFEG_API double determinant(const std::vector<std::vector<double>>& matrix);

	/**
	 * @brief 计算逆矩阵
	 * @param matrix 原矩阵（输入变量）
	 * @param inv 逆矩阵（输出变量）
	 * @return 矩阵行列式的值
	 * @author Xie Wenxi
	 * @date 2025-3-17
	 */
	CDFEG_API double inverse(const std::vector<std::vector<double>>& matrix, std::vector<std::vector<double>>& inv);

	/**
	 * @brief 矩阵转置
	 * @param matrix 原矩阵 (m×n)
	 * @return 转置后的矩阵 (n×m)
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API std::vector<std::vector<double>> transpose(const std::vector<std::vector<double>>& matrix);

	/**
	 * @brief 矩阵乘法 C = A * B
	 * @param A 矩阵 A (m×n)
	 * @param B 矩阵 B (n×p)
	 * @return 结果矩阵 C (m×p)
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API std::vector<std::vector<double>> multiply(
		const std::vector<std::vector<double>>& A,
		const std::vector<std::vector<double>>& B
	);

	/**
	 * @brief 矩阵向量乘法 y = A * x
	 * @param A 矩阵 A (m×n)
	 * @param x 向量 x (n)
	 * @return 结果向量 y (m)
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API std::vector<double> multiply(
		const std::vector<std::vector<double>>& A,
		const std::vector<double>& x
	);

	/**
	 * @brief 向量外积（叉积）result = a × b
	 * @param a 向量 a (3维)
	 * @param b 向量 b (3维)
	 * @return 结果向量 result (3维)
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API std::vector<double> crossProduct(
		const std::vector<double>& a,
		const std::vector<double>& b
	);

	/**
	 * @brief 向量点积（内积）result = a · b
	 * @param a 向量 a
	 * @param b 向量 b
	 * @return 标量结果
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API double dotProduct(
		const std::vector<double>& a,
		const std::vector<double>& b
	);

	/**
	 * @brief 向量加法 result = a + b
	 * @param a 向量 a
	 * @param b 向量 b
	 * @return 结果向量
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API std::vector<double> add(
		const std::vector<double>& a,
		const std::vector<double>& b
	);

	/**
	 * @brief 向量减法 result = a - b
	 * @param a 向量 a
	 * @param b 向量 b
	 * @return 结果向量
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API std::vector<double> subtract(
		const std::vector<double>& a,
		const std::vector<double>& b
	);

	/**
	 * @brief 向量数乘 result = scalar * a
	 * @param a 向量 a
	 * @param scalar 标量
	 * @return 结果向量
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API std::vector<double> scalarMultiply(
		const std::vector<double>& a,
		double scalar
	);

	/**
	 * @brief 向量范数（长度）
	 * @param a 向量 a
	 * @return 向量的2-范数
	 * @author Xie Wenxi
	 * @date 2025-1-19
	 */
	CDFEG_API double norm(const std::vector<double>& a);
	/**
	 * @brief 计算向量2D方向
	 * @param r 向量坐标值
	 * @return cos和sin
	 * @author Xie Wenxi
	 * @date 2026-4-26
	 */
	CDFEG_API std::array<double, 3> calcDir2D(std::vector<double>&r );
	/**
	 * @brief 计算向量3D方向
	 * @param r 向量坐标值
	 * @return 三个方向的cos值
	 * @author Xie Wenxi
	 * @date 2026-4-26
	 */
	CDFEG_API std::array<double, 4> calcDir3D(std::vector<double>& r);
	/**
 * @brief 计算2D方向（2点）
 * @param r 2点坐标值
 * @return cos和sin
 * @author Xie Wenxi
 * @date 2026-4-26
 */
	CDFEG_API std::array<double, 3> calcDir2D2(const std::vector<double>& r);
	/**
	 * @brief 计算3D方向(2点)
	 * @param r 2点坐标值
	 * @return 三个方向的cos值
	 * @author Xie Wenxi
	 * @date 2026-4-26
	 */
	CDFEG_API std::array<double, 4> calcDir3D2(const std::vector<double>& r);
}
#endif
