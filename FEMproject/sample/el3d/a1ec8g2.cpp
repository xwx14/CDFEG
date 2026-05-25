#include "a1ec8g2.h"
#include "el3dData.h"
#include "aFieldData.h"
#include <cmath>

a1ec8g2::a1ec8g2(SIFEG::PhyFieldData* pData)
    : SIFEG::C8G(3, pData) {
    _name="a1ec8g2";
    _dispNames = { "u", "v", "w" };
    _paramNames ={ "pe", "pv", "fu", "fv", "fw", "rou", "alpha" };
    _types.insert("a1ec8g2");
    // TODO: 设置 VTK 单元类型
    // _vtkCellType = VTKCellType::VTK_<TYPE>;

}

a1ec8g2::~a1ec8g2() {

}

SIFEG::EleSubResult& a1ec8g2::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
	std::vector<double> refcoor(_nRefc);
	std::fill(_result.eload.begin(), _result.eload.end(), 0.0);
	std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
	std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
	std::fill(_result.edamp.begin(), _result.edamp.end(), 0.0);
	double pe = matParams.at("pe");
	double pv = matParams.at("pv");
	double fu = matParams.at("fu");
	double fv = matParams.at("fv");
	double fw = matParams.at("fw");
	double rou = matParams.at("rou");
	double alpha = matParams.at("alpha");
	double vol = 1.0;
	double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
	double shear = (0.5 - pv);
	for (int iGaus = 0; iGaus < _nGaus; ++iGaus)
	{
		for (int i = 0; i < _nRefc; ++i)
		{
			refcoor[i] = _refc[_dim * iGaus + i];
		}
		std::vector<std::vector<double>> rctr;
		std::vector<double> coor;
		dcoor(r, iGaus, coor, rctr);
		std::vector<std::vector<double>> crtr;
		double det = SIFEG::inverse(rctr, crtr);
		std::vector<std::vector<double>> cu;
		shapn(iGaus, coor, crtr, cu);
		std::vector<std::vector<double>> cv = cu;
		std::vector<std::vector<double>> cw = cu;
		double weight = _gaus[iGaus] * det;
		std::vector<double> eexx(24, 0.0);
		std::vector<double> eeyy(24, 0.0);
		std::vector<double> eezz(24, 0.0);
		std::vector<double> eeyz(24, 0.0);
		std::vector<double> eexz(24, 0.0);
		std::vector<double> eexy(24, 0.0);
		// 计算应变分量
		int i1, i2, i3;
		for (int i = 0; i < 8; ++i)
		{
			i1 = i * _dim + 0;
			i2 = i * _dim + 1;
			i3 = i * _dim + 2;
			// exx = +[u/x]
			eexx[i1] = +cu[i][1];
			// eyy = +[v/y]
			eeyy[i2] = +cv[i][2];
			// ezz = +[w/z]
			eezz[i3] = +cw[i][3];
			// eyz = +[v/z]+[w/y]
			eeyz[i2] = +cv[i][3];
			eeyz[i3] += +cw[i][2];
			// exz = +[u/z]+[w/x]
			eexz[i1] = +cu[i][3];
			eexz[i3] += +cw[i][1];
			// exy = +[u/y]+[v/x]
			eexy[i1] = +cu[i][2];
			eexy[i2] += +cv[i][1];
		}
		// 组装载荷向量
		double stif;
		for (int i = 0; i < 8; ++i)
		{
			stif = cu[i][0] * fu * vol;
			_result.eload[3 * i + 0] += stif * weight;
			stif = cv[i][0] * fv * vol;
			_result.eload[3 * i + 1] += stif * weight;
			stif = cw[i][0] * fw * vol;
			_result.eload[3 * i + 2] += stif * weight;
		}
		// 组装刚度矩阵
		int ii = -1;
		for (int i = 0; i < 24; ++i)
		{
			for (int j = 0; j < 24; ++j)
			{
				stif = eexx[i] * eexx[j] * (1 - pv) * fact +
					eexx[i] * eeyy[j] * pv * fact +
					eexx[i] * eezz[j] * pv * fact +
					eeyy[i] * eexx[j] * pv * fact +
					eeyy[i] * eeyy[j] * (1 - pv) * fact +
					eeyy[i] * eezz[j] * pv * fact +
					eezz[i] * eexx[j] * pv * fact +
					eezz[i] * eeyy[j] * pv * fact +
					eezz[i] * eezz[j] * (1 - pv) * fact +
					eeyz[i] * eeyz[j] * shear * fact +
					eexz[i] * eexz[j] * shear * fact +
					eexy[i] * eexy[j] * shear * fact;
				_result.estif[++ii] += stif * weight;
			}
		}
	}


    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

