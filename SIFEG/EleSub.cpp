#include "EleSub.h"
#include "PhyFieldData.h"
#include "FemData.h"

SIFEG::EleSubBase::EleSubBase(int nNode, PhyFieldData* pData)
{
	_nNode = nNode;
	_phyData = pData;
	_femData = _phyData->_femData;
}

SIFEG::EleSubResult& SIFEG::EleSubBase::run(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams)
{
	return _result;
}

SIFEG::uResult SIFEG::EleSubBase::uEle(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams)
{
	uResult res;
	return res;
}
