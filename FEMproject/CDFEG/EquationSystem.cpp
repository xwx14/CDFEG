#include "EquationSystem.h"
#include<Eigen/SparseCholesky>
#include <Eigen/Sparse>
#include <vector>


int CDFEG::EquationSystem::init(std::vector<std::set<int>>& mht)
{
	int nEq = mht.size();
	_f.resize(nEq);
	_colMap.resize(nEq);
	int i = 0;
	for (std::set<int>& eqSet : mht)
	{
		_numCol.push_back(_colId.size());
		for (int iEq : eqSet)
		{
			_colMap[i][iEq] = _colId.size();
			_colId.push_back(iEq);
		}
		++i;
	}
	_data.resize(_colId.size());
	_numCol.push_back(_colId.size());
	return 1;
}

Eigen::SparseMatrix<double, Eigen::RowMajor>& CDFEG::EquationSystem::convert2Eigen()
{
	if (_bEigenConverted)return _eigenMat;
	// 确定矩阵维度（需已知列数n）
	int m = _numCol.size() - 1;     // 行数
	int n = m;

	// 扩展行指针数组以符合CSR格式
	//std::vector<int> rowPtr(_numCol.begin(), _numCol.end());
	//rowPtr.push_back(_data.size());

	// 映射到Eigen的行优先稀疏矩阵
	_eigenMat =
		Eigen::Map<Eigen::SparseMatrix<double, Eigen::RowMajor>>(
			m,                  // 行数
			n,                  // 列数
			_data.size(),       // 非零元素数
			_numCol.data(),      // 调整后的行指针数组
			_colId.data(),      // 列索引数组
			_data.data()        // 数据数组
		);
	_bEigenConverted = true;
	return _eigenMat;
}

int CDFEG::EquationSystem::solve()
{
	int n = _f.size();
	convert2Eigen();
	Eigen::SimplicialLDLT<SpMat> chol(_eigenMat);
	Eigen::VectorXd b = Eigen::Map<Eigen::VectorXd>(_f.data(), n);
	Eigen::VectorXd solve1 = chol.solve(b);
	std::vector<double> a;
	_rhs = std::vector<double>(&solve1[0], solve1.data() + solve1.cols() * solve1.rows());
	return 1;
}

void CDFEG::EquationSystem::adda(const std::vector <double>& estifn, const std::vector<int>& equIds)
{
	int n0, n1;
	int nd = equIds.size();
	for (int i = 0; i < nd; ++i)
	{
		int equId1 = equIds[i];
		if (equId1 < 0)continue;
		n0 = _numCol[equId1];
		n1 = _numCol[equId1 + 1];
		for (int j = 0; j < nd; ++j)
		{
			int equId2 = equIds[j];
			if (equId2 < 0)continue;
			int e = _colMap[equId1][equId2];
			_data[e] += estifn[j * nd + i];
		}
	}
}

void CDFEG::EquationSystem::addFirstBC(int equId, double val)
{
	// 检查equId是否有效
	if (equId < 0 || equId >= _numCol.size() - 1) return;
	
	// 首次调用时保存原始数据
	if (!_bSavedData0) {
		_data0 = _data;
		_f0 = _f;
		_bSavedData0 = true;
	}
	
	// 记录/更新边界条件
	_firstBCMap[equId] = val;
	
	// 从原始值恢复
	_data = _data0;
	_f = _f0;
	
	// 重新应用所有边界条件
	int numRows = _numCol.size() - 1;
	for (auto& bc : _firstBCMap)
	{
		int bcEquId = bc.first;
		double bcVal = bc.second;
		
		// 1. 处理bcEquId行：对角元素设为1，非对角元素设为0
		int n0 = _numCol[bcEquId];
		int n1 = _numCol[bcEquId + 1];
		for (int k = n0; k < n1; ++k)
		{
			int col = _colId[k];
			if (col == bcEquId)
				_data[k] = 1.0;
			else
				_data[k] = 0.0;
		}
		
		// 2. 处理其他行的bcEquId列元素
		for (int i = 0; i < numRows; ++i)
		{
			if (_firstBCMap.count(i)) continue; // 跳过边界条件行
			
			int rowStart = _numCol[i];
			int rowEnd = _numCol[i + 1];
			for (int k = rowStart; k < rowEnd; ++k)
			{
				if (_colId[k] == bcEquId)
				{
					double a_ij = _data0[k];
					_data[k] = 0.0;
					_f[i] -= a_ij * bcVal;
				}
			}
		}
		
		// 3. 修改右端项
		_f[bcEquId] = bcVal;
	}
	
	// 4. 标记需要重新转换Eigen矩阵
	_bEigenConverted = false;
}

