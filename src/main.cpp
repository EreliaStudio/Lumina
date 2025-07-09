#include "argument_parser.hpp"

#include "tokenizer.hpp"

#define DEBUG_LINE() std::cout << __FUNCTION__ << "::" << __LINE__ << std::endl;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		ArgumentParser::printHelp(argv[0]);		
		return (0);
	}
	try
	{
		ArgumentParser argumentParser = readArguments(argc, argv);

		if (argumentParser.inputFile().empty())
		{
			std::cerr << "Error: No input file specified." << std::endl;
			ArgumentParser::printHelp(argv[0]);
			return (0);
		}

		std::vector<Token> tokens = Tokenizer::tokenize(argumentParser.inputFile());
		
		if (argumentParser.isVerboseMode())
		{
			std::cout << "Tokenization complete." << std::endl;
			Tokenizer::printTokens(tokens);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}
