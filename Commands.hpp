#pragma once
#include <array>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <variant>

void help();

enum class Error; // defined in ErrorLogging.hpp
using SyntaxErrorIndexes = std::vector<std::size_t>;

extern std::map<std::string, long double> variables;

using SyntaxErrorDetails = std::pair<Error, SyntaxErrorIndexes>;

constexpr std::array<std::string_view, 6> commands{
	"set",
	"reset",
	"save",
	"load",
	"list",
	"savelist"
};

constexpr std::array<std::string_view, 2> reservedIdentifiers{
	"e",
	"pi"
};

bool isCommand(const std::string& formula);

// assumes command is correct
bool hasCommandTheRightNumberOfArgs(const std::string& formula);

bool isReservedIdentifier(const std::string& identifier);

bool isValidVariableName(const std::string& identifier);

using CommandArgs = std::vector<std::string>;

// assumes formula is a well-formed command
CommandArgs getArgs(const std::string& formula);

// assumes the command has the right number of arguments
std::optional<SyntaxErrorDetails> checkArguments(const std::string& formula);

// assuming arguments are all OK
namespace command {
	// but value wasn't checked before
	void set(const CommandArgs& args);
	void reset(const CommandArgs& args);
	void load(const CommandArgs& args);
	void save(const CommandArgs& args);
	void list([[maybe_unused]] const CommandArgs& args);
	void savelist([[maybe_unused]] const CommandArgs& args);
}

void executeCommand(const std::string& formula);