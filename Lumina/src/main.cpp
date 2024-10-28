#pragma once

#include "utils.hpp"
#include "tokenizer.hpp"
#include "expected.hpp"

#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"

#include "printer.hpp"


int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}
	
	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1]);

	if (tokens.size() == 0)
	{
		std::cout << "Empty source file [" << argv[1] << "]" << std::endl;
		return (-1);
	}

	Lumina::Lexer::Product lexerProduct = Lumina::Lexer::lex(tokens);

	if (lexerProduct.errors.size() != 0)
	{
		for (const auto& error : lexerProduct.errors)
		{
			std::cerr << error.what() << std::endl;
		}
		return (-1);
	}

    // Lumina::Printer::print(lexerProduct.value);

	Lumina::Parser::Product parserProduct = Lumina::Parser::parse(lexerProduct.value);

	if (parserProduct.errors.size() != 0)
	{
		for (const auto& error : parserProduct.errors)
		{
			std::cerr << error.what() << std::endl;
		}
		return (-1);
	}

	std::cout << "Parser product : " << std::endl << parserProduct.value << std::endl;

	Lumina::Compiler::Product compilerProduct = Lumina::Compiler::compile(parserProduct.value);

	//std::cout << "Shader : " << std::endl << compilerProduct << std::endl;

	return (0);
}
