#include "IsoEleBase.h"
#include "PhyFieldData.h"
#include "FemData.h"

CDFEG::IsoEleBase::IsoEleBase(int nNode, PhyFieldData* pData /*= nullptr*/) :ElementBase(nNode, pData)
{

}

CDFEG::IsoEleBase::~IsoEleBase()
{

}

int CDFEG::IsoEleBase::shapn(int iGause, std::vector<double>& coor, std::vector<std::vector<double>>& crtr, std::vector<std::vector<double>>& cu)
{
	cu.resize(_nNode);
	for (int i = 0; i < _nNode; ++i)
	{
		cu[i].push_back(_refShapCoef[iGause][0][i]);
		for (int j = 0; j < _dim; ++j)
		{
			double c = 0.0;
			for (int k = 0; k < _nRefc; ++k)
			{
				c += _dRefShapecoef[iGause][i][k] * crtr[k][j];
			}
			cu[i].push_back(c);
		}
		// todo:实现当偏导数大于1时
			//if()
	}
	return 1;
}

int CDFEG::IsoEleBase::shapc(int iGause, int nVal, std::vector<std::vector<double>>& dfdx, std::vector<std::vector<double>>& crtr, std::vector<std::vector<double>>& cu, int dord)
{

	cu.resize(nVal);
	for (int i = 0; i < nVal; ++i)
	{
		cu[i].push_back(dfdx[0][i]);
		for (int j = 0; j < _dim; ++j)
		{
			double c = 0.0;
			for (int k = 0; k < _nRefc; ++k)
			{
				c += dfdx[i][k] * crtr[k][j];
			}
			cu[i].push_back(c);
		}
		// todo:实现当偏导数大于1时
			//if()
	}
	return 1;
}

std::vector<double> CDFEG::IsoEleBase::coordTransFun(const std::vector<double>& _coorr, const std::vector<double>& refc)
{
	std::vector<double> globalCoords(2, 0.0);
	std::vector<double> N = shapeFun(refc);
	for (int i = 0; i < _nNode; ++i) {
		globalCoords[0] += N[i] * _coorr[2 * i];
		globalCoords[1] += N[i] * _coorr[2 * i + 1];
	}
	return globalCoords;
}

