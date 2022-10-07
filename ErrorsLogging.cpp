#include "ErrorsLogging.hpp"
#include "Commands.hpp"
#include <iostream>
#include <sstream>

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

	const auto unexpectedArgumentDetails = [indexes]() {
		std::ostringstream sstream{};
		for (std::size_t i{}; i < indexes.size(); i++) {
			sstream << indexes[i];
			if (i < indexes.size() - 1) {
				sstream << ", ";
			}
		}
		return sstream.str();
	};

	const auto argIndexToFormulaIndex = [formula](std::size_t argIndex) {
		return 1 + formula.substr(1).find(getArgs(formula)[argIndex]);
		// begins the search at index 1, so that there isn't confusion => e.g : "set set 50" -> avoid returning '0'
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

	case Error::BadVariableName:
		writeErrorMessage("Incorrect variable name : " + getArgs(formula)[1]);
		indexes[0] = argIndexToFormulaIndex(indexes[0]);
		break;

	case Error::MissingVariableName:
		writeErrorMessage("Missing variable name");
		break;

	case Error::UnexpectedArgument:
		writeErrorMessage("Unexpected argument", "s", unexpectedArgumentDetails());
		break;

	case Error::NoSaveFile:
		writeErrorMessage("No save file found ('vars.txt'), cannot load variables");
		break;

	case Error::UnknownIndentifier:
		writeErrorMessage("Unknown identifier");
		for (auto& i : indexes) {
			i = argIndexToFormulaIndex(i);
		}
		break;

	case Error::Max:
		break;
	}
	highlightErrorIndexes(indexes, formula);
}