#include "utils.hpp"
#include "tokenizer.hpp"
#include "expected.hpp"

#include <fstream>
#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"

#include "printer.hpp"


int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3)
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

	Lumina::Parser::Product parserProduct = Lumina::Parser::parse(lexerProduct.value);

	if (parserProduct.errors.size() != 0)
	{
		for (const auto& error : parserProduct.errors)
		{
			std::cerr << error.what() << std::endl;
		}
		return (-1);
	}

	Lumina::Compiler::Product compilerProduct = Lumina::Compiler::compile(parserProduct.value);

	std::string outputPath = (argc == 2 ? "a.out" : argv[2]);

	std::ofstream outputFile(outputPath);
	if (outputFile.is_open() == false)
	{
		std::cerr << "Error: Unable to open output file [" << outputPath << "]" << std::endl;
		return (-1);
	}

	// Write the compiler product to the output file
	outputFile << compilerProduct;
	outputFile.close();

	std::cout << "Compilation successful. Output written to [" << outputPath << "]" << std::endl;

	return (0);
}
