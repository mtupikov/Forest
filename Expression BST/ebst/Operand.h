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
	explicit Operand(char varName);
	explicit Operand(NumericType value);

	NumericType value() const;
	bool isUnknown() const;
	char variableName() const;

private:
	NumericType m_value = 0;
	char m_variableName = invalidOperandName;
};

template <typename NumericType, typename E>
Operand<NumericType, E>::Operand(char varName)
    : m_variableName(varName)
{
}

template <typename NumericType, typename E>
Operand<NumericType, E>::Operand(NumericType value)
    : m_value(value)
{
}

template <typename NumericType, typename E>
NumericType Operand<NumericType, E>::value() const {
	return m_value;
}

template <typename NumericType, typename E>
bool Operand<NumericType, E>::isUnknown() const {
	return m_variableName != invalidOperandName;
}

template <typename NumericType, typename E>
char Operand<NumericType, E>::variableName() const {
	return m_variableName;
}

template <typename NumericType>
std::optional<Operand<NumericType>> parseOperandFromString(const std::string &str);

template <>
std::optional<Operand<int>> parseOperandFromString(const std::string &str)
{
   std::smatch sm;
   if (std::regex_match(str, sm, std::regex("^(-?\\d+)$"))) {
	   assert(sm.size() == 2);

	   const auto strNum = sm[1].str();
	   const auto num = std::stoi(strNum);
	   return Operand<int>(num);
   }

   return std::nullopt;
}

template <>
std::optional<Operand<double>> parseOperandFromString(const std::string &str)
{
   std::smatch sm;
   if (std::regex_match(str, sm, std::regex("^([[:alpha:]])$"))) {
	   assert(sm.size() == 2);

	   const auto varName = sm[1].str();
	   assert(varName.length() == 1);

	   return Operand<double>(varName[0]);
   } else if (std::regex_match(str, sm, std::regex("^(-?\\d+\\.\\d*)$"))) {
	   assert(sm.size() == 2);

	   const auto strNum = sm[1].str();
	   const auto num = std::stod(strNum);
	   return Operand<double>(num);
   }

   return std::nullopt;
}
