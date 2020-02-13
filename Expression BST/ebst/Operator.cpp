#include "Operator.h"

#include <map>
#include <algorithm>

Operator::Operator(Type type) : m_type(type) {}

Type Operator::type() const {
	return m_type;
}

uint Operator::precedenceValue() const {
	const std::map<Operator::Type, uint> charToPrecedenceValue {
		{ Operator::Type::Addition, 3 },
		{ Operator::Type::Subtitution, 3 },
		{ Operator::Type::Division, 2 },
		{ Operator::Type::Multiplication, 2 },
		{ Operator::Type::Modulo, 2 },
		{ Operator::Type::Power, 1 }
	};

	const auto it = charToPrecedenceValue.find(m_type);
	if (it != charToPrecedenceValue.cend()) {
		return charToPrecedenceValue[m_type];
	}

	assert(false && "invalid operator type");
	return 0;
}

std::optional<Operator::Type> convertCharToOperandType(char c) {
	const std::map<char, Operator::Type> charToOperandType {
		{ '+', Operator::Type::Addition },
		{ '-', Operator::Type::Subtitution },
		{ '/', Operator::Type::Division },
		{ '*', Operator::Type::Multiplication },
		{ '%', Operator::Type::Modulo },
		{ '^', Operator::Type::Power }
	};

	const auto it = charToOperandType.find(c);
	if (it != charToOperandType.cend()) {
		return charToOperandType[c];
	}

	return {};
}
