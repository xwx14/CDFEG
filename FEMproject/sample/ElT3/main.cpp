#include <iostream>
#include "Elastic2DDispFieldData.h"
#include "ElasticT3Data.h"
#include "CDFEG/gidPrePost.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <project> <path>" << std::endl;
		return 1;
	}
    std::string project = argv[1];
	std::string path = argv[2];
    ElasticT3Data data;
    CDFEG::GidPrePost gidPrePost(&data);
	gidPrePost.setFilePath(path, project);
	gidPrePost.pre();
    data.caculate();
	CDFEG::ResItem resItem1("disp", CDFEG::ResType::Vector);
	resItem1.addVal(0, "u");
	resItem1.addVal(0, "v");
	data._prePostConfig._nodeResItems.push_back(resItem1);
	// 节点应力结果（OnNodes）：常应变三角形无高斯点，应力经节点加权平均外推
	CDFEG::ResItem resItem2("stress", CDFEG::ResType::Matrix);
	resItem2.addVal(0, "Sxx");
	resItem2.addVal(0, "Syy");
	resItem2.addVal(0, "Sxy");
	data._prePostConfig._nodeResItems.push_back(resItem2);
    data.post(0);
    return 0;
}