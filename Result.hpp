#pragma once
#include <vector>
#include <string>
#include <optional>

#include "CharacterType.hpp"

std::vector<std::string> splitFormula(const std::string& formula);

// simplifies '+' and '-', by removing useless ones or transforming for instance '+-' into only '-'
std::string simplifyOperators(const std::string& formula);

// assumes that parenthesises and angle brackets were checked previously
// returns { A, B } -> the most nested area is formula[A to B - 1]
std::pair<std::size_t, std::size_t> mostNestedDelimiterIndexes(const std::vector<std::string>& formula);

std::size_t maxPriorityOperatorIndex(const std::vector<std::string>& formula);

// adds '*' between delimiters -> e.g "8(2)" => "8*(2)"
// assumes syntax was previsouly checked
std::string addImplicitMultiplyOperators(const std::string& formula);

std::string removeSpaces(const std::string& formula);

// assumes syntax was checked previously
std::string replaceVariables(const std::string& formula);

// assumes syntax was checked previously
std::optional<long double> result(const std::string& formula);