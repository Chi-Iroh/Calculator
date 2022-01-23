#pragma once
#include <array>
#include <string>
#include <map>
#include <vector>
#include <optional>

enum class Error; // defined in Errorlogging.hpp
using SyntaxErrorIndexes = std::vector<std::size_t>;

extern std::map<std::string, long double> variables;

using VariableSyntaxErrorDetails = std::pair<Error, SyntaxErrorIndexes>;

// assumes that formula begins with "set" -> variable assignment
std::optional<VariableSyntaxErrorDetails> checkVariableAssignment(const std::string& formula);

// assumes formula is a variable assignment
std::string extractVarName(const std::string& formula);

// assumes formula is a variable assignment
std::optional<std::string> extractVarValue(const std::string& formula);