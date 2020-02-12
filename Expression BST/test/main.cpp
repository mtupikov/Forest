#include "Operand.h"

#include <assert.h>

void operandTest() {
	auto t1 = parseOperandFromString("x^2");
	assert(t1.has_value());

	auto t1Res = t1.value();
	assert(t1Res.degree() == 2);
	assert(t1Res.variableName() == 'x');
	assert(t1Res.isUnknown());
}

int main() {
	operandTest();

	return 0;
}
