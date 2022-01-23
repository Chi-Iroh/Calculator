#include "ErrorsLogging.hpp"
#include "Variables.hpp"
#include <iostream>

void highlightErrorIndexes(SyntaxErrorIndexes indexes, const std::string& formula) {
	std::clog << formula << std::endl;

	std::size_t i{};
	for (const auto pos : indexes) {
		while (i++ != pos) {
			std::clog << '~';
		}

		std::clog << '^';
	}
	while (i++ != formula.size()) {
		std::clog << '~';
	}
	std::clog << std::endl;
}

void logError(Error error, SyntaxErrorIndexes indexes, const std::string& formula) {
	const auto writeErrorMessage = [indexes](const std::string& text, const std::string& pluralSuffix = "s", const std::string& messageEnd = "") {
		std::cerr << text;
		if (indexes.size() > 1) {
			std::cerr << pluralSuffix;
		}
		std::cerr << messageEnd << " !" << std::endl;
	};

	switch (error) {
	case Error::AloneOperators:
		writeErrorMessage("Unexpected operator");
		break;

	case Error::CommasOutsideNumber:
		writeErrorMessage("Comma", "s", "outside a number");
		break;

	case Error::EmptyDelimiters:
		if (indexes.size() == 1) {
			writeErrorMessage("Empty parenthesis or square bracket");
		}
		else {
			writeErrorMessage("Empty parenthesises and/or square brackets", "");
		}
		break;

	case Error::MultipleCommas:
		writeErrorMessage("Multiple commas in the same number", "");
		break;

	case Error::MultipleOperators:
		writeErrorMessage("Multiple operators side-by-side", "");
		break;

	case Error::UnmatchedDelimiters:
		if (indexes.size() == 1) {
			writeErrorMessage("Unmatched parenthesis or square bracket");
		}
		else {
			writeErrorMessage("Unmatched parenthesis and/or square brackets", "");
		}
		break;

	case Error::UnrecognizedCharacters:
		writeErrorMessage("Unrecognized character");
		break;

	case Error::IncorrectVariableName:
		writeErrorMessage("Incorrect variable name : " + extractVarName(formula));
		break;

	// assumes that there's a value
	case Error::IncorrectVariableValue:
		writeErrorMessage("Incorrect variable value : " + extractVarValue(formula).value());
	}
	highlightErrorIndexes(indexes, formula);
	std::clog << std::endl;
}