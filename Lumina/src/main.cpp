#pragma once

#include "lumina_tokenizer.hpp"
#include "lumina_lexer.hpp"
#include "lumina_semantic_checker.hpp"

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code]" << std::endl;
		return (0);
	}

	// Read the raw input file
	std::string rawInput = Lumina::readFileAsString(argv[1]);

	// Tokenize the input
	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(rawInput);

	// Output tokens to a file
	std::fstream ouputStream;
	ouputStream.open("resultToken.txt", std::ios_base::out);
	for (const auto& token : tokens)
	{
		ouputStream << token << std::endl;
	}
	ouputStream.close();

	// Check syntax using the lexer
	Lumina::LexerChecker::Result lexerResult = Lumina::LexerChecker::checkSyntax(argv[1], tokens);

	// Output lexer errors
	for (const auto& error : lexerResult.errors)
	{
		std::cout << error.what() << std::endl;
	}

	Lumina::SemanticChecker::Result syntaxResult = Lumina::SemanticChecker::checkSemantic( argv[1], lexerResult.instructions);

	// Output syntax checker errors
	for (const auto& error : syntaxResult.errors)
	{
		std::cerr << "SemanticChecker Error: " << error.what() << std::endl;
	}

	return (0);
}
