#pragma once

#include <optional>
#include <string>
#include <type_traits>

template <typename NumericType, typename = std::enable_if_t<std::is_arithmetic_v<NumericType>>>
class Operand final {
public:
	explicit Operand(int degree);
	explicit Operand(NumericType value, int degree = 1);

	NumericType value() const;
	int degree() const;
	bool isUnknown() const;

private:
	int m_degree = 1;
	NumericType m_value = 0;
	bool m_isUnknown = false;
};

template <typename NumericType>
Operand<NumericType>::Operand(int degree)
	: m_degree = degree
	, m_isUnknown = true
{
}

template <typename NumericType>
Operand<NumericType>::Operand(NumericType value, int degree = 1)
	: m_degree = degree
	, m_value = value
{
}

template <typename NumericType>
NumericType Operand<NumericType>::value() const {
	return m_value;
}

template <typename NumericType>
int Operand<NumericType>::degree() const {
	return m_degree;
}

template <typename NumericType>
bool Operand<NumericType>::isUnknown() const {
	return m_isUnknown;
}

template <typename NumericType>
std::optional<Operand<NumericType>> parseOperandFromString(const std::string &str) {
	// TODO
}