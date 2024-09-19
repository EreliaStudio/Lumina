#pragma once

#include <vector>

#include "lumina_token.hpp"

namespace Lumina
{
	struct Tokenizer
	{
		static std::vector<Token> tokenize(const std::filesystem::path& p_path);
	};
}