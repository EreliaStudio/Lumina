#include "argument_parser.hpp"

#include <string>
#include <cstddef>

#include <unordered_map>
#include <optional>

struct SourceLocation;

class SourceManager
{
public:
    struct FileID { size_t value; };

private:
	std::vector<std::filesystem::path> _includeFolderPaths;

	static constexpr char pathSeparator()
    {
    #ifdef _WIN32
        return ';';
    #else
        return ':';
    #endif
    }


public:
	SourceManager()
	{
		_includeFolderPaths.push_back(".");

        if (const char* env = std::getenv("PATH"))
        {
            std::stringstream ss{env};
            std::string token;
            const char sep = pathSeparator();

            while (std::getline(ss, token, sep))
            {
                if (!token.empty())
                    _includeFolderPaths.emplace_back(token);
            }
        }
	}

	void addIncludeFolder(const std::filesystem::path& includeFolderPath)
	{
		_includeFolderPaths.push_back(includeFolderPath);
	}

	std::filesystem::path getFileFullPath(const std::filesystem::path& includeFile)
	{
		if (std::filesystem::exists(includeFile))
		{
            return std::filesystem::canonical(includeFile);
		}

        for (const auto& dir : _includeFolderPaths)
        {
            std::filesystem::path candidate = dir / includeFile;
            if (std::filesystem::exists(candidate))
			{
                return std::filesystem::canonical(candidate);
			}
        }

        throw std::runtime_error("SourceManager: could not locate file \"" + includeFile.string() + "\" in any include path.");
	}
};

struct SourceLocation
{
    SourceManager::FileID file;
    std::size_t offset;
};

#include <vector>

struct Token
{
	enum class Type
	{
		EndOfFile,
		Identifier,
		IntLiteral,
		FloatLiteral,
		BoolLiteral,
		StringLiteral,

		KwStruct,
		KwNamespace,
		KwAttributeBlock,
		KwConstantBlock,
		KwTexture,
		KwInput,
		KwVertexPass,
		KwFragmentPass,
		KwOutput,
		KwRaiseException,
		KwDiscard,
		KwIf,
		KwElse,
		KwWhile,
		KwDo,
		KwReturn,
		KwInclude,

		Plus,
		Minus,
		Star,
		Slash,
		Percent,
		PlusEqual,
		MinusEqual,
		StarEqual,
		SlashEqual,
		PercentEqual,
		Increment,
		Decrement,
		Equal,
		EqualEqual,
		NotEqual,
		Less,
		Greater,
		LessEqual,
		GreaterEqual,
		LogicalAnd,
		LogicalOr,
		LogicalNot,
		Arrow,
		Colon,
		DoubleColon,
		Comma,
		Semicolon,
		Dot,
		LeftParen,
		RightParen,
		LeftBrace,
		RightBrace,
		LeftBracket,
		RightBracket,
		Hash,
		
		Unknown
	};

	Type type;
	std::string lexeme;
	size_t row;
	size_t col;
	SourceLocation location;

