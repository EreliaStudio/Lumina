#pragma once

#include <vector>

#include "lumina_token.hpp"

namespace Lumina
{
	struct Tokenizer
	{
		static std::vector<Token> tokenize(const std::filesystem::path& p_path);
		static std::vector<Token> tokenizeString(const std::filesystem::path& p_path, const std::string& p_inputCode);
	};
}