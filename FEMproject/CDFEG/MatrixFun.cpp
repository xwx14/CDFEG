#include "MatrixFun.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#define H 0.02
#define H2 0.04

namespace CDFEG {
	std::vector<std::vector<std::vector<double>>> addArrays(3);

	// 计算行列式
	double determinant(const std::vector<std::vector<double>>& matrix) {
		int n = matrix.size();
		if (n == 1) return matrix[0][0];
		if (n == 0) return 0.0;

		double det = 0;
		for (int col = 0; col < n; ++col) {
			std::vector<std::vector<double>> subMatrix;
			for (int i = 1; i < n; ++i) {
				std::vector<double> row;
				for (int j = 0; j < n; ++j) {
					if (j != col) {
						row.push_back(matrix[i][j]);
					}
				}
				subMatrix.push_back(row);
			}
			det += (col % 2 == 0 ? 1 : -1) * matrix[0][col] * determinant(subMatrix);
		}
		return det;
	}

	double inverse(const std::vector<std::vector<double>>& matrix, std::vector<std::vector<double>>& inv) {
		int n = matrix.size();
		double det = determinant(matrix);
		if (std::abs(det) < 1e-15) {
			throw std::runtime_error("Matrix is singular and cannot be inverted.");
		}
		//计算伴随矩阵的逆矩阵
		std::vector<std::vector<double>> cofactors(n, std::vector<double>(n, 0.0));
		for (int row = 0; row < n; ++row) {
			for (int col = 0; col < n; ++col) {
				std::vector<std::vector<double>> subMatrix;
				for (int i = 0; i < n; ++i) {
					if (i == row) continue;
					std::vector<double> subRow;
					for (int j = 0; j < n; ++j) {
						if (j == col) continue;
						subRow.push_back(matrix[i][j]);
					}
					subMatrix.push_back(subRow);
				}
				cofactors[row][col] = ((row + col) % 2 == 0 ? 1 : -1) * determinant(subMatrix);
			}
		}
		// 计算逆矩阵
		inv.clear();
		inv.resize(n, std::vector<double>(n, 0.0));
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				inv[j][i] = cofactors[i][j] / det;
			}
		}
		return det;
	}

	// 矩阵转置
	std::vector<std::vector<double>> transpose(const std::vector<std::vector<double>>& matrix) {
		if (matrix.empty()) return {};

		int m = matrix.size();      // 行数
		int n = matrix[0].size();   // 列数

		std::vector<std::vector<double>> result(n, std::vector<double>(m));

		for (int i = 0; i < m; ++i) {
			for (int j = 0; j < n; ++j) {
				result[j][i] = matrix[i][j];
			}
		}
		return result;
	}

	// 矩阵乘法 C = A * B
	std::vector<std::vector<double>> multiply(
		const std::vector<std::vector<double>>& A,
		const std::vector<std::vector<double>>& B
	) {
		if (A.empty() || B.empty()) return {};

		int m = A.size();           // A的行数
		int n = A[0].size();        // A的列数 = B的行数
		int p = B[0].size();        // B的列数

		if (n != B.size()) {
			throw std::invalid_argument("Matrix dimensions mismatch for multiplication.");
		}

		std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));

		for (int i = 0; i < m; ++i) {
			for (int j = 0; j < p; ++j) {
				for (int k = 0; k < n; ++k) {
					C[i][j] += A[i][k] * B[k][j];
				}
			}
		}
		return C;
	}

	// 矩阵向量乘法 y = A * x
	std::vector<double> multiply(
		const std::vector<std::vector<double>>& A,
		const std::vector<double>& x
	) {
		if (A.empty() || x.empty()) return {};

		int m = A.size();           // A的行数
		int n = A[0].size();        // A的列数

		if (n != (int)x.size()) {
			throw std::invalid_argument("Matrix-vector dimensions mismatch for multiplication.");
		}

		std::vector<double> y(m, 0.0);

		for (int i = 0; i < m; ++i) {
			for (int j = 0; j < n; ++j) {
				y[i] += A[i][j] * x[j];
			}
		}
		return y;
	}

	// 向量外积（叉积）result = a × b
	std::vector<double> crossProduct(
		const std::vector<double>& a,
		const std::vector<double>& b
	) {
		if (a.size() != 3 || b.size() != 3) {
			throw std::invalid_argument("Cross product requires 3D vectors.");
		}

		return {
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0]
		};
	}

	// 向量点积（内积）result = a · b
	double dotProduct(
		const std::vector<double>& a,
		const std::vector<double>& b
	) {
		if (a.size() != b.size()) {
			throw std::invalid_argument("Vector dimensions must match for dot product.");
		}

		double result = 0.0;
		for (size_t i = 0; i < a.size(); ++i) {
			result += a[i] * b[i];
		}
		return result;
	}

	// 向量加法 result = a + b
	std::vector<double> add(
		const std::vector<double>& a,
		const std::vector<double>& b
	) {
		if (a.size() != b.size()) {
			throw std::invalid_argument("Vector dimensions must match for addition.");
		}

		std::vector<double> result(a.size());
		for (size_t i = 0; i < a.size(); ++i) {
			result[i] = a[i] + b[i];
		}
		return result;
	}

	// 向量减法 result = a - b
	std::vector<double> subtract(
		const std::vector<double>& a,
		const std::vector<double>& b
	) {
		if (a.size() != b.size()) {
			throw std::invalid_argument("Vector dimensions must match for subtraction.");
		}

		std::vector<double> result(a.size());
		for (size_t i = 0; i < a.size(); ++i) {
			result[i] = a[i] - b[i];
		}
		return result;
	}

	// 向量数乘 result = scalar * a
	std::vector<double> scalarMultiply(
		const std::vector<double>& a,
		double scalar
	) {
		std::vector<double> result(a.size());
		for (size_t i = 0; i < a.size(); ++i) {
			result[i] = a[i] * scalar;
		}
		return result;
	}

	// 向量范数（2-范数）
	double norm(const std::vector<double>& a) {
		return std::sqrt(dotProduct(a, a));
	}

	std::array<double, 3> calcDir2D(std::vector<double>& r)
	{
		double L =sqrt( r[0] * r[0] + r[1] * r[1]);
		std::array<double, 3> rt;
		rt[0] = r[0] / L;
		rt[1] = r[1] / L;
		rt[2] = L;
		return rt;
	}

	std::array<double, 4> calcDir3D(std::vector<double>& r)
	{
		double L = sqrt(r[0] * r[0] + r[1] * r[1]+r[2]*r[2]);
		std::array<double, 4> rt;
		rt[0] = r[0] / L;
		rt[1] = r[1] / L;
		rt[2] = r[2] / L;
		rt[3] = L;
		return rt;
	}

	std::array<double, 3> calcDir2D2(const std::vector<double>& r)
	{
		double x = r[2] - r[0];
		double y = r[3] - r[1];
		double L = sqrt(x * x + y * y);
		std::array<double, 3> rt;
		rt[0] = x / L;
		rt[1] = y / L;
		rt[2] = L;
		return rt;
	}

	std::array<double, 4> calcDir3D2(const std::vector<double>& r)
	{
		double x = r[3] - r[0];
		double y = r[4] - r[1];
		double z = r[5] - r[2];
		double L = sqrt(x * x + y * y + z* z);
		std::array<double, 4> rt;
		rt[0] = x / L;
		rt[1] = y / L;
		rt[2] = z / L;
		rt[3] = L;
		return rt;
	}

}
