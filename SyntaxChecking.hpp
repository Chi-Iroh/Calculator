#pragma once
#include <string>
#include <vector>
#include <optional>
#include <map>

#include "CharacterType.hpp"

namespace syntax {
	using SyntaxErrorIndexes = std::vector<std::size_t>;

	std::optional<SyntaxErrorIndexes> multipleOperators(const std::string& formula);

	std::optional<SyntaxErrorIndexes> multipleCommas(const std::string& formula);

	std::optional<SyntaxErrorIndexes> unmatchedDelimiters(const std::string& formula);

	std::optional<SyntaxErrorIndexes> unrecognizedCharacters(const std::string& formula);

	std::optional<SyntaxErrorIndexes> commasOutsideNumber(const std::string& formula);

	// assumes that parenthesis (matching + non-emptiness) and "two operators" were checked previously
	std::optional<SyntaxErrorIndexes> aloneOperators(const std::string& formula);

	std::optional<SyntaxErrorIndexes> emptyDelimiters(const std::string& formula);
}

bool isSyntaxCorrect(const std::string& formula);

void checkSyntax(const std::string& formula);