int CDFEG::IsoEleBase::dcoor(const std::vector<double>& r, int iGaus, std::vector<double>& fx, std::vector<std::vector<double>>& dfdx, int dord)
{
	int n = _nRefc;
	// 求得积分点全局坐标x(rx,ry),y(rx,ry)
	fx = coordTransFun(r, _dim, iGaus, 0);
	int m = fx.size();
	//用于保存中间差分函数值，便于重复利用
	std::vector<std::vector<double>> fxPlus1;
	std::vector<std::vector<double>> fxMinus1;
	// 初始化雅各比矩阵
	// dfdx为雅各比矩阵，格式如下：
	//dx/drx  dx/ry dx/drz
	//dy/drx  dy/dry dy/drz
	//dz/drx  dz/dry dz/drz
	dfdx.resize(m, std::vector<double>(n, 0.0));
	int iShapCoef;
	std::vector<int> pow3s;
	// 求雅各比矩阵
	for (int iRefc = 0; iRefc < _nRefc; ++iRefc)
	{
		iShapCoef = _iPow3s[iRefc];
		// 右向差分x(rx+h,ry),y(rx+h,ry)
		std::vector<double> fxPlus = coordTransFun(r, _dim, iGaus, iShapCoef);
		iShapCoef += _iPow3s[iRefc];
		// 左向差分x(rx-h,ry),y(rx-h,ry)
		std::vector<double> fxMinus = coordTransFun(r, _dim, iGaus, iShapCoef);
		for (int iDim = 0; iDim < m; ++iDim) {
			// dx/drx=[x(rx+h,ry)-x(rx-h,ry)]/2h
			dfdx[iDim][iRefc] = (fxPlus[iDim] - fxMinus[iDim]) / H2;
		}
		if (dord > 1)
		{
			fxPlus1.push_back(fxPlus);
			fxMinus1.push_back(fxMinus);
		}
	}
	if (dord == 1)return 0;
	//黑塞矩阵放在dfdx的后半段
	dfdx.resize(m * 2.0, std::vector<double>(n * n, 0.0));
	//Hf为H的平方
	double Hf = H * H;
	for (int i = 0; i < n; ++i) {
		std::vector<double>& fx_plus_h = fxPlus1[i];
		std::vector<double>& fx_minus_h = fxMinus1[i];
		for (int j = 0; j <= i; ++j) { // 只计算下三角部分（对称矩阵）
			if (i == j) {
				// 计算对角元素（二阶偏导）
				for (size_t k = 0; k < m; ++k) {
					dfdx[k + m][i * n + j] = (fx_plus_h[k] - 2.0 * fx[k] + fx_minus_h[k]) / Hf;
				}
			}
			else {
				iShapCoef = 0;
				// 计算非对角元素（混合偏导）
				// xi=xi0+h,xj=xj0+h
				iShapCoef += _iPow3s[i];
				iShapCoef += _iPow3s[j];
				std::vector<double> f_plus_plus = coordTransFun(r, _dim, iGaus, iShapCoef);
				// xi=xi0+h,xj=xj0-h
				iShapCoef += _iPow3s[j];
				std::vector<double> f_plus_minus = coordTransFun(r, _dim, iGaus, iShapCoef);
				// xi=xi0-h,xj=xj0-h
				iShapCoef += _iPow3s[i];
				std::vector<double> f_minus_minus = coordTransFun(r, _dim, iGaus, iShapCoef);
				// xi=xi0-h,xj=xj0+h
				iShapCoef -= _iPow3s[j];
				std::vector<double> f_minus_plus = coordTransFun(r, _dim, iGaus, iShapCoef);
				int i_n = i * n + j;
				int j_n = j * n + i;
				for (size_t k = 0; k < m; ++k) {
					double mixed_derivative = (f_plus_plus[k] - f_plus_minus[k] - f_minus_plus[k] + f_minus_minus[k]) / (4.0 * Hf);
					dfdx[k + m][i_n] = mixed_derivative;
					dfdx[k + m][j_n] = mixed_derivative; // 由于黑塞矩阵的对称性
				}
			}
		}
	}
	return 0;
}

