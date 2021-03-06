#include <iostream>
#include <string_view>
#include <array>

#include "CharacterType.hpp"
#include "SyntaxChecking.hpp"
#include "ErrorsLogging.hpp"
#include "Result.hpp"
#include "Commands.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

void help() {
	const auto waitInput = [] {
		std::cout << "--- Next ---";
		std::getchar();
		std::cout << "\x1b[2K"; // deletes current line (the "--- Next ---' msg)
		std::cout << "\x1b[1A"; // moves to the beginning of the line
		std::cout << "\x1b[2K"; // deletes current line
	};

	constexpr std::array<std::string_view, 31> helpMsg{

	"Supported features :",
		"\t- Operators +-*/%^",
		"\t\tNote : % only accepts two integer operands => '5 % 2' is valid whereas '1.2 % 5' and '8 % 3.6' aren't",
		"\t\tNote : / only accepts a non-zero right operand => '0 / 4' is valid whereas '4 / 0' isn't",
		"\t- Parethesises () and square brackets []",
		"\t\tNote : you can mix them => '(1 + 1) * [2 + 2]' is valid",
		"\t- Mathematical constants pi and e",
		"\t- Commands : 'set', 'reset', 'save', 'load', 'list', 'savelist'",
		"\t- Variables creation/modification :",
		"\t\t-> 'set <name> [<value>]' creates (or modifies, if exists at the call) the <name> variable",
		"\t\tNote : if <value> isn't specified, <name> is set to 0",
		"\t\tNote : <name> must be compound of only letters (a-z and/or A-Z) and underscores (_)",
		"\t\tNote : <name> mustn't be a command or a constant identifier",
		"\t\tExample : 'set A 20' and 'set B' are valid whereas 'set save' and 'set pi 12' aren't",
		"\t- Variables deletion :",
		"\t\t-> 'reset [<varlist>]' removes <varlist>",
		"\t\tNote : if <varlist> isn't specified, all variables (except constants) are removed",
		"\t\tNote : if <varlist> contains at least one non-existing identifier, a warning will be displayed but the process continues if there are other variables",
		"\t\tExample : 'reset r' and 'reset' are valid whereas 'reset 1' isn't and 'reset pi e' will raise a warning",
		"\t- Saving variables :",
		"\t\t-> 'save [<varlist>]' copies <varlist> into a save file ('vars.txt')",
		"\t\tNote : if <varlist> isn't specified, all variables (except constants) are saved",
		"\t\tExample : if 'a' is set before, then 'save a' is valid, otherwise not",
		"\t- Loading variables :",
		"\t\t-> 'load [<varlist>]' reads <varlist> from 'vars.txt' and overwrites corresponding variables",
		"\t\tNote : if <varlist> contains at least one variable which isn't saved, then a warning is emitted for each one",
		"\t\tNote : a warning is emitted for each variable encountered in 'vars.txt' but not requested in <varlist>",
		"\t- Listing existing variables :",
		"\t\t-> 'list' displays all the existing variables",
		"\t- Listing saved variables :",
		"\t\t-> 'savelist' displays all saved variables"
	};

	for (const auto& helpLine : helpMsg) {
		std::cout << helpLine << std::endl;
		waitInput();
	}
}

int main(int argc, char* argv[]) {

#ifdef _WIN32
	const auto consoleCP{ GetConsoleCP() };
	SetConsoleOutputCP(1252);
#endif

	std::string input{};
	std::size_t argvIndex{ 1 };

	while (true) {
		std::cout << "> ";

		if (std::cmp_less(argvIndex, argc)) {
			input = argv[argvIndex++];
		}
		else if (argc > 1 && std::cmp_equal(argvIndex, argc)) { // all parameters were computed
			break;
		}
		else {
			std::getline(std::cin, input);
		}

		if (input == "quit") {
			break;
		}
		else if (input == "help") {
			help();
			continue;
		}
		else if (areAllCharactersSpaces(input)) { // all characters are spaces
			std::cout << 0 << std::endl;
			continue;
		}
		else if (isCommand(input)) {
			const auto argumentErrors{ checkArguments(input) };
			if (argumentErrors.has_value()) {
				logError(argumentErrors.value().first, argumentErrors.value().second, input);
			}
			else {
				executeCommand(input);
			}
		}
		else if (isSyntaxCorrect(input)) {
			const auto formulaResult{ result(input) };
			if (formulaResult.has_value()) {
				std::cout << formulaResult.value() << std::endl;
			}
		}
		else {
			checkSyntax(input);
		}
	}

#ifdef _WIN32
	SetConsoleCP(consoleCP);
#endif

	return 0;
}