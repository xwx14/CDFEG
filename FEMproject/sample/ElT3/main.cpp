#include <iostream>
#include "Elastic2DDispFieldData.h"
#include "ElasticT3Data.h"
#include "CDFEG/gidProPost.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <project> <path>" << std::endl;
		return 1;
	}
    std::string project = argv[1];
	std::string path = argv[2];
    ElasticT3Data data;
    CDFEG::GidProPost gidProPost(&data);
	gidProPost.setFilePath(path, project);
    data.caculate();
	CDFEG::GidResItem resItem1("disp", CDFEG::GidResultType::Vector);
	resItem1.addVal(0, "u");
	resItem1.addVal(0, "v");
	gidProPost._resItems.push_back(resItem1);
    gidProPost.post();
    return 0;
}