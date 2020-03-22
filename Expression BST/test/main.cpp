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

		const auto result = tree.solution().solutions;
		assert(result.size() == 2);

		auto res1 = result[0];
		auto res2 = result[1];
		assert(res1.varName == "x1");
		assert(res1.varResult == trimToStringDouble(0.0));
		assert(res2.varName == "x2");
		assert(res2.varResult == trimToStringDouble(100.0));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	 try {
		 const auto tree = EBST("(x^2 + (-10.123450 * 660000) + x / 100)");
		 assert(tree.toString(EBST::OutputType::ReducedInfix).compare("-6681477.0 + x / 100.0 + x ^ 2.0") == 0);

		 const auto result = tree.solution().solutions;
		 assert(result.size() == 2);

		 auto res1 = result[0];
		 auto res2 = result[1];
		 assert(res1.varName == "x1");
		 assert(res1.varResult == trimToStringDouble(-2584.860315));
		 assert(res2.varName == "x2");
		 assert(res2.varResult == trimToStringDouble(2584.850315));
	 } catch (const ExpressionException& ex) {
		 std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	 try {
		 const auto tree = EBST("-10 * x^2 - -4 * x + 7 + x");
		 assert(tree.toString(EBST::OutputType::ReducedInfix).compare("7.0 + 5.0 * x + -10.0 * x ^ 2.0") == 0);

		 const auto result = tree.solution().solutions;
		 assert(result.size() == 2);

		 auto res1 = result[0];
		 auto res2 = result[1];
		 assert(res1.varName == "x1");
		 assert(res1.varResult == trimToStringDouble(1.123212));
		 assert(res2.varName == "x2");
		 assert(res2.varResult == trimToStringDouble(-0.623212));
	 } catch (const ExpressionException& ex) {
		 std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		 assert(false);
	 }

	try {
		const auto tree = EBST("(x ^ 2) * 11 - (x ^ 2) * 2");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("9.0 * x ^ 2.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 1);

		auto res = result[0];
		assert(res.varName == "x");
		assert(res.varResult == trimToStringDouble(0.0));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 * x ^ 0 + 4 * x^1 - 9.3 * x ^ 2 - 1 * x^0");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("4.0 + 4.0 * x - 9.30 * x ^ 2.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 2);

		auto res1 = result[0];
		auto res2 = result[1];
		assert(res1.varName == "x1");
		assert(res1.varResult == trimToStringDouble(0.905239));
		assert(res2.varName == "x2");
		assert(res2.varResult == trimToStringDouble(-0.475131));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("x * 4 - x ^ 2 * 4");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("4.0 * x - 4.0 * x ^ 2.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 2);

		auto res1 = result[0];
		auto res2 = result[1];
		assert(res1.varName == "x1");
		assert(res1.varResult == trimToStringDouble(1.0));
		assert(res2.varName == "x2");
		assert(res2.varResult == trimToStringDouble(0.0));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 * x^0 + 4 * x^1 - 4 * x^0");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("1.0 + 4.0 * x") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 1);
		auto res = result.front();
		assert(res.varName == "x");
		assert(res.varResult == trimToStringDouble(-0.25));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 + 4 * x + x^2- x^2");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("5.0 + 4.0 * x") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 1);

		auto res = result.front();
		assert(res.varName == "x");
		assert(res.varResult == trimToStringDouble(-5.0 / 4.0));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 * x^0 - 5 * x^0");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("0.0") == 0);
		assert(tree.solution().solutions.size() == 0);
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 * x^0 - 4 * x^0 - 7 * x^1");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("1.0 - 7.0 * x") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 1);
		auto res = result.front();

		assert(res.varName == "x");
		assert(res.varResult == trimToStringDouble(1.0 / 7.0));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 * x^0 + 13 * x^1 + 3 * x^2 - 1* x^0 - 1 * x^1");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("4.0 + 12.0 * x + 3.0 * x ^ 2.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 2);

		auto res1 = result[0];
		auto res2 = result[1];
		assert(res1.varName == "x1");
		assert(res1.varResult == trimToStringDouble(-3.632993));
		assert(res2.varName == "x2");
		assert(res2.varResult == trimToStringDouble(-0.367007));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("6 * x^0 + 11 * x^1 + 5 * x^2 - 1 * x^0 - 1 * x^1");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("5.0 + 10.0 * x + 5.0 * x ^ 2.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 1);

		auto res = result[0];
		assert(res.varName == "x");
		assert(res.varResult == trimToStringDouble(-1.0));
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("5 * x^0 + 3 * x^1 + 3 * x^2 - 1 * x^0 + 0 * x^1");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("4.0 + 3.0 * x + 3.0 * x ^ 2.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 2);

		auto res1 = result[0];
		auto res2 = result[1];
		assert(res1.varName == "x1");
		assert(res1.varResult == "-0.50-1.040833i");
		assert(res2.varName == "x2");
		assert(res2.varResult == "-0.50+1.040833i");
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("8 * x^0 - 6 * x^1 + 0 * x^2 - 5.6 * x^3 - 3 * x^0");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("5.0 - 6.0 * x - 5.60 * x ^ 3.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 3);

		auto res1 = result[0];
		auto res2 = result[1];
		auto res3 = result[2];
		assert(res1.varName == "x1");
		assert(res1.varResult == "0.615598");
		assert(res2.varName == "x2");
		assert(res2.varResult == "-0.307799+1.164324i");
		assert(res3.varName == "x3");
		assert(res3.varResult == "-0.307799-1.164324i");
	} catch (const ExpressionException& ex) {
		std::cout << ex.toString() << "; column: " << ex.column() << std::endl;
		assert(false);
	}

	try {
		const auto tree = EBST("4 * x ^ 3 - 10 * x ^ 2 + 9.46 * x - 90.5");
		assert(tree.toString(EBST::OutputType::ReducedInfix).compare("-90.50 + 9.460 * x - 10.0 * x ^ 2.0 + 4.0 * x ^ 3.0") == 0);

		const auto result = tree.solution().solutions;
		assert(result.size() == 3);

		auto res1 = result[0];
		auto res2 = result[1];
		auto res3 = result[2];
		assert(res1.varName == "x1");
		assert(res1.varResult == "3.593748");
		assert(res2.varName == "x2");
		assert(res2.varResult == "-0.546874+2.448792i");
		assert(res3.varName == "x3");
		assert(res3.varResult == "-0.546874-2.448792i");
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

	try {
		const auto tree = EBST("4 * x^0 - 8 * x^0");
		assert(false && "did not caught unsolving expresion");
	} catch (const ExpressionException&) {}

	try {
		const auto tree = EBST("8 * x^0 - 6 * x^1 + 0 * x^2 - 5.6 * x^4 - 3 * x^3");
		assert(false && "uncought degree higher than three");
	} catch (const ExpressionException&) {}

    std::cout << "Tree OK" << std::endl;
}

int main() {
	operandTest();
	operatorTest();
    ebstTest();

	return 0;
}
