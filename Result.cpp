#include "Result.hpp"
#include "Commands.hpp"
#include <algorithm>
#include <iostream>

std::vector<std::string> splitFormula(const std::string& formula) {
	std::vector<std::string> split{};
	std::string elem{};
	for (const char c : formula) {
		if (isOperator(c) || isDelimiter(c)) {
			if (!elem.empty()) {
				split.push_back(elem);
				elem.clear();
			}
			split.emplace_back(1, c);
		}
		else {
			elem += c;
		}
	}
	if (!elem.empty()) {
		split.emplace_back(std::move(elem));
	}

	return split;
}

// simplifies '+' and '-', by removing useless ones or transforming for instance '+-' into only '-'
std::string simplifyOperators(const std::string& formula) {
	std::string simplifiedFormula{};
	std::string longestSequenceOfMinusAndPlusOperators{};

	for (std::size_t i{}; i < formula.size(); i++) {
		if (formula[i] != '+' && formula[i] != '-') {
			if (!longestSequenceOfMinusAndPlusOperators.empty()) {
				const auto numberOfMinus{ std::count(longestSequenceOfMinusAndPlusOperators.cbegin(), longestSequenceOfMinusAndPlusOperators.cend(), '-') };
				simplifiedFormula += (numberOfMinus % 2 == 0) ? '+' : '-'; // an even numbers of '-' results into a '+'

				longestSequenceOfMinusAndPlusOperators.clear();
			}
			simplifiedFormula += formula[i];
		}
		else {
			longestSequenceOfMinusAndPlusOperators += formula[i];
		}
	}

	// then remove useless '+'
	for (std::size_t i{}; i < simplifiedFormula.size() - 1; i++) {
		if (simplifiedFormula[i] == '+' && (i == 0 || isOpeningDelimiter(simplifiedFormula[i - 1]))) {
			simplifiedFormula.erase(simplifiedFormula.begin() + static_cast<std::ptrdiff_t>(i));
			i--;
		}
	}

	// at the end, we assume that longestSequenceOfMinusAndPlusOperators is empty, because there shall not are operators at the end of the formula
	return simplifiedFormula;
}

// assumes that parenthesises and angle brackets were checked previously
// returns { A, B } -> the most nested area is formula[A to B - A]
std::pair<std::size_t, std::size_t> mostNestedDelimiterIndexes(const std::vector<std::string>& formula) {
	std::vector<std::size_t> nestingLevels{};

	for (std::size_t i{}; const auto & elem : formula) {
		if (elem == "(" || elem == "[") {
			if (i++ == 0) {
				nestingLevels.push_back(1);
				continue;
			}
			nestingLevels.push_back(nestingLevels.back() + 1);
		}
		else if (elem == ")" || elem == "]") {
			// cannot be at pos i = 0
			nestingLevels.push_back(nestingLevels.back() - 1);
			i++;
		}
		else {
			if (i++ == 0) {
				nestingLevels.push_back(0);
				continue;
			}
			nestingLevels.push_back(nestingLevels.back());
		}
	}

	const auto maxNestingLevel{ *std::max_element(nestingLevels.cbegin(), nestingLevels.cend()) };
	std::size_t mostNestedOpeningDelimiterIndex{};
	std::size_t mostNestedClosingDelimiterIndex{};
	bool hasFoundOpeningDelimiterIndex{};

	// returns the last index with the max value, but the function must return the first
	for (std::size_t i{}; const auto level : nestingLevels) {
		if (level == maxNestingLevel) {
			if (!hasFoundOpeningDelimiterIndex) {
				hasFoundOpeningDelimiterIndex = true;
				mostNestedOpeningDelimiterIndex = i;
			}
		}
		if (level < maxNestingLevel && hasFoundOpeningDelimiterIndex) { // we are just after the closing delimiter of the max nested area
			mostNestedClosingDelimiterIndex = i;
			break;
		}
		i++;
	}
	return { mostNestedOpeningDelimiterIndex, mostNestedClosingDelimiterIndex };
}

std::size_t maxPriorityOperatorIndex(const std::vector<std::string>& formula) {
	const auto operatorPriority = [](char c) {
		return (isOperator(c) ? operators.find(c) + 1 : 0);
		// if not +1, so the '+' operator will be worth 0, the same value as non-operators elements
		// e.g -> In "12+2", the highest priority operator is the '+', its priority will be 0 without the '+1', 
		// but '12' wil also result into a priority of 0 and then the index of 12 will be returned
	};

	const auto maxPriority{
		operatorPriority(
			(
				*std::max_element(
					formula.cbegin(),
					formula.cend(),
					[operatorPriority](const std::string& first, const std::string& second) {
						return operatorPriority(first[0]) < operatorPriority(second[0]);
					}
				)
			)
		[0]
	)};

	const auto findPriority = [=](const std::string& elem) {
		return operatorPriority(elem[0]) == maxPriority;
	};

	return static_cast<std::size_t>(std::find_if(formula.cbegin(), formula.cend(), findPriority) - formula.cbegin());
}

