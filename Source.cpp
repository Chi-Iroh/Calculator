#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <deque>
#include <algorithm>
#include <optional>

constexpr std::string_view operators{
	"+-*/^"
};

constexpr bool isOperator(char c) {
	return operators.find(c) != std::string_view::npos;
}

bool isOperator(const std::string& c) {
	return c.size() == 1 && isOperator(c[0]);
}

constexpr bool isClosingDelimiter(char c) noexcept {
	return c == ')' || c == ']';
}

constexpr bool isOpeningDelimiter(char c) noexcept {
	return c == '(' || c == '[';
}

constexpr bool isParenthesis(char c) noexcept {
	return c == '(' || c == ')';
}

constexpr bool isAngleBracket(char c) noexcept {
	return c == '[' || c == ']';
}

constexpr bool isDelimiter(char c) noexcept {
	return isParenthesis(c) || isAngleBracket(c);
}

// if std::isdigit is called with a character outside [0;255] (e.g 'ç', 'à' etc..), an assertion fails
constexpr bool isDigit(char c) noexcept {
	return c >= '0' && c <= '9';
}

void logPos(std::vector<std::size_t> indexes, std::size_t length) {
	std::size_t i{};
	for (const auto pos : indexes) {
		while (i++ != pos) {
			std::clog << '~';
		}

		std::clog << '^';
	}
	while (i++ != length) {
		std::clog << '~';
	}
	std::clog << std::endl;
}

namespace syntax {
	bool twoOperators(const std::string& formula) {
		for (std::size_t i{}; i < formula.size() - 1; i++) {
			if (isOperator(formula[i]) && isOperator(formula[i + 1]) && formula[i + 1] != '-' && formula[i + 1] != '+') { // 3+-2 <=> 3-2 ; 3--2 <=> 3+2

				std::cerr << "Two operators at pos " << (i + 1) << " (" << formula[i] << " and " << formula[i + 1] << ") !" << std::endl;
				std::clog << std::endl << formula << std::endl;
				logPos({ i, i + 1 }, formula.size());
				return false;
			}
		}

		return  true;
	}

	bool twoCommas(const std::string& formula) {
		bool hasComma{};
		std::size_t firstCommaPos{};
		for (std::size_t i{};  const char c : formula) {
			if (isOperator(c)) {
				hasComma = false;
			}
			else if (c == '.') {
				if (hasComma) {
					std::cerr << "Two commas in the same number (previous was at pos " << firstCommaPos << ") !" << std::endl;
					std::clog << std::endl << formula << std::endl;
					logPos({ firstCommaPos, i }, formula.size());
					return false;
				}
				hasComma = true;
				firstCommaPos = i;
			}
			i++;
		}
		return true;
	}

	bool unmatchedDelimiters(const std::string& formula) {
		std::deque<std::size_t> parenthesisesPos{};
		std::deque<std::size_t> angleBracketsPos{};

		// looking for unmatched parenthesis and/or angle brackets

		for (std::size_t i{}; char c : formula) {
			if (isDelimiter(c)) {
				auto& posVector{ isParenthesis(c) ? parenthesisesPos : angleBracketsPos };
				if (isOpeningDelimiter(c)) {
					posVector.push_back(i);
				}
				else {
					if (posVector.size() == 0) {
						std::cerr << "Unmatched closing " << (isParenthesis(c) ? "parenthesis" : "angle bracket") << " at pos " << (i + 1) << " !" << std::endl;
						std::clog << formula << std::endl;
						logPos({ i }, formula.size());
						return false;
					}
					else {
						posVector.pop_back();
					}
				}
			}
			i++;
		}

		if (parenthesisesPos.empty() && angleBracketsPos.empty()) {
			return true;
		}

		// Error message

		std::cerr << "Unmatched opening ";

		if (!parenthesisesPos.empty()) {
			std::cerr << "parenthesis";
			if (parenthesisesPos.size() > 1) {
				std::cerr << "es";
			}
			std::cerr << ' ';
		}
		if (!angleBracketsPos.empty()) {
			if (!parenthesisesPos.empty()) {
				std::cerr << "and ";
			}
			std::cerr << "angle bracket";
			if (angleBracketsPos.size() > 1) {
				std::cerr << "s";
			}
			std::cerr << ' ';
		}
		std::cerr << '!' << std::endl;
		std::clog << formula << std::endl;

		// Showing where are the unmatched parenthesises / angle brackets

		for (std::size_t i{}; i < formula.size(); i++) {
			if (!parenthesisesPos.empty() && parenthesisesPos.front() == i) {
				std::clog << '^';
				parenthesisesPos.pop_front();
			}
			else if (!angleBracketsPos.empty() && angleBracketsPos.front() == i) {
				std::clog << '^';
				angleBracketsPos.pop_front();
			}
			else {
				std::clog << '~';
			}
		}
		std::clog << std::endl;

		return false;
	}

