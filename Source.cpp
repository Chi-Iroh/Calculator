#include <iostream>

#include "CharacterType.hpp"
#include "SyntaxChecking.hpp"
#include "ErrorsLogging.hpp"
#include "Result.hpp"
#include "Commands.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

void help() {

}

// Output number formatting : 10000 -> 10 000 / 10'000 / 10, 000
// Special commands :
//	- enable / disable output number formatting
//
// Set variables (from A to Z)
// Implement constants (at least pi, e and eventually phi)

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
				std::cout << formulaResult.value() << std::endl << std::endl;
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