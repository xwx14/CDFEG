#include "ElementBase.h"
#include "PhyFieldData.h"
#include "FemData.h"

CDFEG::ElementBase::ElementBase(int nNode, PhyFieldData* pData)
{
	_nNode = nNode;
	_phyData = pData;
	_femData = _phyData->_femData;
}

CDFEG::EleSubResult& CDFEG::ElementBase::run(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams)
{
	return _result;
}

CDFEG::uResult CDFEG::ElementBase::uEle(const std::vector<double>& r, const std::map<std::string, std::vector<double>>& coef, const std::map<std::string, double>& matParams)
{
	uResult res;
	return res;
}
