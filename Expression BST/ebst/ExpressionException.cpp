#include "ExpressionException.h"

#include <map>
#include <assert.h>

ExpressionException::ExpressionException(ExpressionError err, int column)
    : std::logic_error("ExpressionException")
    , m_type(err)
    , m_column(column)
{
}

std::string ExpressionException::toString() const {
	const std::map<ExpressionError, std::string> errorToStringMap {
		{ ExpressionError::DivisionByZero, "Division by zero" },
		{ ExpressionError::MissingOperator, "Missiong operator between operands" },
		{ ExpressionError::MissingLeftParentheses, "Missing `(` parentheses" },
		{ ExpressionError::MissingRightParentheses, "Missing `)` parentheses" },
		{ ExpressionError::LeftBracketError, "Left bracket is invalid" },
		{ ExpressionError::OperatorAfterOperator, "Operator cannot be placed after another operator" },
		{ ExpressionError::InvalidToken, "Invalid token" },
		{ ExpressionError::UnknownOperandSize, "Unknown operand must be single char" },
		{ ExpressionError::CannotBalance, "Cannot simplify given expression" },
		{ ExpressionError::TooComplexDivision, "Too complex expression in lower division part" },
		{ ExpressionError::TooComplexDegree, "Too complex expression in degree" },
		{ ExpressionError::MultipleUnknownOperands, "Multiple unknown operands" },
		{ ExpressionError::Unsolvable, "Expression has no solutions" },
		{ ExpressionError::DegreeHigherThanThree, "Degree is higher than three" }
	};

	auto it = errorToStringMap.find(m_type);
	if (it != errorToStringMap.cend()) {
		return it->second;
	}

	assert(false && "Invalid error type");
	return {};
}

ExpressionError ExpressionException::errorType() const {
	return m_type;
}

int ExpressionException::column() const {
	return m_column;
}
