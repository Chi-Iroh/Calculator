#include "CharacterType.hpp"
#include <algorithm>
#include <cctype>

bool isOperator(const std::string& c) {
	return c.size() == 1 && isOperator(c[0]);
}

bool areAllCharactersSpaces(const std::string& formula) {
	return std::find_if(
		formula.cbegin(), formula.cend(), 
		[](char c) {
			return !std::isspace(c);
		}
	) == formula.cend();
}

std::string longestSequenceOfAlphaCharacters(const std::string& string, std::size_t beginIndex) {
	std::string sequence{};
	for (char c : string.substr(beginIndex)) {
		if (!std::isalpha(c) && c != '_') {
			return sequence;
		}

		sequence += c;
	}

	// all the formula was an identifier
	return sequence;
}