#pragma once

#include "ast.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct SemanticParseResult
{
	struct ExpressionInfo
	{
		std::string typeName;
		bool isConst = false;
		bool isReference = false;
		bool isArray = false;
		bool hasArraySize = false;
		std::optional<std::size_t> arraySize;
		bool isLValue = false;
	};

	std::vector<std::unique_ptr<Instruction>> instructions;
	std::unordered_map<const Expression *, ExpressionInfo> expressionInfo;
};

struct SemanticParser
{
	SemanticParser();

	SemanticParseResult operator()(std::vector<std::unique_ptr<Instruction>> p_rawInstructions);
};
