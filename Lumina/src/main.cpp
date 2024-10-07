#pragma once

#include "utils.hpp"
#include "tokenizer.hpp"

#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::string rawCode = Lumina::readFileAsString(argv[1]);

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1], rawCode);

	if (tokens.size() == 0)
	{
		std::cout << "Empty source file [" << argv[1] << "]" << std::endl;
		return (-1);
	}

	for (const auto& token : tokens)
	{
		std::cout << token << std::endl;
	}

	return (0);
}
