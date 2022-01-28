#pragma once
#include <vector>
#include <string>

using SyntaxErrorIndexes = std::vector<std::size_t>;

void highlightErrorIndexes(SyntaxErrorIndexes indexes, const std::string& formula);

enum class Error {

	// Expression parsing
	UnrecognizedCharacters,
	UnknownIndentifier,
	UnmatchedDelimiters,
	MultipleOperators,
	EmptyDelimiters,
	AloneOperators,
	CommasOutsideNumber,
	MultipleCommas,

	BadVariableName,
	MissingVariableName,
	NoSaveFile,
	UnexpectedArgument,

	Max
};

constexpr std::size_t nErrors{ static_cast<std::size_t>(Error::Max) };

void logError(Error error, SyntaxErrorIndexes indexes, const std::string& formula);