int CDFEG::IsoEleBase::dCoef(const std::vector<double>& x, int iGaus, int nVal, std::vector<double>& fx, std::vector<std::vector<double>>& dfdx, int dord /*= 1*/)
{
	fx = coordTransFun(x, nVal, iGaus, 0);
	int m = fx.size();
	//用于保存中间差分函数值，便于重复利用
	std::vector<std::vector<double>> fxPlus1;
	std::vector<std::vector<double>> fxMinus1;
	// 初始化雅各比矩阵
	// dfdx为雅各比矩阵，格式如下：
	//dCoef1/drx  dCoef1/dry dCoef1/drz
	//dCoef2/drx  dCoef2/dry dCoef2/drz
	//dCoef3/drx  dCoef3/dry dCoef3/drz
	//dCoef4/drx  dCoef4/dry dCoef4/drz
	//……
	dfdx.resize(m, std::vector<double>(nVal, 0.0));
	int iShapCoef;
	std::vector<int> pow3s;
	// 求雅各比矩阵
	for (int iRefc = 0; iRefc < _nRefc; ++iRefc)
	{
		iShapCoef = _iPow3s[iRefc];
		std::vector<double> fxPlus = coordTransFun(x, nVal, iGaus, iShapCoef);
		iShapCoef += _iPow3s[iRefc];
		std::vector<double> fxMinus = coordTransFun(x, nVal, iGaus, iShapCoef);
		for (int iVal = 0; iVal < nVal; ++iVal) {
			dfdx[iVal][iRefc] = (fxPlus[iVal] - fxMinus[iVal]) / H2;
		}
		if (dord > 1)
		{
			fxPlus1.push_back(fxPlus);
			fxMinus1.push_back(fxMinus);
		}
	}
	if (dord == 1)return 0;
	//黑塞矩阵放在dfdx的后半段
	dfdx.resize(m * 2.0, std::vector<double>(nVal * nVal, 0.0));
	//Hf为H的平方
	double Hf = H * H;
	for (int i = 0; i < nVal; ++i) {
		std::vector<double>& fx_plus_h = fxPlus1[i];
		std::vector<double>& fx_minus_h = fxMinus1[i];
		for (int j = 0; j <= i; ++j) { // 只计算下三角部分（对称矩阵）
			if (i == j) {
				// 计算对角元素（二阶偏导）
				for (size_t k = 0; k < m; ++k) {
					dfdx[k + m][i * nVal + j] = (fx_plus_h[k] - 2.0 * fx[k] + fx_minus_h[k]) / Hf;
				}
			}
			else {
				iShapCoef = 0;
				// 计算非对角元素（混合偏导）
				// xi=xi0+h,xj=xj0+h
				iShapCoef += _iPow3s[i];
				iShapCoef += _iPow3s[j];
				std::vector<double> f_plus_plus = coordTransFun(x, nVal, iGaus, iShapCoef);
				// xi=xi0+h,xj=xj0-h
				iShapCoef += _iPow3s[j];
				std::vector<double> f_plus_minus = coordTransFun(x, nVal, iGaus, iShapCoef);
				// xi=xi0-h,xj=xj0-h
				iShapCoef += _iPow3s[i];
				std::vector<double> f_minus_minus = coordTransFun(x, nVal, iGaus, iShapCoef);
				// xi=xi0-h,xj=xj0+h
				iShapCoef -= _iPow3s[j];
				std::vector<double> f_minus_plus = coordTransFun(x, nVal, iGaus, iShapCoef);
				int i_n = i * nVal + j;
				int j_n = j * nVal + i;
				for (size_t k = 0; k < m; ++k) {
					double mixed_derivative = (f_plus_plus[k] - f_plus_minus[k] - f_minus_plus[k] + f_minus_minus[k]) / (4.0 * Hf);
					dfdx[k + m][i_n] = mixed_derivative;
					dfdx[k + m][j_n] = mixed_derivative; // 由于黑塞矩阵的对称性
				}
			}
		}
	}
	return 0;
}


