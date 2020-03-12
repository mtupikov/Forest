#include "ExpressionNode.h"
#include "EBST.h"
#include "ExpressionException.h"

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
	try {
		const auto tree = EBST("x - X + x * x - 10 * (10 * X)");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("-100.0 * x + x ^ 2.0") == 0);
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	 try {
		 const auto tree = EBST("(x^2 + (-10.123450 * 660000) + x % 100)");
		 assert(tree.toString(EBST::OutputType::ReducedInfix).compare("-6681477.0 + x % 100.0 + x ^ 2.0") == 0);
	 } catch (const ExpressionException& ex) {
		 std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("-10 * x^2 - -4 * x + 7 + x");
		 assert(tree.toString(EBST::OutputType::ReducedInfix).compare("7.0 + 5.0 * x + -10.0 * x ^ 2.0") == 0);
	 } catch (const ExpressionException& ex) {
		 std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	try {
		const auto tree = EBST("(x ^ 2) * 11 - (x ^ 2) * 2");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("9.0 * x ^ 2.0") == 0);
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("(x + (10 * x)) * x - 20 * x + 10 * x ^ 3 - x ^ 2 * 2");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("-20.0 * x + 9.0 * x ^ 2.0 + 10.0 * x ^ 3.0") == 0);
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 * x ^ 0 + 4 * x^1 - 9.3 * x ^ 2 - 1 * x^0");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("4.0 + 4.0 * x - 9.30 * x ^ 2.0") == 0);
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("(x - 2) * 10 + x ^ 3 * -10 - (x * 12) * x");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("-20.0 + 10.0 * x - 12.0 * x ^ 2.0 + -10.0 * x ^ 3.0") == 0);
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("x * 4 - x ^ 2 * 4");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("4.0 * x - 4.0 * x ^ 2.0") == 0);
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	// invalid scenarios
	try {
		const auto tree = EBST("x^2 - 4 * y + 5 * x");
		assert(false && "did not caught multiple unknowns");
	} catch (const ExpressionException&) {}

    try {
        const auto tree = EBST("x^2 - 4x + 7 + x");
        assert(false && "did not caught missing operator");
	} catch (const ExpressionException&) {}

    try {
        const auto tree = EBST("xd^2 - 4 * x + 7 + x");
        assert(false && "did not caught multiple char variable");
	} catch (const ExpressionException&) {}

    try {
        const auto tree = EBST("(x + 10");
        assert(false && "did not caught missing ')' bracket");
	} catch (const ExpressionException&) {}

    try {
        const auto tree = EBST("x + 10)");
        assert(false && "did not caught missing '(' bracket");
	} catch (const ExpressionException&) {}

    try {
        const auto tree = EBST("10 ? 2");
        assert(false && "did not caught invalid operator");
	} catch (const ExpressionException&) {}

    try {
        const auto tree = EBST("10.312.312.312 + 543.534543.543");
        assert(false && "did not caught invalid operand");
	} catch (const ExpressionException&) {}

	try {
		const auto tree = EBST("10 / (x - 2) + 10 * x ^ 2");
		assert(false && "did not caught too complex division");
	} catch (const ExpressionException&) {}

	try {
		const auto tree = EBST("3 ^ (x - 20) + 20 * x");
		assert(false && "did not caught too complex degree");
	} catch (const ExpressionException&) {}

	try {
		const auto tree = EBST("x / (x - 2) + 10 * x ^ 2");
		assert(false && "did not caught too complex division");
	} catch (const ExpressionException&) {}

	try {
		const auto tree = EBST("x ^ (x - 20) + 20 * x");
		assert(false && "did not caught too complex degree");
	} catch (const ExpressionException&) {}

	try {
		const auto tree = EBST("(x / 100) / (x - 2) + 10 * x ^ 2");
		assert(false && "did not caught too complex division");
	} catch (const ExpressionException&) {}

	try {
		const auto tree = EBST("(x / 100) ^ (x - 20) + 20 * x");
		assert(false && "did not caught too complex degree");
	} catch (const ExpressionException&) {}

    std::cout << "Tree OK" << std::endl;
}

int main() {
	operandTest();
	operatorTest();
    ebstTest();

	return 0;
}
