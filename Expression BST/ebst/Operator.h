#pragma once

#include <optional>

class Operator final {
public:
	enum class Type {
		Addition,
		Subtitution,
		Division,
		Multiplication,
		Modulo,
		Power
	};

	explicit Operator(Type type);

	Type type() const;

	uint precedenceValue() const;

private:
	Type m_type;
};

std::optional<Operator::Type> convertCharToOperandType(char c);