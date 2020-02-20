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

template <ExpressionType ExType>
class ExpressionNode {
public:
	template <typename = std::enable_if_t<ExType == ExpressionType::Operator>>
	explicit ExpressionNode(OperatorType type) {
		m_operatorType = type;
	}

	template <typename = std::enable_if_t<ExType == ExpressionType::Operand>>
	explicit ExpressionNode(double value) {
		m_operand.value = value;
	}

	template <typename = std::enable_if_t<ExType == ExpressionType::Operand>>
	explicit ExpressionNode(char variableName) {
		m_operand.variableName = variableName;
	}

	ExpressionType type() const {
		return ExType;
	}

	template <typename = std::enable_if_t<ExType == ExpressionType::Operator>>
	OperatorType operatorType() const {
		return m_operatorType;
	}

	template <typename = std::enable_if_t<ExType == ExpressionType::Operand>>
	Operand operandValue() const {
		return m_operand;
	}

private:
	union {
		OperatorType m_operatorType;
		Operand m_operand;
	};
};

std::optional<ExpressionNode<ExpressionType::Operand>> parseOperandNodeFromString(const std::string &str);
std::optional<ExpressionNode<ExpressionType::Operator>> parseOperatorNodeFromChar(char op);

bool isOperandUnknown(Operand op);