// adds '*' between delimiters -> e.g "8(2)" => "8*(2)"
// assumes syntax was previsouly checked and spaces were removes
std::string addMultiplyOperatorBetweenDelimiters(const std::string& formula) {
	if (formula.size() < 4) { // there cannot be valid delimiters
		return formula;
	}

	std::string copy{ formula };

	for (std::size_t i{ 1 }; i <= copy.size() - 1; i++) {

		if (isOpeningDelimiter(copy[i]) && !isOpeningDelimiter(copy[i - 1]) && !isOperator(copy[i - 1])) {
			copy.insert(i++, "*");
		}
		else if (isClosingDelimiter(copy[i - 1]) && !isClosingDelimiter(copy[i]) && !isOperator(copy[i])) {
			copy.insert(i++, "*");
		}
	}

	return copy;
}

std::string removeSpaces(const std::string& formula) {
	std::string reducedFormula{};
	for (char c : formula) {
		if (!std::isspace(c)) {
			reducedFormula += c;
		}
	}
	return reducedFormula;
}

// assumes syntax was checked previously
std::string replaceVariables(const std::string& formula) {
	std::string newFormula{};

	for (std::size_t i{}; i < formula.size(); i++) {

		if (std::isalpha(formula[i]) || formula[i] == '_') {
			const auto identifier{ longestSequenceOfAlphaCharacters(formula, i) }; // it is an existing variable because syntax was checked
			newFormula += std::to_string(variables.at(identifier));
			i += identifier.size() - 1;
		}
		else {
			newFormula += formula[i];
		}
	}

	return newFormula;
}

// assumes syntax was checked previously
std::optional<long double> result(const std::string& formula) {
	auto vector{
		splitFormula(
			simplifyOperators(
				addMultiplyOperatorBetweenDelimiters(
					removeSpaces(
						replaceVariables(formula)
					)
				)
			)
		)
	};

	while (vector.size() > 1) {


		// removes the parenthesises and/or angle brackets
		while (std::find_if(vector.cbegin(), vector.cend(), [](const std::string& elem) {return isDelimiter(elem[0]); }) != vector.cend()) {
			const auto mostNestedArea{ mostNestedDelimiterIndexes(vector) };
			std::string maxPriorityExpression{};

			// mostNestedArea.first + 1, so that it doesn't include the delimiter
			for (std::size_t i{ mostNestedArea.first + 1 }; i < mostNestedArea.second; i++) {
				maxPriorityExpression += vector[i];
			}
			const auto maxPriorityExpressionResult{ result(maxPriorityExpression) };
			if (!maxPriorityExpressionResult.has_value()) {
				return std::nullopt;
			}
			vector[mostNestedArea.first] = std::to_string(maxPriorityExpressionResult.value());
			vector.erase(
				vector.begin() + static_cast<std::ptrdiff_t>(mostNestedArea.first) + 1,
				vector.begin() + static_cast<std::ptrdiff_t>(mostNestedArea.second) + 1
			);
		}

		// e.g -> "(4*7)" results into "28" at this point
		if (vector.size() == 1) {
			break;
		}

		// computes simple operations, assuming no parenthesises and angle brackets remain
		const auto operatorIndex{ maxPriorityOperatorIndex(vector) };
		auto& firstOperand{ vector[operatorIndex - 1] };
		auto& secondOperand{ vector[operatorIndex + 1] };

		const auto isInteger = [](const std::string& elem) {return !isOperator(elem[0]) && elem.find('.') == std::string::npos; };

		switch (vector[operatorIndex][0]) {
		case '+':
			firstOperand = std::to_string(std::stold(firstOperand) + std::stold(secondOperand));
			break;

		case '-':
			firstOperand = std::to_string(std::stold(firstOperand) - std::stold(secondOperand));
			break;

		case '*':
			firstOperand = std::to_string(std::stold(firstOperand) * std::stold(secondOperand));
			break;

		case '/':
			if (std::stold(secondOperand) == 0.L) {
				std::cerr << "A division by zero occured !" << std::endl;
				return std::nullopt;
			}
			firstOperand = std::to_string(std::stold(firstOperand) / std::stold(secondOperand));
			break;

		case '%':
			if (!isInteger(firstOperand) || !isInteger(secondOperand)) {
				std::cerr << "A modulo with non-integer values occured !" << std::endl;
				return std::nullopt;
			}
			firstOperand = std::to_string(std::stoll(firstOperand) % std::stoll(secondOperand));
			break;

		case '^':
			firstOperand = std::to_string(std::pow(std::stold(firstOperand), std::stold(secondOperand)));
			break;
		}

		vector.erase(vector.begin() + static_cast<std::ptrdiff_t>(operatorIndex), vector.begin() + static_cast<std::ptrdiff_t>(operatorIndex) + 2);
	}

	return std::stold(vector[0]);
}