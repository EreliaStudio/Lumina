#pragma once

#include "lumina_tokenizer.hpp"
#include "lumina_lexer.hpp"
#include "lumina_semantic_checker.hpp"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
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

	std::fstream compiledShader(argv[2], std::ios_base::out);

	const std::string LAYOUTS_DELIMITER = "## LAYOUTS DEFINITION ##";
	const std::string CONSTANTS_DELIMITER = "## CONSTANTS DEFINITION ##";
	const std::string ATTRIBUTES_DELIMITER = "## ATTRIBUTES DEFINITION ##";
	const std::string VERTEX_DELIMITER = "## VERTEX SHADER CODE ##";
	const std::string FRAGMENT_DELIMITER = "## FRAGMENT SHADER CODE ##";

	compiledShader << LAYOUTS_DELIMITER << std::endl;
	compiledShader << syntaxResult.sections.layout << std::endl;
	compiledShader << CONSTANTS_DELIMITER << std::endl;
	compiledShader << syntaxResult.sections.constant << std::endl;
	compiledShader << ATTRIBUTES_DELIMITER << std::endl;
	compiledShader << syntaxResult.sections.attribute << std::endl;
	compiledShader << VERTEX_DELIMITER << std::endl;
	compiledShader << syntaxResult.sections.vertexShader << std::endl;
	compiledShader << FRAGMENT_DELIMITER << std::endl;
	compiledShader << syntaxResult.sections.fragmentShader << std::endl;

	compiledShader.close();

	std::cout << Lumina::readFileAsString(argv[2]);

	return (0);
}
