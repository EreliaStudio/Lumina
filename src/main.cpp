#include "argument_parser.hpp"

#include <cstddef>
#include <expected>

#include "token.hpp"

template<typename TType>
struct Expected
{
	TType value;
	bool hasError;
	std::string errorMessage;

	Expected() : hasError(false) {}
	Expected(const TType& value) : value(value), hasError(false) {}

	static Expected<TType> Raise(const std::string& errorMessage)
	{
		Expected<TType> result;

		result.hasError = true;
		result.errorMessage = errorMessage;

		return result;
	}
};

class Lexer
{
public:
	using TokenListResult = Expected<TokenList>;

private:
	bool _isVerbose = false;

public:
	void setVerbose(bool verbose)
	{
		_isVerbose = verbose;
	}
	TokenListResult tokenize(const std::string& source)
	{
		TokenList result;

		return result;
	}
};

struct ASTNode
{
	std::vector<std::shared_ptr<ASTNode>> children;
};

class Parser
{
public:
	using ASTResult = Expected<ASTNode>;

private:
	bool _isVerbose = false;

public:
	void setVerbose(bool verbose)
	{
		_isVerbose = verbose;
	}

	ASTResult parse(const TokenList& tokens)
	{
		ASTNode result;

		return result;
	}
};

class Analyzer
{
private:
	bool _isVerbose = false;

public:
	void setVerbose(bool verbose)
	{
		_isVerbose = verbose;
	}

	Expected<std::nullptr_t> analyze(const ASTNode& instructionList)
	{
		return {};
	}
};

class GLSLEmitter
{
private:
	bool _isVerbose = false;

public:
	void setVerbose(bool verbose)
	{
		_isVerbose = verbose;
	}

	Expected<std::string> generate(const ASTNode& ast)
	{
		std::string result;

		return result;
	}
};

struct CompilationUnit
{
	std::string layoutDefinition;
	std::string frameBufferDefinition;
	std::string exceptionDefinition;
	std::string constantDefinition;
	std::string attributeDefinition;
	std::string textureDefinition;
	std::string vertexCode;
	std::string fragmentCode;
};

class Compiler
{
private:
	enum class State
	{
		Continue,
		Stop
	};

	ArgumentParser argumentParser;
	Lexer lexer;
	Parser parser;
	Analyzer analyzer;
	GLSLEmitter glslEmitter;

	std::string _inputFile;
	std::string _outputFile;
	std::vector<std::string> _includeFolders;

	Expected<State> _parseArguments(int argc, char** argv)
	{
		try
		{
			argumentParser.parse(argc, argv);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << std::endl << std::endl;
			argumentParser.printHelp();
			return Expected<State>::Raise("Argument parsing failed: " + std::string(e.what()));
		}

		if (argumentParser.option("--help").activated == true)
		{
			argumentParser.printHelp();
			return State::Stop;
		}

		if (argumentParser.option("--verbose").activated == true)
		{
			std::cout << "Verbose mode is enabled." << std::endl;
			argumentParser.print();
		}

		_applyOptions();

		return State::Continue;
	}

	void _applyOptions()
	{
		if (argumentParser.option("--verbose").activated)
		{
			lexer.setVerbose(true);
			parser.setVerbose(true);
			analyzer.setVerbose(true);
			glslEmitter.setVerbose(true);
		}

		const ArgumentParser::Option& outputOption = argumentParser.option("--output");
		if (outputOption.activated)
		{
			if (outputOption.parameters.size() != 1)
			{
				throw std::runtime_error("Invalid number of parameters for --output option. Expected 1, got " + std::to_string(outputOption.parameters.size()));
			}
			_outputFile = outputOption.parameters[0];
		}
		else
		{
			_outputFile = "a.out";
		}

		const ArgumentParser::Option& includeFolderOption = argumentParser.option("--includeFolders");
		if (includeFolderOption.activated)
		{
			_includeFolders = includeFolderOption.parameters;
		}

		if (argumentParser.parameters().size() == 1)
		{
			_inputFile = argumentParser.parameters()[0];
		}
		else
		{
			throw std::runtime_error("Invalid number of input files. Expected 1, got " + std::to_string(argumentParser.parameters().size()));
		}
	}

