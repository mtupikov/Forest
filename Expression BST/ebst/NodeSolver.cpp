#include "EBST.h"

#include "ExpressionException.h"

#include <sstream>

namespace {

std::string complexToString(const std::complex<double>& c) {
	std::stringstream ss;
	ss << c;
	return ss.str();
}

} // end anonymous namepsace

void EBST::solveExpression() {
	switch (m_maxDegree) {
	case 0: {
		solveNumber();
		break;
	}
	case 1: {
		solveLinear();
		break;
	}
	case 2: {
		solveQuadratic();
		break;
	}
	default: {
		solveLobachevsky();
		break;
	}
	}
}

void EBST::solveNumber() {
	assert(m_degreeSubtrees.size() == 1);

	const auto& vec = m_degreeSubtrees[0];
	assert(vec.size() == 1);

	if (vec.front().subtree->m_keyValue.first.operandValue().value != 0.0) {
		throw ExpressionException(ExpressionError::Unsolvable, 0);
	}
}

void EBST::solveLinear() {
	assert(m_degreeSubtrees.size() == 2);

	const auto& vecDegreeZero = m_degreeSubtrees[0];
	const auto& vecDegreeOne = m_degreeSubtrees[1];

	assert(vecDegreeZero.size() == 1);
	assert(vecDegreeOne.size() == 1);

	const auto solve = [](double a, double b) {
		return -1.0 * b / a;
	};

	const auto numNode = vecDegreeZero.front().subtree;
	const auto numB = numNode->m_keyValue.first.operandValue().value;
	const auto unkNode = vecDegreeOne.front().subtree;
	const auto numA = retrieveNumberFromNode(unkNode, vecDegreeOne.front().op);

	m_solution.solutions.push_back({ {m_unknownOperandName}, std::to_string(solve(numA, numB)) });
}

void EBST::solveQuadratic() {
	assert(m_degreeSubtrees.find(2) != m_degreeSubtrees.end());

	std::vector<SubtreeWithOperator> vecDegreeZero;
	std::vector<SubtreeWithOperator> vecDegreeOne;
	const auto vecDegreeTwo = m_degreeSubtrees[2];

	assert(vecDegreeTwo.size() == 1);

	const auto containsZeroDeg = m_degreeSubtrees[0].size() != 0;
	if (containsZeroDeg) {
		vecDegreeZero = m_degreeSubtrees[0];
	}

	const auto containsOneDeg = m_degreeSubtrees[1].size() != 0;
	if (containsOneDeg) {
		vecDegreeOne = m_degreeSubtrees[1];
	}

	const auto countSqrtOfDiscriminant = [](double d, double c = 1) -> std::complex<double> {
		if (d >= 0) {
			return sqrt(d);
		}

		return std::complex<double>(0, c * sqrt(abs(d)));
	};

	const auto returnComplexResult = [this, &countSqrtOfDiscriminant](double a, double b, double d) -> std::vector<ExpressionResult> {
		const auto sqrtD1 = countSqrtOfDiscriminant(d, -1);
		const auto sqrtD2 = countSqrtOfDiscriminant(d);

		const auto v1Name = m_unknownOperandName + "1";
		const auto v2Name = m_unknownOperandName + "2";
		const auto v1Res = sqrtD1 - b / 2 * a;
		const auto v2Res = sqrtD2 - b / 2 * a;

		ExpressionResult res1{ v1Name, complexToString(v1Res) };
		ExpressionResult res2{ v2Name, complexToString(v2Res) };

		return { res1, res2 };
	};

	const auto returnRealNumberResult = [this, &countSqrtOfDiscriminant](double a, double b, double d) -> std::vector<ExpressionResult> {
		const auto sqrtD = countSqrtOfDiscriminant(d);

		const auto numRes = (-1 * b + sqrtD.real()) / 2 * a;
		ExpressionResult res{ m_unknownOperandName, std::to_string(numRes) };

		return { res };
	};

	const auto returnRealNumbersResult = [this, &countSqrtOfDiscriminant](double a, double b, double d) -> std::vector<ExpressionResult> {
		const auto sqrtD = countSqrtOfDiscriminant(d);

		const auto num1Res = (-1 * b - sqrtD.real()) / 2 * a;
		const auto num2Res = (-1 * b + sqrtD.real()) / 2 * a;
		ExpressionResult res1{ m_unknownOperandName + "1", std::to_string(num1Res) };
		ExpressionResult res2{ m_unknownOperandName + "2", std::to_string(num2Res) };

		return { res1, res2 };
	};

	const auto discriminant = [](double a, double b, double c) {
		return std::pow(b, 2) - 4.0 * a * c;
	};

	double a = 0;
	double b = 0;
	double c = 0;

	if (containsZeroDeg) {
		const auto numNode = vecDegreeZero.front().subtree;
		c = numNode->m_keyValue.first.operandValue().value;
	}

	if (containsOneDeg) {
		const auto unkNode = vecDegreeOne.front().subtree;
		b = retrieveNumberFromNode(unkNode, vecDegreeOne.front().op);
	}

	a = retrieveNumberFromNode(vecDegreeTwo.front().subtree, vecDegreeTwo.front().op);

	const auto d = discriminant(a, b ,c);

	m_solution.discriminant = d;
	if (d < 0) {
		m_solution.solutions = returnComplexResult(a, b, d);
	} else if (d == 0.0) {
		m_solution.solutions = returnRealNumberResult(a, b, d);
	} else {
		m_solution.solutions = returnRealNumbersResult(a, b, d);
	}
}

void EBST::solveLobachevsky() {

}
