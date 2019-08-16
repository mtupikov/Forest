#include "RBST.h"

#include <string>
#include <iostream>
#include <assert.h>

int main() {
	RBST<int, std::string> tree;

	/* insertion */

	for (auto i = 0; i < 100; ++i) {
		tree.insert(i, std::to_string(i));
	}

	for (auto i = 0; i < 100; ++i) {
		assert(tree.find(i) == std::to_string(i));
	}

	tree.printTree();

	std::cout << "Insertion OK" << std::endl;

	/* removal */

	for (auto i = 0; i < 100; ++i) {
		if (i % 2 == 0) {
			tree.remove(i);
		}
	}

	assert(tree.size() == 50);

	for (auto i = 0; i < 100; ++i) {
		if (i % 2 != 0) {
			auto& item = tree.find(i);
			assert(item == std::to_string(i));
		}
	}

	std::cout << "Removal OK" << std::endl;

	/* clear */

	tree.clear();

	assert(tree.size() == 0);

	std::cout << "Clear OK" << std::endl;

	/* remove unexisting */

	for (auto i = 0; i < 100; ++i) {
		tree.insert(i, std::to_string(i));
	}

	for (auto i = 0; i < 200; ++i) {
		if (i % 2 != 0) {
			auto b = tree.remove(i);
			if (i >= 100) {
				//assert(!b); enable when remove is full done
			}
		}
	}

	for (auto i = 0; i < 100; ++i) {
		if (i % 2 == 0) {
			auto& item = tree.find(i);
			assert(item == std::to_string(i));
		}
	}

	std::cout << "Remove unexisting OK" << std::endl;

	tree.clear();

	for (auto i = 0; i < 100000; ++i) {
		tree.insert(i, std::to_string(i));
	}

	for (auto i = 0; i < 100000; ++i) {
		assert(tree.find(i) == std::to_string(i));
	}

	std::cout << "Large Insertion OK" << std::endl;


	return 0;
}
