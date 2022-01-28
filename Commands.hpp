#pragma once
#include <array>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <variant>

enum class Error; // defined in ErrorLogging.hpp
using SyntaxErrorIndexes = std::vector<std::size_t>;

extern std::map<std::string, long double> variables;

using SyntaxErrorDetails = std::pair<Error, SyntaxErrorIndexes>;

/*
Commands :

- set <var> [<val>] initializes <var> to <val>, if given. Otherwise, <val> is zero-initialized. Note : <val> can be written with spaces
	-> set a 44
	-> set b 25 + 6
	-> set c a + b

- reset [<vars>] assigns 0 to <vars>, if given. Otherwise, all existing variables are reset to 0.
	-> reset a
	-> reset b c
	-> reset

- save [<vars>] exports name and value of <vars> in a vars.txt file, if <vars> is/are given. Otherwise, all existing variables are exporting.
	-> save a
	-> save b c
	-> save

- load [<vars>] imports <vars> from vars.txt, if given. Otherwise, all variables saved in this file are loaded.
	Note : if an existing variable has the same name than a saved one, the existing one's value is overwritten

	-> load a
	-> load b c
	-> load

- list displays the name and value of all existing variables

- savelist displays the name and value of all saved variables in vars.txt
*/

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
	void list(const CommandArgs& args);
	void savelist(const CommandArgs& args);
}

void executeCommand(const std::string& formula);