#pragma once

#include "semantic_parser.hpp"

#include <memory>
#include <string>
#include <vector>

struct Compiler
{
	explicit Compiler(bool enableDebugOutput = false);

	std::string operator()(const SemanticParseResult &result) const;

private:
	bool debugEnabled = false;
};
