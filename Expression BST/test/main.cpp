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
	 try {
		 const auto tree = EBST("(x^2 + (-10.123450 * 660000) + x % 100)");
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("x^2 - 4 * x + 7 + x");
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

    try {
		const auto tree = EBST("(x + (10 * x)) * x - 20 * x + 10 * x ^ 3 - x ^ 2 * 2");
        std::cout << tree.toString(EBST::OutputType::Infix) << std::endl;
        std::cout << tree.toString(EBST::OutputType::InfixWithParentheses) << std::endl;
        std::cout << tree.toString(EBST::OutputType::ReducedInfixWithParentheses) << std::endl;
    } catch (const ExpressionTreeException& ex) {
        std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
        assert(false);
    }

    // rules test start

	 try {
		 const auto tree = EBST("x * (x - 10) * (x + 5)");
		 std::cout << "r1: " << tree.toString(EBST::OutputType::ReducedInfixWithParentheses) << std::endl;
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("5 * (x - 10) * (x + 5)");
		 std::cout << "r2: " << tree.toString(EBST::OutputType::ReducedInfixWithParentheses) << std::endl;
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("x * (x - 10) + x * (x + 2)");
		 std::cout << "r3: " << tree.toString(EBST::OutputType::ReducedInfixWithParentheses) << std::endl;
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("2 * (x - 10) + 2 * (x + 2)");
		 std::cout << "r4: " << tree.toString(EBST::OutputType::ReducedInfixWithParentheses) << std::endl;
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("(x - 10) - (x - (x - 2))");
		 std::cout << "r5: " << tree.toString(EBST::OutputType::ReducedInfixWithParentheses) << std::endl;
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("(x - 10) - (3 + (x - 2))");
		 std::cout << "r6: " << tree.toString(EBST::OutputType::ReducedInfixWithParentheses) << std::endl;
	 } catch (const ExpressionTreeException& ex) {
		 std::cout << ex.errorMessage() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

    // rules test finish

    try {
        const auto tree = EBST("x^2 - 4x + 7 + x");
        assert(false && "did not caught missing operator");
    } catch (const ExpressionTreeException&) {}

    try {
        const auto tree = EBST("xd^2 - 4 * x + 7 + x");
        assert(false && "did not caught multiple char variable");
    } catch (const ExpressionTreeException&) {}

    try {
        const auto tree = EBST("(x + 10");
        assert(false && "did not caught missing ')' bracket");
    } catch (const ExpressionTreeException&) {}

    try {
        const auto tree = EBST("x + 10)");
        assert(false && "did not caught missing '(' bracket");
    } catch (const ExpressionTreeException&) {}

    try {
        const auto tree = EBST("10 ? 2");
        assert(false && "did not caught invalid operator");
    } catch (const ExpressionTreeException&) {}

    try {
        const auto tree = EBST("10.312.312.312 + 543.534543.543");
        assert(false && "did not caught invalid operand");
    } catch (const ExpressionTreeException&) {}

    std::cout << "Tree OK" << std::endl;
}

int main() {
	operandTest();
	operatorTest();
    ebstTest();

	return 0;
}
