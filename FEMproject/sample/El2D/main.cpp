#include <iostream>
#include "ElDispFieldData.h"
#include "elData.h"
#include "CDFEG/gidPrePost.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <project> <path>" << std::endl;
		return 1;
	}
    std::string project = argv[1];
	std::string path = argv[2];
    elData data;
    CDFEG::GidPrePost gidPrePost(&data);
	gidPrePost.setFilePath(path, project);
	gidPrePost.pre();
    data.caculate();
	CDFEG::GidResItem resItem1("disp",CDFEG::GidResultType::Vector);
	resItem1.addVal(0, "u");
    resItem1.addVal(0, "v");
    gidPrePost._resItems.push_back(resItem1);
    CDFEG::GidResItem resItem2("stress",CDFEG::GidResultType::Matrix);
    resItem2.addVal(0, "sigmaXX");
    resItem2.addVal(0, "sigmaYY");
    resItem2.addVal(0, "sigmaXY");
    gidPrePost._resItems.push_back(resItem2);
    gidPrePost.post2();
    return 0;
}