#pragma once

#include <vector>

#include "lumina_token.hpp"

namespace Lumina
{
	struct Tokenizer
	{
		static std::vector<Token> tokenize(const std::string& p_rawCode);
	};
}