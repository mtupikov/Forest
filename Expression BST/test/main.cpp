#include "Operand.h"
#include "Operator.h"

#include <assert.h>

void operandTest() {
	{
		auto t = parseOperandFromString<double>("x");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.variableName() == 'x');
		assert(tRes.isUnknown());
	}

	{
		auto t = parseOperandFromString<double>("xy");
		assert(!t.has_value());
	}

	{
		auto t = parseOperandFromString<double>("3.14567");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.value() == 3.14567);
		assert(!tRes.isUnknown());
	}

	{
		auto t = parseOperandFromString<double>("4.");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.value() == 4.);
	}

	{
		auto t = parseOperandFromString<int>("24434312");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.value() == 24434312);
	}

	{
		auto t = parseOperandFromString<int>("-100500");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.value() == -100500);
	}

	{
		auto t = parseOperandFromString<double>("-24434312.49328409234932");
		assert(t.has_value());
		auto tRes = t.value();
		assert(tRes.value() == -24434312.49328409234932);
	}
}

void operatorTest() {
	{
		auto o = convertCharToOperandType('a');
		assert(!o.has_value());
	}

	{
		auto o = convertCharToOperandType('+');
		assert(o.has_value());
		assert(o.type() == Operator::Type::Addition);
	}
}

int main() {
	operandTest();
	operatorTest();

	return 0;
}
