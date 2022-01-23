#include <iostream>

#include "CharacterType.hpp"
#include "SyntaxChecking.hpp"
#include "ErrorsLogging.hpp"
#include "Result.hpp"
#include "Variables.hpp"

void help() {

}

// Output number formatting : 10000 -> 10 000 / 10'000 / 10, 000
// Special commands :
//	- enable / disable output number formatting
//
// Set variables (from A to Z)
// Implement constants (at least pi, e and eventually phi)

int main(int argc, char* argv[]) {

	if (argc > 1) {
		for (std::size_t i{ 1 }; i < argc; i++) {
			if (isSyntaxCorrect(argv[i])) {
				const auto formulaResult{ result(argv[i]) };
				if (formulaResult.has_value()) {
					std::cout << formulaResult.value() << std::endl;
				}
			}
		}
		return 0;
	}

	std::string input{};
	while (true) {
		std::cout << "> ";
		std::getline(std::cin, input);

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
		else if (input.starts_with("set")) {
			const auto variableAssignment{ checkVariableAssignment(input) };
			if (!variableAssignment.has_value()) {
				const auto varValue{ extractVarValue(input) };
				variables[extractVarName(input)] = varValue.has_value() ? result(varValue.value()).value() : 0.L;
			}
			else {
				logError(variableAssignment.value().first, variableAssignment.value().second, input);
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

	return 0;
}