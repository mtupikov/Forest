#include "EBST.h"

#include "ExpressionException.h"

#include <sstream>

namespace {

std::string complexToString(const std::complex<double>& c) {
	const auto realNum = c.real();
	const auto imagNum = c.imag();

	const auto realStr = trimToStringDouble(realNum);
	const auto imagStr = trimToStringDouble(imagNum);

	const auto optPlus = imagNum >= 0.0 ? "+" : "";

	return realStr + optPlus + imagStr + "i";
}

const auto pi = std::acos(-1);

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
	case 3: {
		solveCubic();
		break;
	}
	default: throw ExpressionException(ExpressionError::DegreeHigherThanThree, 0);
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

	const auto solve = [](double a, double b) {
		return -1.0 * b / a;
	};

	const auto a = getNthDegreeUnknownValue(1);
	const auto b = getNthDegreeUnknownValue(0, true);

	m_solution.solutions.push_back({ {m_unknownOperandName}, trimToStringDouble(solve(a, b)) });
}

void EBST::solveQuadratic() {
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
		const auto v1Res = (sqrtD1 - b) / (2.0 * a);
		const auto v2Res = (sqrtD2 - b) / (2.0 * a);

		ExpressionResult res1{ v1Name, complexToString(v1Res) };
		ExpressionResult res2{ v2Name, complexToString(v2Res) };

		return { res1, res2 };
	};

	const auto returnRealNumberResult = [this, &countSqrtOfDiscriminant](double a, double b, double d) -> std::vector<ExpressionResult> {
		const auto sqrtD = countSqrtOfDiscriminant(d);

		const auto numRes = (-b + sqrtD.real()) / (2 * a);
		ExpressionResult res{ m_unknownOperandName, trimToStringDouble(numRes) };

		return { res };
	};

	const auto returnRealNumbersResult = [this, &countSqrtOfDiscriminant](double a, double b, double d) -> std::vector<ExpressionResult> {
		const auto sqrtD = countSqrtOfDiscriminant(d);

		const auto num1Res = (-b - sqrtD.real()) / (2.0 * a);
		const auto num2Res = (-b + sqrtD.real()) / (2.0 * a);
		ExpressionResult res1{ m_unknownOperandName + "1", trimToStringDouble(num1Res) };
		ExpressionResult res2{ m_unknownOperandName + "2", trimToStringDouble(num2Res) };

		return { res1, res2 };
	};

	const auto discriminant = [](double a, double b, double c) {
		return std::pow(b, 2) - 4.0 * a * c;
	};

	const auto a = getNthDegreeUnknownValue(2);
	const auto b = getNthDegreeUnknownValue(1);
	const auto c = getNthDegreeUnknownValue(0, true);

	const auto d = discriminant(a, b ,c);

	m_solution.discriminant.emplace(d);
	std::vector<ExpressionResult> solutions;
	if (d < 0) {
		solutions = returnComplexResult(a, b, d);
	} else if (d == 0.0) {
		solutions = returnRealNumberResult(a, b, d);
	} else {
		solutions = returnRealNumbersResult(a, b, d);
	}
	m_solution.solutions = solutions;
}

void EBST::solveCubic() {
	const auto returnRealNumbersResults = [this](double r, double t1) -> std::vector<ExpressionResult> {
		const auto r13 = r < 0 ? -std::cbrt(-r) : std::cbrt(r);
		const auto resNum1 = -t1 + 2.0 * r13;
		const auto resNum2 = -(r13 + t1);
		ExpressionResult res1{ m_unknownOperandName + "1", trimToStringDouble(resNum1) };
		ExpressionResult res2{ m_unknownOperandName + "2", trimToStringDouble(resNum2) };

		return { res1, res2 };
	};

	const auto returnRealUnequalNumbersResults = [this](double q, double r, double t1) -> std::vector<ExpressionResult> {
		q *= -1.0;
		const auto d1 = std::acos(r / std::sqrt(std::pow(q, 3)));
		const auto r13 = 2.0 * std::sqrt(q);

		const auto x1 = -t1 + r13 * std::cos(d1 / 3.0);
		const auto x2 = -t1 + r13 * std::cos((d1 + 2.0 * pi) / 3.0);
		const auto x3 = -t1 + r13 * std::cos((d1 + 4.0 * pi) / 3.0);

		ExpressionResult res1{ m_unknownOperandName + "1", trimToStringDouble(x1) };
		ExpressionResult res2{ m_unknownOperandName + "2", trimToStringDouble(x2) };
		ExpressionResult res3{ m_unknownOperandName + "3", trimToStringDouble(x3) };

		return { res1, res2, res3 };
	};

	const auto returnComplexResults = [this](double r, double disc, double t1) -> std::vector<ExpressionResult> {
		auto s = r + std::sqrt(disc);
		s = (s < 0) ? -std::cbrt(-s) : std::cbrt(s);
		auto t = r - std::sqrt(disc);
		t = (t < 0) ? -std::cbrt(-t) : std::cbrt(t);
		const auto t2 = t1 + (s + t) / 2.0;
		const auto t3 = std::sqrt(3.0) * (-t + s) / 2.0;

		const auto x1 = -t1 + s + t;
		const auto x23r = -t2;

		ExpressionResult res1{ m_unknownOperandName + "1", trimToStringDouble(x1) };
		ExpressionResult res2{ m_unknownOperandName + "2", complexToString(std::complex<double>(x23r, t3)) };
		ExpressionResult res3{ m_unknownOperandName + "3", complexToString(std::complex<double>(x23r, -t3)) };

		return { res1, res2, res3 };
	};

	const auto a = getNthDegreeUnknownValue(3);
	const auto b = getNthDegreeUnknownValue(2) / a;
	const auto c = getNthDegreeUnknownValue(1) / a;
	const auto d = getNthDegreeUnknownValue(0, true) / a;

	const auto q = (3.0 * c - std::pow(b, 2)) / 9.0;
	const auto r = (-(27.0 * d) + b * (9.0 * c - 2.0 * std::pow(b, 2))) / 54.0;
	const auto disc = std::pow(q, 3) + std::pow(r, 2);
	const auto t1 = b / 3.0;

	std::vector<ExpressionResult> solutions;
	if (disc > 0) {
		solutions = returnComplexResults(r, disc, t1);
	} else if (disc == 0.0) {
		solutions = returnRealNumbersResults(r, t1);
	} else {
		solutions = returnRealUnequalNumbersResults(q, r, t1);
	}
	m_solution.solutions = solutions;
}

double EBST::getNthDegreeUnknownValue(int degree, bool isFirst) const {
	assert(degree >= 0);
	if (m_degreeSubtrees.count(degree) == 0 || m_degreeSubtrees.at(degree).size() == 0) {
		return 0.0;
	}

	const auto vec = m_degreeSubtrees.at(degree);
	const auto exp = vec.front();
	return retrieveNumberFromNode(exp.subtree, exp.op, isFirst);
}
