#include <iostream>
#include "Elastic2DDispFieldData.h"
#include "ElasticT3Data.h"
#include "CDFEG/gidProPost.h"
int makeData(ElasticT3Data& data) {
    // TODO: 添加测试数据
	// 节点信息
	data.addNode(1, 0.0, -20.0);
	data.addNode(2, 40.0, 0.0);
	data.addNode(3, 0.0, 20.0);
	data.addNodeEnd();
	// 单元信息
	data.addEle(1, { 1,2,3 }, "ElT3");
	// 材料信息
	std::map<std::string, double> param1;
	param1["E"] = 210e9;
	param1["nu"] = 0.25;
	param1["t"] = 20e-3;
	param1["fx"] = 0;
	param1["fy"] = 0;
	data.addMate(param1);
	// 单元材料信息
	data.setEleMateId(1, 0);
	Elastic2DDispFieldData* phydata = static_cast<Elastic2DDispFieldData*>(data._phyDatas[0]);
	// 边界条件
	phydata->setFirstBoundry(1, 0.0);
	phydata->setFirstBoundry(1, 0.05e-3,1);
	phydata->setFirstBoundry(2, 0.025e-3);
	phydata->setFirstBoundry(2, 0.0, 1);
	phydata->setFirstBoundry(3, 0.0);
	phydata->setFirstBoundry(3, 0.05e-3, 1);
    return 0;
}

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
	gidProPost.pre();
	//makeData(data);
	data.caculate();
	gidProPost.post();
	return 0;
}