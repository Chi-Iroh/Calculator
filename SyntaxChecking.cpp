#include <deque>
#include <iostream>
#include <functional>
#include <array>

#include "SyntaxChecking.hpp"
#include "ErrorsLogging.hpp"
#include "Commands.hpp"

std::optional<syntax::SyntaxErrorIndexes> syntax::multipleOperators(const std::string& formula) {
	SyntaxErrorIndexes indexes{};

	for (std::size_t i{}; i < formula.size() - 1; i++) {
		if (isOperator(formula[i]) && isOperator(formula[i + 1]) && formula[i + 1] != '-' && formula[i + 1] != '+') { // 3+-2 <=> 3-2 ; 3--2 <=> 3+2
			indexes.push_back(i);
			indexes.push_back(i + 1);
		}
	}

	if (indexes.empty()) {
		return std::nullopt;
	}
	return indexes;
}

std::optional<syntax::SyntaxErrorIndexes> syntax::multipleCommas(const std::string& formula) {
	SyntaxErrorIndexes indexes{};

	bool hasComma{};
	std::size_t firstCommaPos{};
	for (std::size_t i{}; const char c : formula) {
		if (isOperator(c)) {
			hasComma = false;
		}
		else if (c == '.') {
			if (hasComma) {
				indexes.push_back(firstCommaPos);
				indexes.push_back(i);
			}
			hasComma = true;
			firstCommaPos = i;
		}
		i++;
	}
	return std::nullopt;
}

std::optional<syntax::SyntaxErrorIndexes> syntax::unmatchedDelimiters(const std::string& formula) {
	std::deque<std::size_t> parenthesisesPos{};
	std::deque<std::size_t> squareBracketsPos{};

	// looking for unmatched parenthesis and/or angle brackets

	for (std::size_t i{}; char c : formula) {
		if (isDelimiter(c)) {
			auto& posVector{ isParenthesis(c) ? parenthesisesPos : squareBracketsPos };
			if (isOpeningDelimiter(c)) {
				posVector.push_back(i);
			}
			else { // closing delimiter
				if (posVector.size() == 0) {
					return SyntaxErrorIndexes{ i };
				}
				else {
					posVector.pop_back();
				}
			}
		}
		i++;
	}

	if (parenthesisesPos.empty() && squareBracketsPos.empty()) {
		return std::nullopt;
	}

	// Showing where are the unmatched parenthesises / angle brackets

	SyntaxErrorIndexes indexes{};

	for (std::size_t i{}; i < formula.size(); i++) {
		if (!parenthesisesPos.empty() && parenthesisesPos.front() == i) {
			indexes.push_back(i);
			parenthesisesPos.pop_front();
		}
		else if (!squareBracketsPos.empty() && squareBracketsPos.front() == i) {
			indexes.push_back(i);
			squareBracketsPos.pop_front();
		}
	}

	return indexes;
}

std::optional<syntax::SyntaxErrorIndexes> syntax::unrecognizedCharacters(const std::string& formula) {
	SyntaxErrorIndexes unrecognizedCharacters{};

	for (std::size_t i{}; i < formula.size(); i++) {

		if (std::isalpha(formula[i]) || formula[i] == '_') {
			const auto identifier{ longestSequenceOfAlphaCharacters(formula, i) };
			if (variables.find(identifier) != variables.cend()) {
				i += identifier.size() - 1;
				continue;
			}
		}

		else if (!isDigit(formula[i]) && !isDelimiter(formula[i]) && formula[i] != '.' && !isOperator(formula[i]) && !std::isspace(formula[i])) {
			unrecognizedCharacters.push_back(i);
		}
	}
	if (unrecognizedCharacters.empty()) {
		return std::nullopt;
	}
	return unrecognizedCharacters;
}