void CDFEG::EquationSystem::addSecondBC(int equId, double val)
{
	// 检查equId是否有效
	if (equId < 0 || equId >= _f.size()) return;

	// 二类边界条件：直接在右端项上添加节点力
	_f[equId] += val;
}

void CDFEG::EquationSystem::applyFirstBCs(const std::map<int, double>& bc1s, const std::vector<int>& ida)
{
	if (bc1s.empty()) return;
	
	// 首次调用时保存原始数据
	if (!_bSavedData0) {
		_data0 = _data;
		_f0 = _f;
		_bSavedData0 = true;
	}
	
	// 将dofId转换为equId，更新边界条件记录
	_firstBCMap.clear();
	for (auto& bc : bc1s)
	{
		int dofId = bc.first;
		double val = bc.second;
		if (dofId >= 0 && dofId < ida.size())
		{
			int equId = ida[dofId];
			if (equId >= 0) // equId < 0 表示此自由度不存在
				_firstBCMap[equId] = val;
		}
	}
	
	// 从原始值恢复
	_data = _data0;
	_f = _f0;
	
	// 应用所有第一类边界条件
	int numRows = _numCol.size() - 1;
	for (auto& bc : _firstBCMap)
	{
		int bcEquId = bc.first;
		double bcVal = bc.second;
		
		// 检查equId是否有效
		if (bcEquId < 0 || bcEquId >= numRows) continue;
		
		// 1. 处理bcEquId行：对角元素设为1，非对角元素设为0
		int n0 = _numCol[bcEquId];
		int n1 = _numCol[bcEquId + 1];
		for (int k = n0; k < n1; ++k)
		{
			int col = _colId[k];
			if (col == bcEquId)
				_data[k] = 1.0;
			else
				_data[k] = 0.0;
		}
		
		// 2. 处理其他行的bcEquId列元素
		for (int i = 0; i < numRows; ++i)
		{
			if (_firstBCMap.count(i)) continue; // 跳过边界条件行
			
			int rowStart = _numCol[i];
			int rowEnd = _numCol[i + 1];
			for (int k = rowStart; k < rowEnd; ++k)
			{
				if (_colId[k] == bcEquId)
				{
					double a_ij = _data0[k];
					_data[k] = 0.0;
					_f[i] -= a_ij * bcVal;
				}
			}
		}
		
		// 3. 修改右端项
		_f[bcEquId] = bcVal;
	}
	
	// 标记需要重新转换Eigen矩阵
	_bEigenConverted = false;
}

void CDFEG::EquationSystem::applySecondBCs(const std::map<int, double>& bc2s, const std::vector<int>& ida)
{
	// 批量应用第二类边界条件
	for (auto& bc : bc2s)
	{
		int dofId = bc.first;
		double val = bc.second;
		if (dofId >= 0 && dofId < ida.size())
		{
			int equId = ida[dofId];
			if (equId >= 0 && equId < _f.size())
				_f[equId] += val;
		}
	}
}

int CDFEG::EquationSystem::calRightVals()
{
	std::vector<int>& numCol = _numCol;
	int n = numCol.size() - 1;
	_rightVals.resize(n);
	int s, e;
	for (int i = 0; i < n; ++i)
	{
		s = numCol[i];
		e = numCol[i + 1];
		for (int j = s; j < e; ++j)
		{
			int iCol = _colId[j];
			_rightVals[i] += _data0[j] * _rhs[iCol];
		}
	}
	return 1;
}