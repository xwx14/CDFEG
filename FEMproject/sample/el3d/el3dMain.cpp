#include <iostream>
#include "aFieldData.h"
#include "el3dData.h"
#include "SIFEG\gidPrePost.h"
#include "SIFEG\inpReader.h"
int testGID(el3dData& data) {
    // TODO: 添加测试数据
	SIFEG::GidPrePost pre(&data);
	pre.setFilePath("D:\\", "el3d1");
	pre.pre();
	data.caculate();
	pre.post();
    return 0;
}
int testInp(el3dData& data) {
	SIFEG::inpReader reader(&data,nullptr);
	reader.setFilePath("");

}

int main() {
    el3dData data;
	testGID(data);
    return 0;
}