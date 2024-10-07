#pragma once

#include <vector>
#include "token.hpp"

namespace Lumina
{
	struct Tokenizer
	{
		static std::vector<Token> tokenize(const std::filesystem::path& p_path, const std::string& p_inputCode);
	};
}
