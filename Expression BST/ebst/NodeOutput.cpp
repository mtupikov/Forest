#include "EBST.h"

std::string EBST::outputInfix(const NodePtr& ptr, bool withBrackets) const {
	std::string result;

	if (ptr) {
		auto typeIsOperator = ptr->m_keyValue.first.type() == ExpressionType::Operator;

		if (withBrackets && typeIsOperator) {
			result += '(';
		}

		result += outputInfix(ptr->m_left, withBrackets);
		if (typeIsOperator) {
			result += ' ';
		}
		result += ptr->m_keyValue.first.toString();
		if (typeIsOperator) {
			result += ' ';
		}
		result += outputInfix(ptr->m_right, withBrackets);

		if (withBrackets && typeIsOperator) {
			result += ')';
		}
	}

	return result;
}

std::string EBST::outputPostfix(const EBST::NodePtr &ptr) const {
	std::string result;

	if (ptr) {
		result += outputPostfix(ptr->m_left);
		result += outputPostfix(ptr->m_right);
		result += ptr->m_keyValue.first.toString();
		result += ' ';
	}

	return result;
}

std::string EBST::outputPrefix(const EBST::NodePtr &ptr) const {
	std::string result;

	if (ptr) {
		result += ptr->m_keyValue.first.toString();
		result += ' ';
		result += outputPrefix(ptr->m_left);
		result += outputPrefix(ptr->m_right);
	}

	return result;
}
