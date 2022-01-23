#pragma once
#include <string_view>
#include <string>

constexpr std::string_view operators{
	"%+-*/^"
};

constexpr bool isOperator(char c) {
	return operators.find(c) != std::string_view::npos;
}

bool isOperator(const std::string& c);

constexpr bool isOpeningDelimiter(char c) noexcept {
	return c == '(' || c == '[';
}

constexpr bool isClosingDelimiter(char c) noexcept {
	return c == ')' || c == ']';
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

bool areAllCharactersSpaces(const std::string& formula);

std::string longestSequenceOfAlphaCharacters(const std::string& string, std::size_t beginIndex);