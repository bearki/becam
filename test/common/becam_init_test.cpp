#include <becam/becam.h>
#include <fstream>
#include <pkg/LogOutput.hpp>

int main() {
	// 来个死循环
	while (true) {
		// 初始化句柄
		auto handle = BecamNew();
		if (handle == nullptr) {
			DEBUG_LOG("Failed to initialize handle.");
			return 1;
		}
		// OK
		DEBUG_LOG("OK");
		// 释放句柄
		BecamFree(&handle);
	}

	// OK
	return 0;
}