int CDFEG::IsoEleBase::dshap(int iGaus, std::vector<double>& fx, std::vector<std::vector<double>>& dfdx, int dord)
{
	//int n = x.size();
	//fx = shapeFun(x);
	//int m = fx.size();
	int m = _refShapCoef[iGaus][0].size();
	//用于保存中间差分函数值，便于重复利用
	std::vector<std::vector<double>> fxPlus1;
	std::vector<std::vector<double>> fxMinus1;
	// 初始化雅各比矩阵，格式如下：
	//dN1/drx  dN1/dry dN1/drz……
	//dN2/drx  dN2/dry dN2/drz……
	//dN3/drx  dN3/dy dN3/drz……
	//dN4/drx  dN4/dry dN4/drz……
	//……
	dfdx.resize(m, std::vector<double>(_dim, 0.0));
	int pow3;
	// 求雅各比矩阵
	for (int iDim = 0; iDim < _dim; ++iDim)
	{
		pow3 = _iPow3s[iDim];
		std::vector<double>& fxPlus = _refShapCoef[iGaus][pow3];
		std::vector<double>& fxMinus = _refShapCoef[iGaus][pow3 * 2];
		for (int i = 0; i < m; ++i) {
			dfdx[i][iDim] = (fxPlus[i] - fxMinus[i]) / H2;
		}
		if (dord > 1)
		{
			fxPlus1.push_back(fxPlus);
			fxMinus1.push_back(fxMinus);
		}
	}
	if (dord == 1)return 0;
	//黑塞矩阵放在dfdx的后半段
	dfdx.resize(m * 2.0, std::vector<double>(_dim * _dim, 0.0));
	//Hf为H的平方
	int iShapCoef = 0;
	double Hf = H * H;
	for (int i = 0; i < _dim; ++i) {
		std::vector<double>& fx_plus_h = fxPlus1[i];
		std::vector<double>& fx_minus_h = fxMinus1[i];
		for (int j = 0; j <= i; ++j) { // 只计算下三角部分（对称矩阵）
			if (i == j) {
				// 计算对角元素（二阶偏导）
				for (size_t k = 0; k < m; ++k) {
					dfdx[k + m][i * _dim + j] = (fx_plus_h[k] - 2.0 * fx[k] + fx_minus_h[k]) / Hf;
				}
			}
			else {
				iShapCoef = 0;
				// 计算非对角元素（混合偏导）
				// xi=xi0+h,xj=xj0+h
				iShapCoef += _iPow3s[i];
				iShapCoef += _iPow3s[j];
				std::vector<double>& f_plus_plus = _refShapCoef[iGaus][iShapCoef];
				// xi=xi0+h,xj=xj0-h
				iShapCoef += _iPow3s[j];
				std::vector<double>& f_plus_minus = _refShapCoef[iGaus][iShapCoef];
				iShapCoef += _iPow3s[i];
				std::vector<double>& f_minus_minus = _refShapCoef[iGaus][iShapCoef];
				// xi=xi0-h,xj=xj0+h
				iShapCoef -= _iPow3s[j];
				std::vector<double>& f_minus_plus = _refShapCoef[iGaus][iShapCoef];
				int i_n = i * _dim + j;
				int j_n = j * _dim + i;
				for (size_t k = 0; k < m; ++k) {
					double mixed_derivative = (f_plus_plus[k] - f_plus_minus[k] - f_minus_plus[k] + f_minus_minus[k]) / (4.0 * Hf);
					dfdx[k + m][i_n] = mixed_derivative;
					dfdx[k + m][j_n] = mixed_derivative; // 由于黑塞矩阵的对称性
				}
			}
		}
	}
	return 0;
}

int CDFEG::IsoEleBase::caculateShapeCoef(int dim)
{
	_iPow3 = pow(3, dim);
	int iPow3 = _iPow3;
	for (int i = 0; i < dim; ++i)
	{
		iPow3 /= 3;
		_iPow3s.push_back(iPow3);
	}
	_refShapCoef.resize(_nGaus);
	for (int i = 0; i < _nGaus; ++i)
	{
		std::vector<double> refShapeCoef;
		std::vector<std::vector<double>> dRefShapeCoef;
		std::vector<double> refc;
		for (int j = 0; j < dim; ++j)
		{
			refc.push_back(_refc[dim * i + j]);
		}
		std::vector<std::vector<double>> cur1 = { refc };
		std::vector<std::vector<double>> cur2;
		for (int j = 0; j < dim; ++j)
		{
			for (std::vector<double>& refc1 : cur1)
			{
				cur2.push_back(refc1);
				refc1[j] += H;
				cur2.push_back(refc1);
				refc1[j] -= H2;
				cur2.push_back(refc1);
				refc1[j] += H;
			}
			cur1 = cur2;
			cur2.clear();
		}
		for (std::vector<double>& refc1 : cur1)
		{
			_refShapCoef[i].push_back(shapeFun(refc1));
		}
		dshap(i, refShapeCoef, dRefShapeCoef);
		//_refShapCoef.push_back(refShapeCoef);
		_dRefShapecoef.push_back(dRefShapeCoef);
	}
	return 1;
}

std::vector<double> CDFEG::IsoEleBase::coordTransFun(const std::vector<double>& x, int nVal, int iGaus, int i)
{
	int iVal = -1;
	const std::vector<double>& coef = _refShapCoef[iGaus][i];
	int nCoef = coef.size();
	std::vector<double> rt(nVal, 0.0);
	for (int i = 0; i < nCoef; ++i)
	{
		for (int j = 0; j < nVal; ++j)
		{
			rt[j] += coef[i] * x[++iVal];
		}
	}
	return rt;
}

