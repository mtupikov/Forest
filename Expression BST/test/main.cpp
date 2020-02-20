#include "ExpressionNode.h"

#include <assert.h>

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
}

int main() {
	operandTest();
	operatorTest();

	return 0;
}
