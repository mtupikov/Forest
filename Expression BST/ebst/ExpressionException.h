#pragma once

#include <string>

enum class ExpressionError {
	DivisionByZero,
	MissingOperator,
	MissingLeftParentheses,
	MissingRightParentheses,
	LeftBracketError,
	OperatorAfterOperator,
	InvalidToken,
	UnknownOperandSize,
	CannotBalance,
	TooComplexDivision,
	TooComplexDegree,
	MultipleUnknownOperands,
	Unsolvable,
	DegreeHigherThanThree
};

class ExpressionException : public std::logic_error {
public:
	ExpressionException(ExpressionError err, int column);

	std::string toString() const;
	ExpressionError errorType() const;
	int column() const;

private:
	ExpressionError m_type;
	int m_column;
};
