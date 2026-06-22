#include <iostream>
#include "elaFieldData.h"
#include "elbFieldData.h"
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
    gidPrePost.post();
    return 0;
}