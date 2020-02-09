#pragma once

#include <optional>

class Operator final {
public:
	enum class OperatorType {
		Addition,
		Subtitution,
		Division,
		Multiplication
	};

	explicit Operator(OperatorType type);

	OperatorType type() const;

private:
	OperatorType m_type;
};

std::optional<Operator::OperatorType> convertCharToOperandType(char c);