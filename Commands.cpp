#include "Commands.hpp"
#include "SyntaxChecking.hpp"
#include "ErrorsLogging.hpp"
#include "Result.hpp"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <numbers>
#include <functional>
#include <filesystem>
#include <iostream>

void help() {
	const auto waitInput = [] {
		std::cout << "--- Next ---";
		std::getchar();
		std::cout << "\x1b[2K"; // deletes current line (the "--- Next ---' msg)
		std::cout << "\x1b[1A"; // moves to the beginning of the line
		std::cout << "\x1b[2K"; // deletes current line
	};

	constexpr std::array<std::string_view, 39> helpMsg{

	"'help' displays this menu",
	"'quit' exits the app\n",
	"Supported features :",
		"\t- Operators +-*/%^",
		"\t\tNote : % only accepts two integer operands => '5 % 2' is valid whereas '1.2 % 5' and '8 % 3.6' aren't",
		"\t\tNote : / and % only accepts a non-zero right operand => '0 / 4' and '3 % 7' are valid whereas '1 / 0' and '2 % 0' aren't\n",
		"\t- Parethesises () and square brackets []",
		"\t\tNote : you can mix them => '(1 + 1) * [2 + 2]' is valid",
		"\t\tNote : implicit multiplications are supported",
		"\t\tExample : '3(4)', '(4)[5]', and '(5)7' are respectively evaluated as '3*(4)', '(4)*[5]' and '(5)*7'\n",
		"\t- Mathematical constants pi and e",
		"\t\tNote : implicit multiplications are supported",
		"\t\tExample : '3pi' and 'e4' are respectively evaluated as '3*pi' and 'e*4'\n",
		"\t- Commands : 'set', 'reset', 'save', 'load', 'list', 'savelist'\n",
		"\t- Variables creation/modification :",
		"\t\t-> 'set <name> [<value>]' creates (or modifies, if exists at the call) the <name> variable",
		"\t\tNote : if <value> isn't specified, <name> is set to 0",
		"\t\tNote : <name> must be compound of only letters (a-z and/or A-Z) and underscores (_)",
		"\t\tNote : variables may be more than 1 character long, so implicit product of two variables isn't supported :",
		"\t\t\tExample : let a, b and ab three variables, then inputting 'ab' is ambiguous because it may refer to the 'ab' variable or the implicit product 'a*b'",
		"\t\tNote : <name> mustn't be a command or a constant identifier",
		"\t\tExample : 'set A 20' and 'set B' are valid whereas 'set save' and 'set pi 12' aren't\n",
		"\t- Variables deletion :",
		"\t\t-> 'reset [<varlist>]' removes <varlist>",
		"\t\tNote : if <varlist> isn't specified, all variables (except constants) are removed",
		"\t\tNote : if <varlist> contains at least one non-existing identifier, a warning will be displayed but the process continues if there are other variables",
		"\t\tExample : 'reset r' and 'reset' are valid whereas 'reset 1' isn't and 'reset pi e' will raise a warning\n",
		"\t- Saving variables :",
		"\t\t-> 'save [<varlist>]' copies <varlist> into a save file ('vars.txt')",
		"\t\tNote : if <varlist> isn't specified, all variables (except constants) are saved",
		"\t\tExample : if 'a' is set before, then 'save a' is valid, otherwise not\n",
		"\t- Loading variables :",
		"\t\t-> 'load [<varlist>]' reads <varlist> from 'vars.txt' and overwrites corresponding variables",
		"\t\tNote : if <varlist> contains at least one variable which isn't saved, then a warning is emitted for each one",
		"\t\tNote : a warning is emitted for each variable encountered in 'vars.txt' but not requested in <varlist>\n",
		"\t- Listing existing variables :",
		"\t\t-> 'list' displays all the existing variables\n",
		"\t- Listing saved variables :",
		"\t\t-> 'savelist' displays all saved variables\n",
	};

	for (const auto& helpLine : helpMsg) {
		std::cout << helpLine << std::endl;
		waitInput();
	}
}

std::map<std::string, long double> variables{
	{"e", std::numbers::e_v<long double>},
	{"pi", std::numbers::pi_v<long double>}
};

