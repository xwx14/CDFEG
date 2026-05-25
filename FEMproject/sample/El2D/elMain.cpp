#include <iostream>
#include "aFieldData.h"
#include "elData.h"
#include "SIFEG\gidProPost.h"

int main() {
    elData data;
	SIFEG::GidProPost pre(&data);
	pre.setFilePath("E:\\SQGenFEM\\test\\el2dData", "el");
	pre.pre();
    data.caculate();
    pre.post();
    return 0;
}