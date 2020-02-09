#include "Operator.h"

#include <map>
#include <algorithm>

Operator::Operator(OperatorType type) : m_type(type) {}

OperatorType Operator::type() const {
	return m_type;
}

std::optional<Operator::OperatorType> convertCharToOperandType(char c) {
	const std::map<char, Operator::OperatorType> charToOperandType {
		{ '+', Operator::OperatorType::Addition },
		{ '-', Operator::OperatorType::Subtitution },
		{ '/', Operator::OperatorType::Division },
		{ '*', Operator::OperatorType::Multiplication }
	};

	const auto it = charToOperandType.find(c);
	if (it != charToOperandType.cend()) {
		return charToOperandType[c];
	}

	return {};
}
