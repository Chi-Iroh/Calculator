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

std::map<std::string, long double> variables{
	{"e", std::numbers::e_v<long double>},
	{"pi", std::numbers::pi_v<long double>}
};

bool isCommand(const std::string& formula) {
	return std::find_if(commands.cbegin(), commands.cend(), [formula](const auto& command) {return formula.starts_with(command); }) != commands.cend();
}

/*std::optional<SyntaxErrorDetails> checkVariableAssignment(const std::string& formula) {
	std::istringstream sstream{ formula };

	std::string elem{};
	sstream >> elem;

	sstream >> elem;
	SyntaxErrorIndexes indexes{};
	for (std::size_t i{};  char c : elem) {
		if (!std::isalpha(c) && c != '_') {
			indexes.push_back(i);
		}
		i++;
	}

	if (!indexes.empty()) {
		return SyntaxErrorDetails{ Error::BadVariableName, indexes };
	}

	const auto copyElem{ elem };
	std::getline(sstream, elem); // if we're at the end of the sstream buffer, then elem isn't changed

	if (elem != copyElem && !isSyntaxCorrect(elem)) {
		return SyntaxErrorDetails{ Error::BadVariableValue, {} }; // indexes aren't important here, because the syntax will be checked later
	}

	return std::nullopt;
}*/

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
	return std::find(reservedIdentifiers.cbegin(), reservedIdentifiers.cend(), identifier) != reservedIdentifiers.cend();
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
		return SyntaxErrorDetails{ Error::NonExistingVariable, errors };
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
		for (auto& var : variables) {
			if (!isReservedIdentifier(var.first)) {
				var.second = 0.L;
			}
		}
	}
	else {
		for (std::size_t i{ 1 }; i < args.size(); i++) {
			variables[args[i]] = 0.L;
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

	while (!vars.eof()) {
		vars >> var.first >> var.second;
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
				std::clog << "[Waring] : Variable \"" << args[i] << "\" isn't saved in file 'vars.txt', its value remains the same." << std::endl;
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
			vars << var.first << ' ' << var.second << std::endl;
		}
		return;
	}

	for (std::size_t i{ 1 }; i < args.size(); i++) {
		vars << args[i] << ' ' << variables.at(args[i]) << std::endl;
	}
}

void command::list(const CommandArgs& args) {
	for (const auto& var : variables) {
		if (isReservedIdentifier(var.first)) {
			std::cout << "[Reserved] ";
		}
		std::cout << var.first << " = " << var.second << std::endl;
	}
}

void command::savelist(const CommandArgs& args) {
	std::ifstream vars{ "vars.txt" };
	if (!vars) {
		std::cerr << "Unexpected error while trying to read save file 'vars.txt' !" << std::endl;
		return;
	}

	std::pair<std::string, long double> var{};

	while (!vars.eof()) {
		vars >> var.first >> var.second;
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
		if (formula.starts_with(command.first)) {
			command.second(getArgs(formula));
			break;
		}
	}
}