	Expected<std::string> _readFile(const std::filesystem::path& filePath)
	{
		try
		{
			std::ifstream inputFile(filePath);
			if (!inputFile.is_open())
			{
				return Expected<std::string>::Raise("Could not open file: " + filePath.string());
			}
			return Expected<std::string>(std::string((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>()));
		}
		catch (const std::exception& e)
		{
			return Expected<std::string>::Raise("Error reading file: " + std::string(e.what()));
		}
	}

	Expected<ASTNode> _buildAST(const std::string& source)
	{
		auto tokenListResult = lexer.tokenize(source);
		if (tokenListResult.hasError)
		{
			return Expected<ASTNode>::Raise("Tokenization failed: " + tokenListResult.errorMessage);
		}

		auto astResult = parser.parse(tokenListResult.value);
		if (astResult.hasError)
		{
			return Expected<ASTNode>::Raise("Syntax parsing failed: " + astResult.errorMessage);
		}

		auto semanticResult = analyzer.analyze(astResult.value);
		if (semanticResult.hasError)
		{
			return Expected<ASTNode>::Raise("Semantic parsing failed: " + semanticResult.errorMessage);
		}

		return astResult;
	}

	Expected<CompilationUnit> _generate(const ASTNode& ast)
	{
		CompilationUnit compiledFile;

		return compiledFile;
	}

	Expected<std::nullptr_t> _saveCompilationUnit(const CompilationUnit& compiledFile, const std::filesystem::path& outputPath)
	{
		std::ofstream outputFile(outputPath);
		if (!outputFile.is_open())
		{
			return Expected<std::nullptr_t>::Raise(std::string("Could not open output file: ") + outputPath.string());
			
		}
		outputFile << "## LAYOUTS DEFINITION ##"<< std::endl;
		outputFile << compiledFile.layoutDefinition << std::endl;
		outputFile << std::endl;

		outputFile << "## FRAMEBUFFER DEFINITION ##"<< std::endl;
		outputFile << compiledFile.frameBufferDefinition << std::endl;
		outputFile << std::endl;
		
		outputFile << "## EXCEPTION DEFINITION ##"<< std::endl;
		outputFile << compiledFile.exceptionDefinition << std::endl;
		outputFile << std::endl;
		
		outputFile << "## CONSTANTS DEFINITION ##"<< std::endl;
		outputFile << compiledFile.constantDefinition << std::endl;
		outputFile << std::endl;
		
		outputFile << "## ATTRIBUTES DEFINITION ##"<< std::endl;
		outputFile << compiledFile.attributeDefinition << std::endl;
		outputFile << std::endl;
		
		outputFile << "## TEXTURES DEFINITION ##"<< std::endl;
		outputFile << compiledFile.textureDefinition << std::endl;
		outputFile << std::endl;
		
		outputFile << "## VERTEX SHADER CODE ##"<< std::endl;
		outputFile << compiledFile.vertexCode << std::endl;
		outputFile << std::endl;
		
		outputFile << "## FRAGMENT SHADER CODE ##"<< std::endl;
		outputFile << compiledFile.fragmentCode << std::endl;
		outputFile << std::endl;
		
		outputFile.close();

		return {};
	}

public:
	Compiler()
	{
		argumentParser.setUsage("Lumina inputFile.lum");
		argumentParser.addOption("--help", "-h", "Display this help message", true, 0);
		argumentParser.addOption("--verbose", "-v", "Enable verbose output", true, 0);
		argumentParser.addOption("--output", "-o", "Specify output file", true, 1);
		argumentParser.addOption("--includeFolders", "-i", "Specify additionnal include folders", true, 1);
	}

	int execute(int argc, char** argv)
	{
		auto state = _parseArguments(argc, argv);
		if (state.hasError == true || state.value == State::Stop)
		{
			return 0;
		}

		auto source = _readFile(_inputFile);
		if (source.hasError)
		{
			std::cerr << source.errorMessage << std::endl;
			return 1;
		}

		auto astNode = _buildAST(source.value);
		if (astNode.hasError)
		{
			std::cerr << astNode.errorMessage << std::endl;
			return 1;
		}

		auto compiledFile = _generate(astNode.value);
		if (compiledFile.hasError)
		{
			std::cerr << compiledFile.errorMessage << std::endl;
			return 1;
		}

		auto saveResult = _saveCompilationUnit(compiledFile.value, _outputFile);
		if (saveResult.hasError)
		{
			std::cerr << saveResult.errorMessage << std::endl;
			return 1;
		}

		return (0);
	}
};

int main(int argc, char** argv)
{
	Compiler program;

	auto result = program.execute(argc, argv);
	if (result != 0)
	{
		std::cerr << "An error occurred during execution." << std::endl;
		return result;
	}		
	return (0);
}
