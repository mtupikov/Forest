#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <regex>
#include <assert.h>

const auto invalidOperandName{'?'};

template <typename NumericType, typename = std::enable_if_t<std::is_arithmetic_v<NumericType>>>
class Operand final {
public:
	Operand(char varName, int degree);
	Operand(NumericType value, int degree);

	NumericType value() const;
	int degree() const;
	bool isUnknown() const;
	char variableName() const;

private:
	int m_degree = 1;
	NumericType m_value = 0;
	bool m_isUnknown = false;
	char m_variableName = invalidOperandName;
};

template <typename NumericType, typename E>
Operand<NumericType, E>::Operand(char varName, int degree)
	: m_degree(degree)
	, m_isUnknown(true)
	, m_variableName(varName)
{
}

template <typename NumericType, typename E>
Operand<NumericType, E>::Operand(NumericType value, int degree)
	: m_degree(degree)
	, m_value(value)
{
}

template <typename NumericType, typename E>
NumericType Operand<NumericType, E>::value() const {
	return m_value;
}

template <typename NumericType, typename E>
int Operand<NumericType, E>::degree() const {
	return m_degree;
}

template <typename NumericType, typename E>
bool Operand<NumericType, E>::isUnknown() const {
	return m_isUnknown;
}

template <typename NumericType, typename E>
char Operand<NumericType, E>::variableName() const {
	return m_variableName;
}

template <typename NumericType>
std::optional<Operand<NumericType>> parseOperandFromString(const std::string &str) {
	std::smatch sm;
	if (std::regex_match(str, sm, std::regex("^([:alpha:])^(\\d+)?$"))) {
		assert(sm.size() > 0);

		const auto varName = sm[0].str();
		assert(varName.length() == 1);

		auto degree = 1;
		if (sm.size() == 2) {
			degree = std::stoi(sm[1].str());
		}

		return Operand<double>(varName[0], degree);
	} else if (std::regex_match(str, sm, std::regex("^(-?\\d+(\\.\\d*)?)^(\\d+)?$"))) {
		assert(sm.size() > 0);

		const auto strNum = sm[0].str();
		const auto isDouble = strNum.find('.') != std::string::npos;

		auto degree = 1;
		if (sm.size() == 2) {
			degree = std::stoi(sm[1].str());
		}

		if (isDouble) {
			const auto num = std::stod(strNum);
			return Operand<double>(num, degree);
		}

		const auto num = std::stoi(strNum);
		return Operand<int>(num, degree);
	}

	return std::nullopt;
}