	bool unrecognizedCharacter(const std::string& formula) {
		for (std::size_t i{};  char c : formula) {
			if (!isDigit(c) && !isDelimiter(c) && c != '.' && !isOperator(c) && c != ' ') {
				std::cerr << "Unrecognized character \'" << c << "\' at pos " << (i + 1) << " !" << std::endl;
				std::clog << formula << std::endl;
				logPos({ i }, formula.size());
				std::clog << std::endl;
				return false;
			}
			i++;
		}
		return true;
	}

	bool commaOutsideNumber(const std::string& formula) {
		for (std::size_t i{}; char c : formula) {
			if (c == '.') {

				// no matters if a comma is at the end of the formula, "2." can be std::cin-ed and doesn't need a trailing zero
				if (i == 0 || (!std::isdigit(formula[i - 1]) && formula[i - 1] != '.')) {
					std::cerr << "Comma outside a number !" << std::endl;
					std::clog << std::endl << formula << std::endl;
					logPos({ i }, formula.size());
					return false;
				}
			}
			i++;
		}
		return true;
	}

	// assumes that parenthesis (matching + non-emptiness) and "two operators" were checked previously
	bool aloneOperator(const std::string& formula) {
		if (isOperator(formula[0]) && formula[0] != '+' && formula[0] != '-') {
			std::cerr << "Unexpected operator \'" << formula[0] << "\' at pos 1 !" << std::endl;
			std::clog << std::endl << formula << std::endl;
			logPos({ 0 }, formula.size());
			return false;
		}
		if (isOperator(formula.back())) {
			std::cerr << "Unexpected operator \'" << formula.back() << "\' at pos " << formula.size() - 1 << " !" << std::endl;
			std::clog << std::endl << formula << std::endl;
			logPos({ formula.size() - 1 }, formula.size());
			return false;
		}

		// checks if there's an alone operator in parenthesises, e.g -> "(+)"
		for (std::size_t i{ 1 }; i < formula.size() - 1; i++) {
			if (isParenthesis(formula[i - 1]) && isOperator(formula[i]) && isParenthesis(formula[i + 1])) {
				std::cerr << "Unexpected operator \'" << formula[i] << "\' at pos " << i << " !\t(alone in parenthesises)" << std::endl;
				std::clog << std::endl << formula << std::endl;
				logPos({ i }, formula.size());
				return false;
			}
			else if (isAngleBracket(formula[i - 1]) && isOperator(formula[i]) && isAngleBracket(formula[i + 1])) {
				std::cerr << "Unexpected operator \'" << formula[i] << "\' at pos " << i << " !\t(alone in angle brackets)" << std::endl;
				std::clog << std::endl << formula << std::endl;
				logPos({ i }, formula.size());
				return false;
			}
		}

		return true;
	}

