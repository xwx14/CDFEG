#include "a1eq4g2.h"
#include "elData.h"
#include "aFieldData.h"

a1eq4g2::a1eq4g2(SIFEG::PhyFieldData* pData)
    : SIFEG::Q4g(2, pData) {
    _name="a1eq4g2";
    _dispNames = { "u", "v" };
    _paramNames ={ "pe", "pv", "fu", "fv", "rou", "alpha" };
    _types.insert("a1eq4g2");
}

a1eq4g2::~a1eq4g2() {

}

SIFEG::EleSubResult& a1eq4g2::run(
    const std::vector<double>& r,
    const std::map<std::string, std::vector<double>>& coef,
    const std::map<std::string, double>& matParams
) {
	std::vector<double> refcoor(2);
	std::fill(_result.eload.begin(), _result.eload.end(), 0.0);
	std::fill(_result.estif.begin(), _result.estif.end(), 0.0);
	std::fill(_result.emass.begin(), _result.emass.end(), 0.0);
	double pe = matParams.at("pe");
	double pv = matParams.at("pv");
	double fu = matParams.at("fu");
	double fv = matParams.at("fv");
	double rou = matParams.at("rou");
	double alpha = matParams.at("alpha");
	double vol = 1.0;
	double fact = pe / (1.0 + pv) / (1.0 - pv * 2.0) * vol;
    double shear = 0.5 - pv;
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
		double weight = _gaus[iGaus] * det;
		std::vector<double> eexx(8, 0.0);
		std::vector<double> eeyy(8, 0.0);
		std::vector<double> eexy(8, 0.0);
		int i1, i2;

		//eexx={ du1/dx,0.0, du2/dx,0.0, du3/dx, 0.0, du4/dx,   0.0 };
		//eeyy={ 0.0, dv1/dy, 0.0, dv2/dy,0.0, dv3/dy, 0.0, dv4/dy };
		//eexy={ du1/dy, dv1/dx, du2/dy, dv2/dx, du3/dy, dv3/dx, du4/dy, dv4/dx };
		for (int i = 0; i < 4; ++i)
		{
			i1 = i * 2;
			i2 = i * 2 + 1;
			eexx[i1] = +cu[i][1];
			eeyy[i2] = +cv[i][2];
			eexy[i1] += cu[i][2];
			eexy[i2] += cv[i][1];
		}
		double stif;
		int ii = -1;
		for (int i = 0; i < 4; ++i)
		{
			stif = cu[i][0] * fu * vol;
			_result.eload[2 * i] += stif * weight;
			stif = cv[i][0] * fv * vol;
			_result.eload[2 * i + 1] += stif * weight;
		}
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				stif = +eexx[i] * eexx[j] * (1. - pv) * fact
					+ eexx[i] * eeyy[j] * pv * fact
					+ eeyy[i] * eexx[j] * pv * fact
					+ eeyy[i] * eeyy[j] * (1. - pv) * fact
					+ eexy[i] * eexy[j] * shear * fact;
				_result.estif[++ii] += stif * weight;
			}
		}
	}

    if (_bSaveResult) _results.push_back(_result);
    return _result;
}

SIFEG::uResult a1eq4g2::uEle(
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
    const std::vector<double>& vDisp = coef.at("v");

    // 节点应力和权重
    std::vector<double> nodeSigmaXX(4, 0.0), nodeSigmaYY(4, 0.0);
    std::vector<double> nodeSigmaXY(4, 0.0);
    std::vector<double> nodeWeight(4, 0.0);

    // 单元平均应力
    double sigmaXX = 0.0, sigmaYY = 0.0, sigmaXY = 0.0;
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
        std::vector<std::vector<double>> cv = cu;
        double weight = _gaus[iGaus] * det;
        totalWeight += weight;

        double exx = 0.0, eyy = 0.0, exy = 0.0;

        for (int i = 0; i < 4; ++i)
        {
            exx += cu[i][1] * u[i];
            eyy += cu[i][2] * vDisp[i];
            exy += cu[i][2] * u[i] + cu[i][1] * vDisp[i];
        }

        double gSigmaXX = fact * ((1 - pv) * exx + pv * eyy);
        double gSigmaYY = fact * (pv * exx + (1 - pv) * eyy);
        double gSigmaXY = fact * shear * exy;

        sigmaXX += gSigmaXX * weight;
        sigmaYY += gSigmaYY * weight;
        sigmaXY += gSigmaXY * weight;

        // 将高斯点应力分配到节点，按形函数值加权
        for (int i = 0; i < 4; ++i)
        {
            double N = cu[i][0];
            double nodeW = N * weight;
            nodeSigmaXX[i] += gSigmaXX * nodeW;
            nodeSigmaYY[i] += gSigmaYY * nodeW;
            nodeSigmaXY[i] += gSigmaXY * nodeW;
            nodeWeight[i] += nodeW;
        }
    }

    // 单元平均应力
    if (totalWeight > 0.0)
    {
        sigmaXX /= totalWeight;
        sigmaYY /= totalWeight;
        sigmaXY /= totalWeight;
    }

    res.eleResult["sigmaXX"] = sigmaXX;
    res.eleResult["sigmaYY"] = sigmaYY;
    res.eleResult["sigmaXY"] = sigmaXY;
    res.eleResult["volume"] = totalWeight;

    // 节点应力（带权重）
    res.nodeResult["sigmaXX"] = nodeSigmaXX;
    res.nodeResult["sigmaYY"] = nodeSigmaYY;
    res.nodeResult["sigmaXY"] = nodeSigmaXY;
    res.nodeResult["weight"] = nodeWeight;

    return res;
}
