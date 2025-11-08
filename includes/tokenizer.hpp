#pragma once

#include "token.hpp"

#include <filesystem>
#include <vector>

struct Tokenizer
{
	Tokenizer();

	std::vector<Token> operator()(const std::filesystem::path &p_path) const;
};
