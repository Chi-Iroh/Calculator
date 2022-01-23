#include "Variables.hpp"
#include "SyntaxChecking.hpp"
#include "ErrorsLogging.hpp"
#include <algorithm>
#include <sstream>

std::map<std::string, long double> variables{};

std::optional<VariableSyntaxErrorDetails> checkVariableAssignment(const std::string& formula) {
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
		return VariableSyntaxErrorDetails{ Error::IncorrectVariableName, indexes };
	}

	const auto copyElem{ elem };
	std::getline(sstream, elem); // if we're at the end of the sstream buffer, then elem isn't changed

	if (elem != copyElem && !isSyntaxCorrect(elem)) {
		return VariableSyntaxErrorDetails{ Error::IncorrectVariableValue, {} }; // indexes aren't important here, because the syntax will be checked later
	}

	return std::nullopt;
}

// assumes formula is a variable assignment
std::string extractVarName(const std::string& formula) {
	std::istringstream sstream{ formula };
	std::string lastPart{};
	sstream >> lastPart; // the "set" instruction is discarded
	sstream >> lastPart; // the name

	return lastPart;
}

// assumes formula is a variable assignment
std::optional<std::string> extractVarValue(const std::string& formula) {
	std::istringstream sstream{ formula };
	std::string lastPart{};
	sstream >> lastPart; // the "set" instruction is discarded
	sstream >> lastPart; // the name
	const auto lastPartCopy{ lastPart };

	std::getline(sstream, lastPart); // value

	if (lastPartCopy == lastPart) { // we're at the end of sstream -> no value
		return std::nullopt;
	}

	return lastPart;
}