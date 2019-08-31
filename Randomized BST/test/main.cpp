#include "RBST.h"

#include <assert.h>

int main() {
	RBST<int, std::string> tree;

	/* insertion */

	for (auto i = 0; i < 100; ++i) {
		tree.insert(i, std::to_string(i));
	}

	for (auto i = 0; i < 100; ++i) {
		assert(tree.find(i)->second == std::to_string(i));
	}

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
			auto item = tree.find(i);
			assert(item->second == std::to_string(i));
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
				assert(!b);
			}
		}
	}

	for (auto i = 0; i < 100; ++i) {
		if (i % 2 == 0) {
			auto item = tree.find(i);
			assert(item->second == std::to_string(i));
		}
	}

	std::cout << "Remove unexisting OK" << std::endl;

	tree.clear();

	for (auto i = 0; i < 100000; ++i) {
		tree.insert(i, std::to_string(i));
	}

	for (auto i = 0; i < 100000; ++i) {
		assert(tree.find(i)->second == std::to_string(i));
	}

	std::cout << "Large Insertion OK" << std::endl;

	tree.clear();

	for (auto i = 0; i < 15; ++i) {
		tree.insert(i, std::to_string(i));
	}

	for (auto it = tree.begin(); it != tree.end(); ++it) {
		it->second = std::to_string(it->first * 2);
	}

	size_t i = 0;
	for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
		assert(it->second == std::to_string(it->first * 2));
		++i;
	}

	assert(i == tree.size());

	std::cout << "Iterators test OK" << std::endl;

	tree.printTree();

	return 0;
}
