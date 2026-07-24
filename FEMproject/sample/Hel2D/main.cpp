#include <iostream>
#include "HeatFieldData.h"
#include "DelDispFieldData.h"
#include "hel2dData.h"
#include "CDFEG/gidPrePost.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <project> <path>" << std::endl;
		return 1;
	}
    std::string project = argv[1];
    std::string path = argv[2];
    hel2dData data;
    CDFEG::GidPrePost gidPrePost(&data);
    gidPrePost.setFilePath(path, project);
    gidPrePost.pre();
    data.caculate();

    // 注册 GiD 结果项：温度（Heat 场 index=0）、位移与应力（DelDisp 场 index=1）
    CDFEG::GidResItem tempItem("temperature", CDFEG::GidResultType::Scalar);
    tempItem.addVal(0, "T");
    gidPrePost._resItems.push_back(tempItem);

    CDFEG::GidResItem dispItem("disp", CDFEG::GidResultType::Vector);
    dispItem.addVal(1, "u");
    dispItem.addVal(1, "v");
    gidPrePost._resItems.push_back(dispItem);

    CDFEG::GidResItem stressItem("stress", CDFEG::GidResultType::Matrix);
    stressItem.addVal(1, "sigmaXX");
    stressItem.addVal(1, "sigmaYY");
    stressItem.addVal(1, "sigmaXY");
    gidPrePost._resItems.push_back(stressItem);

    gidPrePost.post();
    return 0;
}
