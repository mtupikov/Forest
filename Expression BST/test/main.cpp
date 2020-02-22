#include "ExpressionNode.h"
#include "EBST.h"

#include <assert.h>
#include <iostream>

void operandTest() {
	{
		auto t = parseOperandNodeFromString("x");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.type() == ExpressionType::Operand);
		assert(tRes.operandValue().variableName == 'x');
		assert(isOperandUnknown(tRes.operandValue()));
	}

	{
		auto t = parseOperandNodeFromString("xy");
		assert(!t.has_value());
	}

	{
		auto t = parseOperandNodeFromString("3.14567");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.operandValue().value == 3.14567);
	}

	{
		auto t = parseOperandNodeFromString("4.");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.operandValue().value == 4.);
	}

	{
		auto t = parseOperandNodeFromString("+24434312");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.operandValue().value == 24434312);
	}

	{
		auto t = parseOperandNodeFromString("-100500");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.operandValue().value == -100500);
	}

	{
		auto t = parseOperandNodeFromString("-24434312.49328409234932");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.operandValue().value == -24434312.49328409234932);
	}

	std::cout << "Operands OK" << std::endl;
}

void operatorTest() {
	{
		auto o = parseOperatorNodeFromChar('a');
		assert(!o.has_value());
	}

	{
		auto o = parseOperatorNodeFromChar('+');
		assert(o.has_value());
		auto oRes = o.value();
		assert(oRes.type() == ExpressionType::Operator);
		assert(oRes.operatorType() == OperatorType::Addition);
	}

	std::cout << "Operators OK" << std::endl;
}

void ebstTest() {
    auto tree = EBST("(1+2)*(3/4)-(5+6)");

    std::cout << tree.toString() << std::endl;
    std::cout << tree.toString(EBST::OutputType::WithParenthese) << std::endl;
    std::cout << tree.toString(EBST::OutputType::Postfix) << std::endl;
    std::cout << tree.toString(EBST::OutputType::Prefix) << std::endl;

    std::cout << "Tree OK" << std::endl;
}

int main() {
	operandTest();
	operatorTest();
    ebstTest();

	return 0;
}
