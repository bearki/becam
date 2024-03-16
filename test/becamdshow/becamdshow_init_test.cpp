#include <becam/becam.h>
#include <fstream>
#include <iostream>

int main() {
	// 来个死循环
	while (true) {
		// 初始化句柄
		auto handle = BecamNew();
		if (handle == nullptr) {
			std::cerr << "Failed to initialize handle." << std::endl;
			return 1;
		}
		// 释放句柄
		BecamFree(&handle);
	}

	// OK
	return 0;
}