	bool emptyDelimiters(const std::string& formula) {
		for (std::size_t i{}; i < 2; i++) {
			const std::size_t emptyDelimitersPos{ formula.find((i == 0) ? "()" : "[]") };
			if (emptyDelimitersPos != std::string::npos) {
				std::cerr << "Empty " << ((i == 0) ? "parenthesises" : "angle brackets") << " at pos " << emptyDelimitersPos + 1 << " !" << std::endl;
				std::clog << std::endl << formula << std::endl;
				logPos({ emptyDelimitersPos }, formula.size());
				return false;
			}
		}
		return true;
	}
}

bool checkSyntax(const std::string& formula) {
	return
		!formula.empty()						&&	// to avoid out-of-range indexes
		syntax::unrecognizedCharacter(formula)	&&
		syntax::unmatchedDelimiters(formula)	&&
		syntax::twoOperators(formula)			&&
		syntax::emptyDelimiters(formula)		&&
		syntax::aloneOperator(formula)			&&
		syntax::commaOutsideNumber(formula)		&&
		syntax::twoCommas(formula);
}

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
			simplifiedFormula.erase(simplifiedFormula.begin() + i);
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
		return (isOperator(c) ? operators.find(c) : 0);
	};

	const auto maxPriority{ 
		operatorPriority((*std::max_element(
			formula.cbegin(),
			formula.cend(),
			[operatorPriority](const std::string& first, const std::string& second) {
				return operatorPriority(first[0]) < operatorPriority(second[0]);
			}))[0])
	};

	for (std::size_t i{}; const auto& elem : formula) {
		if (operatorPriority(elem[0]) == maxPriority) {
			return i;
		}
		i++;
	}
}

// assumes syntax was checked previously
std::optional<long double> result(const std::string& formula) {
	auto vector{ splitFormula(simplifyOperators(formula)) };

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
			vector.erase(vector.begin() + mostNestedArea.first + 1, vector.begin() + mostNestedArea.second + 1);
		}

		// computes simple operations, assuming no parenthesises and angle brackets remain
		const auto operatorIndex{ maxPriorityOperatorIndex(vector) };
		auto& firstOperand{ vector[operatorIndex - 1] };
		auto& secondOperand{ vector[operatorIndex + 1] };

		const auto isInteger = [](const std::string& elem) {return !isOperator(elem[0]) && elem.find('.') == std::string::npos; };

		switch (vector[operatorIndex][0]) {
		case '+':
			firstOperand = std::stold(firstOperand) + std::stold(secondOperand);
			break;

		case '-':
			firstOperand = std::stold(firstOperand) + std::stold(secondOperand);
			break;

		case '*':
			firstOperand = std::stold(firstOperand) * std::stold(secondOperand);
			break;

		case '/':
			if (secondOperand == "0") {
				std::cerr << "A division by zero occured !" << std::endl;
				return std::nullopt;
			}
			firstOperand = std::stold(firstOperand) / std::stold(secondOperand);
			break;

		case '%':
			if (!isInteger(firstOperand) || !isInteger(secondOperand)) {
				std::cerr << "A modulo with non-integer values occured !" << std::endl;
				std::clog << firstOperand << " % " << secondOperand << std::endl;
				return std::nullopt;
			}
			firstOperand = std::stoll(firstOperand) % std::stoll(secondOperand);
			break;

		case '^':
			firstOperand = std::pow(std::stold(firstOperand), std::stold(secondOperand));
			break;
		}
	}

	return std::stold(vector[0]);
}

void help() {

}

int main() {
	std::string input{};
	while (true) {
		std::cout << "Input expression (\"quit\" to exit) :" << std::endl;
		std::getline(std::cin, input);

		if (input == "quit") {
			break;
		}
		else if (input == "help") {
			help();
		}
		else if (checkSyntax(input)) {
			const auto formulaResult{ result(input) };
			if (formulaResult.has_value()) {
				std::cout << formulaResult.value() << std::endl;
			}
		}
		std::cout << "-----------------------------------" << std::endl << std::endl;
	}

	return 0;
}