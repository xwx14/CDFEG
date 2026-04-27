#include "GidPrePost2.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
CDFEG::GidPrePost2::GidPrePost2(FEMData* data) :Processor(data, nullptr)
{

}

CDFEG::GidPrePost2::~GidPrePost2()
{

}

int CDFEG::GidPrePost2::pre()
{
	readMatFile();
	readDatFile();
	return 0;
}

int CDFEG::GidPrePost2::post(int it /*= 0*/)
{
	return 0;
}

bool CDFEG::GidPrePost2::readMatFile()
{

	return true;
}

bool CDFEG::GidPrePost2::readDatFile()
{
	return true;
}
