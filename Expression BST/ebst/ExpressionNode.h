#pragma once

#include <optional>
#include <type_traits>
#include <regex>
#include <assert.h>
#include <map>

const auto invalidOperandVarName = '?';

enum class OperatorType {
	Addition,
	Subtitution,
	Division,
	Multiplication,
	Modulo,
	Power
};

enum class ExpressionType {
	Operator,
	Operand
};

struct Operand final {
	double value = 0.0;
	char variableName = invalidOperandVarName;
};

class ExpressionNode {
public:
	explicit ExpressionNode(OperatorType type);
	explicit ExpressionNode(double value);
	explicit ExpressionNode(char variableName);

	ExpressionType type() const;
	OperatorType operatorType() const;
	Operand operandValue() const;

private:
	ExpressionNode(ExpressionType type);

	union {
		OperatorType m_operatorType;
		Operand m_operand;
	};

	ExpressionType m_type;
};

std::optional<ExpressionNode> parseOperandNodeFromString(const std::string &str);
std::optional<ExpressionNode> parseOperatorNodeFromChar(char op);

bool isOperandUnknown(Operand op);