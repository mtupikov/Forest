#include "RBST.h"

#include <string>
#include <iostream>

int main() {
	RBST<int, std::string> tree;

	for (auto i = 0; i < 100; ++i) {
		tree.insert(i, std::to_string(i));
	}

	for (auto i = 0; i < 100; ++i) {
		std::cout << tree.find(i) << std::endl;
	}

	return 0;
}