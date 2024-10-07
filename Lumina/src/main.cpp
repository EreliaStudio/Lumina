#pragma once

#include "utils.hpp"
#include "tokenizer.hpp"
#include "expected.hpp"

#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"

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

	Lumina::Lexer::Result lexerResult = Lumina::Lexer::lex(tokens);

	if (lexerResult.errors.size() != 0)
	{
		for (const auto& error : lexerResult.errors)
		{
			std::cerr << error.what() << std::endl;
		}
		return (-1);
	}

	Lumina::Parser::Result parserResult = Lumina::Parser::parse(lexerResult.value);

	if (parserResult.errors.size() != 0)
	{
		for (const auto& error : parserResult.errors)
		{
			std::cerr << error.what() << std::endl;
		}
		return (-1);
	}

	Lumina::ShaderImpl compilerResult = Lumina::Compiler::compile(parserResult.value);

	std::cout << "Shader : " << std::endl << compilerResult << std::endl;

	return (0);
}
