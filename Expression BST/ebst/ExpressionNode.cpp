#include "ExpressionNode.h"

#include <string>

namespace {

const std::string variableRegex{"^([[:alpha:]])$"};
const std::string numberRegex{"^[+-]?(?=[.]?[0-9])[0-9]*(?:[.][0-9]*)?(?:[Ee][+-]?[0-9]+)?$"};

} // end anonymous namespace

ExpressionNode::ExpressionNode(OperatorType type) : ExpressionNode(ExpressionType::Operator) {
	m_operatorType = type;
}

ExpressionNode::ExpressionNode(double value) : ExpressionNode(ExpressionType::Operand) {
	m_operand.value = value;
}

ExpressionNode::ExpressionNode(char variableName) : ExpressionNode(ExpressionType::Operand) {
	m_operand.variableName = variableName;
}

ExpressionType ExpressionNode::type() const {
	return m_type;
}

OperatorType ExpressionNode::operatorType() const {
	return m_operatorType;
}

Operand ExpressionNode::operandValue() const {
	return m_operand;
}

ExpressionNode::ExpressionNode(ExpressionType type) : m_type(type) {}

std::optional<ExpressionNode> parseOperandNodeFromString(const std::string &str) {
   std::smatch sm;
   if (std::regex_match(str, sm, std::regex(variableRegex))) {
	   assert(sm.size() == 2);

	   const auto varName = sm[1].str();
	   assert(varName.length() == 1);

	   return ExpressionNode(varName[0]);
   } else if (std::regex_match(str, sm, std::regex(numberRegex))) {
	   assert(sm.size() > 0);

	   const auto strNum = sm[0].str();
	   const auto num = std::stod(strNum);
	   return ExpressionNode(num);
   }

   return std::nullopt;
}

std::optional<ExpressionNode> parseOperatorNodeFromChar(char op) {
	const std::map<char, OperatorType> charToOperandType {
		{ '+', OperatorType::Addition },
		{ '-', OperatorType::Subtitution },
		{ '/', OperatorType::Division },
		{ '*', OperatorType::Multiplication },
		{ '%', OperatorType::Modulo },
		{ '^', OperatorType::Power }
	};

	const auto it = charToOperandType.find(op);
	if (it != charToOperandType.cend()) {
		return ExpressionNode(charToOperandType.at(op));
	}

	return std::nullopt;
}

bool isOperandUnknown(Operand op) {
	return op.variableName != invalidOperandVarName;
}
