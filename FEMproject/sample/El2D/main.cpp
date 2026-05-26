#include <iostream>
#include "ElDispFieldData.h"
#include "elData.h"
#include "CDFEG/gidProPost.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <project> <path>" << std::endl;
		return 1;
	}
    std::string project = argv[1];
	std::string path = argv[2];
    elData data;
    CDFEG::GidProPost gidProPost(&data);
	gidProPost.setFilePath(path, project);
    data.caculate();
    gidProPost.post();
    return 0;
}