	static std::string to_string(Type type)
	{
		switch (type)
		{
			// — literals & identifiers —
			case Type::EndOfFile        : return "EndOfFile";
			case Type::Identifier       : return "Identifier";
			case Type::IntLiteral       : return "IntLiteral";
			case Type::FloatLiteral     : return "FloatLiteral";
			case Type::BoolLiteral      : return "BoolLiteral";
			case Type::StringLiteral    : return "StringLiteral";

			// — keywords —
			case Type::KwStruct         : return "Struct";
			case Type::KwNamespace      : return "Namespace";
			case Type::KwAttributeBlock : return "AttributeBlock";
			case Type::KwConstantBlock  : return "ConstantBlock";
			case Type::KwTexture        : return "Texture";
			case Type::KwInput          : return "Input";
			case Type::KwVertexPass     : return "VertexPass";
			case Type::KwFragmentPass   : return "FragmentPass";
			case Type::KwOutput         : return "Output";
			case Type::KwRaiseException : return "RaiseException";
			case Type::KwDiscard        : return "Discard";
			case Type::KwIf             : return "If";
			case Type::KwElse           : return "Else";
			case Type::KwWhile          : return "While";
			case Type::KwDo             : return "Do";
			case Type::KwReturn         : return "Return";
			case Type::KwInclude        : return "Include";

			// — operators & punctuation —
			case Type::Plus             : return "Plus";
			case Type::Minus            : return "Minus";
			case Type::Star             : return "Star";
			case Type::Slash            : return "Slash";
			case Type::Percent          : return "Percent";
			case Type::PlusEqual        : return "PlusEqual";
			case Type::MinusEqual       : return "MinusEqual";
			case Type::StarEqual        : return "StarEqual";
			case Type::SlashEqual       : return "SlashEqual";
			case Type::PercentEqual     : return "PercentEqual";
			case Type::Increment        : return "Increment";
			case Type::Decrement        : return "Decrement";
			case Type::Equal            : return "Equal";
			case Type::EqualEqual       : return "EqualEqual";
			case Type::NotEqual         : return "NotEqual";
			case Type::Less             : return "Less";
			case Type::Greater          : return "Greater";
			case Type::LessEqual        : return "LessEqual";
			case Type::GreaterEqual     : return "GreaterEqual";
			case Type::LogicalAnd       : return "LogicalAnd";
			case Type::LogicalOr        : return "LogicalOr";
			case Type::LogicalNot       : return "LogicalNot";
			case Type::Arrow            : return "Arrow";
			case Type::Colon            : return "Colon";
			case Type::DoubleColon      : return "DoubleColon";
			case Type::Comma            : return "Comma";
			case Type::Semicolon        : return "Semicolon";
			case Type::Dot              : return "Dot";
			case Type::LeftParen        : return "LeftParen";
			case Type::RightParen       : return "RightParen";
			case Type::LeftBrace        : return "LeftBrace";
			case Type::RightBrace       : return "RightBrace";
			case Type::LeftBracket      : return "LeftBracket";
			case Type::RightBracket     : return "RightBracket";
			case Type::Hash             : return "Hash";

			default                     : return "Unknown";
		}
	}
};

using TokenList = std::vector<Token>;

template<typename TType>
struct Expected
{
	TType value;
	bool hasError;
	Token token;
	std::string errorMessage;

	Expected() : hasError(false) {}
	Expected(const TType& value) : value(value), hasError(false) {}

	static Expected<TType> Raise(const Token& token, const std::string& errorMessage)
	{
		Expected<TType> result;

		result.hasError = true;
		result.token = token;
		result.errorMessage = errorMessage;

		return result;
	}

	static Expected<TType> Raise(const std::string& errorMessage)
	{
		return Raise(Token{ Token::Type::Unknown, "", 0, 0, {} }, errorMessage);
	}
};

class Lexer
{
public:
	using TokenListResult = Expected<TokenList>;

private:
	SourceManager& sourceManager;
	bool _isVerbose = false;

public:
	Lexer(SourceManager& sourceManager) : sourceManager(sourceManager) {}
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

class SemanticAnalyzer
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
	SourceManager sourceManager;
	Lexer lexer;
	Parser parser;
	SemanticAnalyzer analyzer;
	GLSLEmitter glslEmitter;

	std::string _inputFile;
	std::string _outputFile;

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
			for (const auto& path : includeFolderOption.parameters)
			{
				sourceManager.addIncludeFolder(path);
			}
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
	Compiler() :
		argumentParser(),
		lexer(sourceManager),
		parser(),
		analyzer(),
		glslEmitter()
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
	Compiler compiler;

	auto result = compiler.execute(argc, argv);
	if (result != 0)
	{
		std::cerr << "An error occurred during execution." << std::endl;
		return result;
	}		
	return (0);
}
