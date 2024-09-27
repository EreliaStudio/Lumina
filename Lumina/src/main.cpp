#pragma once

#include "lumina_tokenizer.hpp"
#include "lumina_metatokenizer.hpp"
#include "lumina_compiler.hpp"

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

	Lumina::Compiler::Product shader = Lumina::Compiler::compile(metaTokens.value);

	if (shader.errors.size() != 0)
	{
		for (const auto& error : shader.errors)
		{
			std::cout << error.what() << std::endl;
		}
		return (-1);
	}

	std::fstream compiledShader(argv[2], std::ios_base::out);

	compiledShader << shader.value;

	compiledShader.close();

	std::cout << Lumina::readFileAsString(argv[2]);


	return (0);
}
