#include "GidPrePost2.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
SIFEG::GidPrePost2::GidPrePost2(FEMData* data) :Processor(data, nullptr)
{

}

SIFEG::GidPrePost2::~GidPrePost2()
{

}

int SIFEG::GidPrePost2::pre()
{
	readMatFile();
	readDatFile();
	return 0;
}

int SIFEG::GidPrePost2::post(int it /*= 0*/)
{
	return 0;
}

bool SIFEG::GidPrePost2::readMatFile()
{

	return true;
}

bool SIFEG::GidPrePost2::readDatFile()
{
	return true;
}