bool isCommand(const std::string& formula) {
	for (const auto& command : commands) {
		if (formula.size() == command.size()) {
			if (formula == command) {
				return true;
			}
			continue;
		}

		// checks if the character right after the command is a space
		if (formula.starts_with(command)) {
			if (std::isspace(formula[command.size()])) {
				return true;
			}
		}
	}
	return false;
}

// assumes formula is a well-formed command
CommandArgs getArgs(const std::string& formula) {
	CommandArgs args{};
	std::istringstream sstream{ formula };
	std::string command{};
	sstream >> command;
	args.push_back(command);

	std::string elem{};
	std::size_t count{};
	while (!sstream.eof()) {
		if (command == "set" && count == 1) { // the value can be written with space chars
			std::getline(sstream, elem);
			do {
				elem.erase(elem.cbegin());
			} while (std::isspace(elem[0]));

			args.push_back(elem);
			break;
		}

		const std::string previousElem{ elem };
		sstream >> elem;

		if (elem == previousElem) { // the formula ends with space characters
			break;
		}
		args.push_back(elem);
		count++;
	}

	return args;
}

// assumes command is correct
bool hasCommandTheRightNumberOfArgs(const std::string& formula) {
	const auto args{ getArgs(formula) };
	if (args[0] == "list" || args[0] == "savelist") {
		return args.size() == 1; // these commands don't take any argument
	}

	if (args[0] == "set") {
		return args.size() == 1 || args.size() == 2;
	}

	// "reset", "save", or "load"
	return true;
}

bool isReservedIdentifier(const std::string& identifier) {
	return std::find(commands.cbegin(), commands.cend(), identifier) != commands.cend() ||
		std::find(reservedIdentifiers.cbegin(), reservedIdentifiers.cend(), identifier) != reservedIdentifiers.cend();
}

bool isValidVariableName(const std::string& identifier) {
	const bool isNotReserved{ !isReservedIdentifier(identifier) };
	const bool isValidIdentifier{ std::find_if_not(identifier.cbegin(), identifier.cend(), [](char c) {return std::isalpha(c) || c == '_'; }) == identifier.cend() };
	return isNotReserved && isValidIdentifier;
}

// assumes the command has the right number of arguments
std::optional<SyntaxErrorDetails> checkArguments(const std::string& formula) {
	const auto args{ getArgs(formula) };
	SyntaxErrorIndexes errors{};

	if (args[0] == "list" || args[0] == "savelist") { // no arguments to check
		if (args.size() > 1) {
			for (std::size_t i{ 1 }; i < args.size(); i++) {
				errors.push_back(i);
			}
			return SyntaxErrorDetails{ Error::UnexpectedArgument, errors };
		}
		return std::nullopt;
	}

	if (args[0] == "reset" || args[0] == "save") { // variable number of arguments which must be valid existing identifiers
		for (std::size_t i{ 1 }; i < args.size(); i++) {
			if (variables.find(args[i]) == variables.cend()) { // variable identifier not found
				errors.push_back(i);
			}
		}
		if (errors.empty()) {
			return std::nullopt;
		}
		return SyntaxErrorDetails{ Error::UnknownIndentifier, errors };
	}

	if (args[0] == "set") {
		if (args.size() == 1) {
			return SyntaxErrorDetails{ Error::MissingVariableName, {} };
		}

		if (!isValidVariableName(args[1])) {
			return SyntaxErrorDetails{ Error::BadVariableName, {1} };
		}

		return std::nullopt;
	}

	// args[0] == "load"
	if (!std::filesystem::exists("vars.txt")) {
		return SyntaxErrorDetails{ Error::NoSaveFile, {} };
	}

	if (args.size() == 1) {
		return std::nullopt;
	}

	for (std::size_t i{ 1 }; i < args.size(); i++) {
		if (!isValidVariableName(args[i])) {
			errors.push_back(i);
		}
	}

	if (!errors.empty()) {
		return SyntaxErrorDetails{ Error::BadVariableName, errors };
	}

	return std::nullopt;
}

void command::set(const CommandArgs& args) {
	if (args.size() == 2) {
		variables[args[1]] = 0.L;
	}
	else {
		if (!isSyntaxCorrect(args[2])) {
			std::cerr << "Bad value syntax :" << std::endl;
			checkSyntax(args[2]);
			return;
		}

		const auto resultValue{ result(args[2]) };
		if (!resultValue.has_value()) {
			std::cerr << "Failed to evaluate value, variable \"" << args[1] << "\" remains unchanged." << std::endl;
			return;
		}

		variables[args[1]] = resultValue.value();
	}
}

