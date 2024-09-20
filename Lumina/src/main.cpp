#pragma once

#include "lumina_tokenizer.hpp"
#include "lumina_exception.hpp"
#include "lumina_utils.hpp"
#include "lumina_metatokenizer.hpp"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1]);

	Lumina::MetaTokenizer::Product metaTokens = Lumina::MetaTokenizer::analyse(tokens);

	if (metaTokens.errors.size() != 0)
	{
		for (const auto& error : metaTokens.errors)
		{
			std::cout << error.what() << std::endl;
		}
		return (-1);
	}

	return (0);
}
