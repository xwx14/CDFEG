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
    CDFEG::ResItem tempItem("temperature", CDFEG::ResType::Scalar);
    tempItem.addVal(0, "T");
    data._prePostConfig._nodeResItems.push_back(tempItem);

    CDFEG::ResItem dispItem("disp", CDFEG::ResType::Vector);
    dispItem.addVal(1, "u");
    dispItem.addVal(1, "v");
    data._prePostConfig._nodeResItems.push_back(dispItem);

    CDFEG::ResItem stressItem("stress", CDFEG::ResType::Matrix);
    stressItem.addVal(1, "sigmaXX");
    stressItem.addVal(1, "sigmaYY");
    stressItem.addVal(1, "sigmaXY");
    data._prePostConfig._nodeResItems.push_back(stressItem);
    // 单元应力（OnGaussPoints，位移场 index=1；温度场无单元量）
    CDFEG::ResItem eleStress("eleStress", CDFEG::ResType::Matrix, CDFEG::ResLocation::OnGaussPoints);
    eleStress.addVal(1, "sigmaXX");
    eleStress.addVal(1, "sigmaYY");
    eleStress.addVal(1, "sigmaXY");
    data._prePostConfig._eleResItems.push_back(eleStress);
    // 单元体积（OnGaussPoints，位移场 index=1）
    CDFEG::ResItem eleVolume("eleVolume", CDFEG::ResType::Scalar, CDFEG::ResLocation::OnGaussPoints);
    eleVolume.addVal(1, "volume");
    data._prePostConfig._eleResItems.push_back(eleVolume);

    data.post(0);
    return 0;
}