void command::reset(const CommandArgs& args) {
	if (args.size() == 1) {
		variables = {
			{"e", std::numbers::e_v<long double>},
			{"pi", std::numbers::pi_v<long double>}
		};
	}
	else {
		for (std::size_t i{ 1 }; i < args.size(); i++) {
			variables.erase(args[i]);
		}
	}
}

void command::load(const CommandArgs& args) {
	std::ifstream vars{ "vars.txt" };
	if (!vars) {
		std::cerr << "Unexpected error while trying to read save file 'vars.txt' !" << std::endl;
		return;
	}

	std::pair<std::string, long double> var{};
	std::vector<std::string> varsLoaded{};
	std::string previousVarName{};

	while (true) {
		vars >> var.first >> var.second;
		if (previousVarName == var.first) { // input didn't change "var"'s value, end of file reached
			break;
		}
		previousVarName = var.first;

		if (!isReservedIdentifier(var.first)) {
			if (args.size() == 1) { // loads all
				variables[var.first] = var.second;
				continue;
			}

			if (std::find(args.cbegin() + 1, args.cend(), var.first) != args.cend()) { // variable to load
				variables[var.first] = var.second;
				varsLoaded.push_back(var.first);
				continue;
			}

			std::clog << "[Warning] Variable \"" << var.first << "\" read in file 'vars.txt', but not in the command arguments, so isn't loaded." << std::endl;
		}
	}

	if (args.size() > 1 && varsLoaded.size() != (args.size() - 1)) { // if there are less read variables than given in the args
		for (std::size_t i{ 1 }; i < args.size(); i++) {
			if (std::find(varsLoaded.cbegin(), varsLoaded.cend(), args[i]) == varsLoaded.cend()) { // var in the args weren't in save file
				std::clog << "[Warning] : Variable \"" << args[i] << "\" isn't saved in file 'vars.txt', its value remains the same." << std::endl;
			}
		}
	}
}

void command::save(const CommandArgs& args) {
	std::ofstream vars{ "vars.txt" };
	if (!vars) {
		std::cerr << "Unexpected error while trying to write into save file 'vars.txt' !" << std::endl;
		return;
	}

	if (args.size() == 1) { // saves all
		for (const auto& var : variables) {
			if (!isReservedIdentifier(var.first)) {
				vars << var.first << ' ' << var.second << std::endl;
			}
		}
		return;
	}

	for (std::size_t i{ 1 }; i < args.size(); i++) {
		if (!isReservedIdentifier(args[i])) {
			vars << args[i] << ' ' << variables.at(args[i]) << std::endl;
		}
		else {
			std::clog << "[Warning] \"" << args[i] << "\" is a constant and wasn't saved into 'vars.txt'" << std::endl;
		}
	}
}

void command::list([[maybe_unused]] const CommandArgs& args) {
	for (const auto& var : variables) {
		if (isReservedIdentifier(var.first)) {
			std::cout << "[Reserved] ";
		}
		std::cout << var.first << " = " << var.second << std::endl;
	}
	std::cout << std::endl;
}

void command::savelist([[maybe_unused]] const CommandArgs& args) {
	std::ifstream vars{ "vars.txt" };
	if (!vars) {
		std::cerr << "Unexpected error while trying to read save file 'vars.txt' !" << std::endl;
		return;
	}

	std::pair<std::string, long double> var{};
	std::string previousVarName{};

	while (true) {
		vars >> var.first >> var.second;
		if (previousVarName == var.first) { // input didn't change "var"'s value, end of file reached
			break;
		}
		previousVarName = var.first;

		if (isReservedIdentifier(var.first)) {
			std::cout << "[Reserved] ";
		}
		std::cout << var.first << " = " << var.second << std::endl;
	}
}

void executeCommand(const std::string& formula) {
	using funcType = decltype(std::function(command::set));

	std::map<std::string, funcType> commandsMap{
		{"set", command::set},
		{"reset", command::reset},
		{"load", command::load},
		{"save", command::save},
		{"list", command::list},
		{"savelist", command::savelist}
	};

	for (const auto& command : commandsMap) {
		if (getArgs(formula)[0] == command.first) {
			command.second(getArgs(formula));
			break;
		}
	}
}