SIFEG::uResult a1ec8g2::uEle(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
    SIFEG::uResult res;

    double pe = matParams.at("pe");
    double pv = matParams.at("pv");
    double vol = 1.0;
    double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = (0.5 - pv);

    const std::vector<double>& u = coef.at("u");
    const std::vector<double>& v = coef.at("v");
    const std::vector<double>& w = coef.at("w");

    // 节点应力和权重
    std::vector<double> nodeSigmaXX(8, 0.0), nodeSigmaYY(8, 0.0), nodeSigmaZZ(8, 0.0);
    std::vector<double> nodeSigmaYZ(8, 0.0), nodeSigmaXZ(8, 0.0), nodeSigmaXY(8, 0.0);
    std::vector<double> nodeWeight(8, 0.0);

    // 单元平均应力
    double sigmaXX = 0.0, sigmaYY = 0.0, sigmaZZ = 0.0;
    double sigmaYZ = 0.0, sigmaXZ = 0.0, sigmaXY = 0.0;
    double totalWeight = 0.0;

    for (int iGaus = 0; iGaus < _nGaus; ++iGaus)
    {
        std::vector<std::vector<double>> rctr;
        std::vector<double> coor;
        dcoor(r, iGaus, coor, rctr);
        std::vector<std::vector<double>> crtr;
        double det = SIFEG::inverse(rctr, crtr);
        std::vector<std::vector<double>> cu;
        shapn(iGaus, coor, crtr, cu);
        double weight = _gaus[iGaus] * det;
        totalWeight += weight;

        double exx = 0.0, eyy = 0.0, ezz = 0.0;
        double eyz = 0.0, exz = 0.0, exy = 0.0;

        for (int i = 0; i < 8; ++i)
        {
            exx += cu[i][1] * u[i];
            eyy += cu[i][2] * v[i];
            ezz += cu[i][3] * w[i];
            eyz += cu[i][3] * v[i] + cu[i][2] * w[i];
            exz += cu[i][3] * u[i] + cu[i][1] * w[i];
            exy += cu[i][2] * u[i] + cu[i][1] * v[i];
        }

        double gSigmaXX = fact * ((1 - pv) * exx + pv * eyy + pv * ezz);
        double gSigmaYY = fact * (pv * exx + (1 - pv) * eyy + pv * ezz);
        double gSigmaZZ = fact * (pv * exx + pv * eyy + (1 - pv) * ezz);
        double gSigmaYZ = fact * shear * eyz;
        double gSigmaXZ = fact * shear * exz;
        double gSigmaXY = fact * shear * exy;

        sigmaXX += gSigmaXX * weight;
        sigmaYY += gSigmaYY * weight;
        sigmaZZ += gSigmaZZ * weight;
        sigmaYZ += gSigmaYZ * weight;
        sigmaXZ += gSigmaXZ * weight;
        sigmaXY += gSigmaXY * weight;

        // 将高斯点应力分配到节点，按形函数值加权
        for (int i = 0; i < 8; ++i)
        {
            double N = cu[i][0];
            double nodeW = N * weight;
            nodeSigmaXX[i] += gSigmaXX * nodeW;
            nodeSigmaYY[i] += gSigmaYY * nodeW;
            nodeSigmaZZ[i] += gSigmaZZ * nodeW;
            nodeSigmaYZ[i] += gSigmaYZ * nodeW;
            nodeSigmaXZ[i] += gSigmaXZ * nodeW;
            nodeSigmaXY[i] += gSigmaXY * nodeW;
            nodeWeight[i] += nodeW;
        }
    }

    // 单元平均应力
    if (totalWeight > 0.0)
    {
        sigmaXX /= totalWeight;
        sigmaYY /= totalWeight;
        sigmaZZ /= totalWeight;
        sigmaYZ /= totalWeight;
        sigmaXZ /= totalWeight;
        sigmaXY /= totalWeight;
    }

    res.eleResult["sigmaXX"] = sigmaXX;
    res.eleResult["sigmaYY"] = sigmaYY;
    res.eleResult["sigmaZZ"] = sigmaZZ;
    res.eleResult["sigmaYZ"] = sigmaYZ;
    res.eleResult["sigmaXZ"] = sigmaXZ;
    res.eleResult["sigmaXY"] = sigmaXY;

    double vonMises = sqrt(0.5 * ((sigmaXX - sigmaYY) * (sigmaXX - sigmaYY) +
                                   (sigmaYY - sigmaZZ) * (sigmaYY - sigmaZZ) +
                                   (sigmaZZ - sigmaXX) * (sigmaZZ - sigmaXX) +
                                   6.0 * (sigmaXY * sigmaXY + sigmaYZ * sigmaYZ + sigmaXZ * sigmaXZ)));
    res.eleResult["vonMises"] = vonMises;
    res.eleResult["volume"] = totalWeight;

    // 节点应力（带权重）
    res.nodeResult["sigmaXX"] = nodeSigmaXX;
    res.nodeResult["sigmaYY"] = nodeSigmaYY;
    res.nodeResult["sigmaZZ"] = nodeSigmaZZ;
    res.nodeResult["sigmaYZ"] = nodeSigmaYZ;
    res.nodeResult["sigmaXZ"] = nodeSigmaXZ;
    res.nodeResult["sigmaXY"] = nodeSigmaXY;
    res.nodeResult["weight"] = nodeWeight;

    return res;
}