std::optional<syntax::SyntaxErrorIndexes> syntax::commasOutsideNumber(const std::string& formula) {
	SyntaxErrorIndexes indexes{};

	for (std::size_t i{}; char c : formula) {
		if (c == '.') {

			// no matters if a comma is at the end of the formula, "2." can be std::cin-ed and doesn't need a trailing zero
			if (i == 0 || (!std::isdigit(formula[i - 1]) && formula[i - 1] != '.')) {
				indexes.push_back(i);
			}
		}
		i++;
	}
	if (indexes.empty()) {
		return std::nullopt;
	}
	return indexes;
}

// assumes that parenthesis (matching + non-emptiness) and "two operators" were checked previously
std::optional<syntax::SyntaxErrorIndexes> syntax::aloneOperators(const std::string& formula) {
	SyntaxErrorIndexes indexes{};

	if (isOperator(formula.front()) && formula.front() != '+' && formula.front() != '-') {
		indexes.push_back(0);
	}

	if (formula.size() == 1) { // formula.front() == formula.back()
		if (indexes.empty()) {
			return std::nullopt;
		}
		return indexes;
	}

	// checks if there's an alone operator in parenthesises, e.g -> "(+)" ; "(8*)"
	for (std::size_t i{ 1 }; i < formula.size() - 1; i++) {
		if (isOperator(formula[i]) && (isOpeningDelimiter(formula[i - 1]) || isClosingDelimiter(formula[i + 1]))) {
			indexes.push_back(i);
		}
	}

	if (isOperator(formula.back())) {
		indexes.push_back(formula.size() - 1);
	}

	if (indexes.empty()) {
		return std::nullopt;
	}
	return indexes;
}

std::optional<syntax::SyntaxErrorIndexes> syntax::emptyDelimiters(const std::string& formula) {
	SyntaxErrorIndexes indexes{};
	
	for (std::size_t i{}; i < formula.size() - 1; i++) {
		if (formula[i] == '(' && formula[i + 1] == ')') {
			indexes.push_back(i);
			indexes.push_back(i + 1);
		}
		else if (formula[i] == '[' && formula[i + 1] == ']') {
			indexes.push_back(i);
			indexes.push_back(i + 1);
		}
	}

	if (indexes.empty()) {
		return std::nullopt;
	}
	return indexes;
}

std::optional<SyntaxErrorIndexes> syntax::unknownIdentifiers(const std::string& formula) {
	SyntaxErrorIndexes errors{};

	for (std::size_t i{}; i < formula.size(); i++) {

		if (std::isalpha(formula[i]) || formula[i] == '_') {
			const auto identifier{ longestSequenceOfAlphaCharacters(formula, i) }; // it is an existing variable because syntax was checked
			if (variables.find(identifier) == variables.cend()) {
				errors.push_back(i);
			}
			i += identifier.size() - 1;
		}
	}

	if (errors.empty()) {
		return std::nullopt;
	}
	return errors;
}

bool isSyntaxCorrect(const std::string& formula) {
	if (formula.empty() || areAllCharactersSpaces(formula)) {
		return true;
	}

	return
		!syntax::unrecognizedCharacters(formula).has_value()	&&
		!syntax::unknownIdentifiers(formula).has_value()		&&
		!syntax::unmatchedDelimiters(formula).has_value()		&&
		!syntax::multipleOperators(formula).has_value()			&&
		!syntax::emptyDelimiters(formula).has_value()			&&
		!syntax::aloneOperators(formula).has_value()			&&
		!syntax::commasOutsideNumber(formula).has_value()		&&
		!syntax::multipleCommas(formula).has_value();
}

void checkSyntax(const std::string& formula) {
	using functionType = decltype(std::function(syntax::aloneOperators));

	const std::array<functionType, nErrors> errors{
		syntax::unrecognizedCharacters,
		syntax::unknownIdentifiers,
		syntax::unmatchedDelimiters,
		syntax::multipleOperators,
		syntax::emptyDelimiters,
		syntax::aloneOperators,
		syntax::commasOutsideNumber,
		syntax::multipleCommas
	};

	for (std::size_t i{};  const auto & function : errors) {
		const auto indexes{ function(formula) };
		if (indexes.has_value()) {
			logError(static_cast<Error>(i), indexes.value(), formula);
			return;
		}
		i++;
	}
}