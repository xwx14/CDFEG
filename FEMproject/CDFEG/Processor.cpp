#include "Processor.h"
#include "FemData.h"
namespace CDFEG {
	Processor::Processor(FEMData* data, PhyFieldData* fieldData)
	{
		_femData = data;
		_phyFieldData = fieldData;
	}

	int Processor::pre()
	{
		return -1;
	}

	int Processor::post(int it /*= 0*/)
	{
		return -1;
	}

	Processor::~Processor()
	{

	}
}