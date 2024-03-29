#include <iostream>
#include <string_view>
#include <array>
#include <utility>

#include "CharacterType.hpp"
#include "SyntaxChecking.hpp"
#include "ErrorsLogging.hpp"
#include "Result.hpp"
#include "Commands.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <locale>
#endif

int main(int argc, char* argv[]) {

#ifdef _WIN32
	// C locale to input accents
	std::locale::global(std::locale(""));
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

#ifdef _WIN32
		// input is formatted as OEM (Microsoft's standard) and must be converted back to ASCII
		OemToCharBuffA(input.c_str(), &input[0], static_cast<DWORD>(input.size()));
#endif

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

	return 0;
}