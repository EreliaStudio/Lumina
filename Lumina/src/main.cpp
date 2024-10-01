#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

#define DEBUG_INFORMATION std::string(" ") + std::string(__FUNCTION__) + "::" + std::to_string(__LINE__)

namespace Lumina
{
	std::string readFileAsString(const std::filesystem::path& p_path)
	{
		std::fstream inputFile;
		inputFile.open(p_path, std::ios_base::in);

		std::string line;
		std::string result = "";
		while (std::getline(inputFile, line))
		{
			result += line + "\n";
		}

		inputFile.close();

		std::string tab = "\t";
		std::string spaces = "    "; // 4 spaces
		size_t pos = 0;

		while ((pos = result.find(tab, pos)) != std::string::npos)
		{
			result.replace(pos, tab.length(), spaces);
			pos += spaces.length();
		}

		return (result);
	}

	std::filesystem::path composeFilePath(const std::string& fileName, const std::vector<std::filesystem::path>& additionnalPaths = {})
	{
		std::string pathStr;

#ifdef _WIN32
		char* pathEnv = nullptr;
		size_t len = 0;
		if (_dupenv_s(&pathEnv, &len, "Path") != 0 || pathEnv == nullptr)
		{
			std::cerr << "PATH environment variable not found." << std::endl;
			return std::filesystem::path();
		}
		pathStr = std::string(pathEnv);
		free(pathEnv);
#else
		const char* pathEnv = std::getenv("PATH");
		if (!pathEnv)
		{
			std::cerr << "PATH environment variable not found." << std::endl;
			return std::filesystem::path();
		}
		pathStr = std::string(pathEnv);
#endif

		std::vector<std::filesystem::path> paths;
		std::stringstream ss(pathStr);
		std::string path;

#ifdef _WIN32
		const char delimiter = ';';
#else
		const char delimiter = ':';
#endif

		while (std::getline(ss, path, delimiter))
		{
			paths.push_back(path);
		}

		// Check in system PATH directories
		for (const auto& dir : paths)
		{
			std::filesystem::path filePath = dir / fileName;
			if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
			{
				return filePath;
			}
		}

		// Check in additional paths
		for (const auto& dir : additionnalPaths)
		{
			std::filesystem::path filePath = dir / fileName;
			if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
			{
				return filePath;
			}
		}

		// Check in the current directory
		std::filesystem::path currentDir = std::filesystem::current_path();
		std::filesystem::path filePath = currentDir / fileName;
		if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
		{
			return filePath;
		}

		return std::filesystem::path();
	}
}

namespace Lumina
{
	struct Token
	{
		struct Context
		{
			int line = 0;
			int column = 0;
			std::filesystem::path originFile = "";
			std::string inputLine = "";
		};

		enum class Type
		{
			Unknow,
			Include, // "#include"
			StringLitteral, // String contained inside a '"' on both sides
			IncludeLitteral, // String contained between '<' and '>'
			PipelineFlow, // "Input", "VertexPass", or "FragmentPass"
			PipelineFlowSeparator, // '->'
			NamespaceSeparator, // "::"
			Separator, // ':'
			Identifier, // Alphanumeric string
			Number, // Numeric value
			StructureBlock, // "struct"
			AttributeBlock, // "AttributeBlock"
			ConstantBlock, // "ConstantBlock"
			Texture, // "Texture"
			Namespace, // "namespace"
			OpenCurlyBracket, // '{'
			CloseCurlyBracket, // '}'
			OpenParenthesis, // '('
			CloseParenthesis, // ')'
			Accessor, // '.'
			Comment, // Comments: "//" until end of line or "/* */"
			Operator, // Operators: +, -, *, /, etc.
			ComparatorOperator, // Operators: <, >, >=, <=, == and !=.
			Return, // "return"
			Discard, // "discard"
			BoolStatement, // "true" or "false"
			ConditionOperator, // Operator || and &&
			IfStatement, // "if"
			WhileStatement, // "while"
			ElseStatement, // "else"
			EndOfSentence, // ';'
			Assignator, // '=', "+=", "-=" etc
			Incrementor, // '++', '--'
			Comma, // ','
			OpenBracket, // '['
			CloseBracket // ']'
		};

		Type type = Type::Unknow;
		std::string content = "";
		Context context;

		Token() = default;

		Token(const std::string& p_content, Type p_type, int p_line, int p_column, const std::filesystem::path& p_originFile, const std::string& p_inputLine) :
			type(p_type),
			content(p_content),
			context{ p_line, p_column, p_originFile, p_inputLine }
		{

		}

		Token(Type p_type) :
			type(p_type)
		{

		}

		Token(const std::string& p_content, Type p_type, const Context& p_context) :
			type(p_type),
			content(p_content),
			context(p_context)
		{

		}

		bool operator == (const std::string& p_string) const
		{
			return (content == p_string);
		}

		bool operator != (const std::string& p_string) const
		{
			return (content != p_string);
		}

		friend std::ostream& operator << (std::ostream& p_os, const Token::Type& p_type)
		{
			p_os << to_string(p_type);
			return p_os;
		}

		friend std::ostream& operator << (std::ostream& p_os, const Token& p_token)
		{
			p_os << "[" << std::setw(25) << p_token.type << "] | [" << std::setw(3) << p_token.context.line << "::" << std::left << std::setw(3) << p_token.context.column << std::right << "] | " << p_token.content;

			return (p_os);
		}

		static std::string to_string(Token::Type p_type)
		{
			switch (p_type)
			{
			case Token::Type::Unknow: return "Unknow";
			case Token::Type::Include: return "#include";
			case Token::Type::StringLitteral: return "StringLitteral";
			case Token::Type::IncludeLitteral: return "IncludeLitteral";
			case Token::Type::PipelineFlow: return "PipelineFlow";
			case Token::Type::PipelineFlowSeparator: return "PipelineFlowSeparator";
			case Token::Type::NamespaceSeparator: return "NamespaceSeparator";
			case Token::Type::Separator: return "Separator";
			case Token::Type::Identifier: return "Identifier";
			case Token::Type::Number: return "Number";
			case Token::Type::StructureBlock: return "StructureBlock";
			case Token::Type::AttributeBlock: return "AttributeBlock";
			case Token::Type::ConstantBlock: return "ConstantBlock";
			case Token::Type::Texture: return "Texture";
			case Token::Type::Namespace: return "Namespace";
			case Token::Type::OpenCurlyBracket: return "OpenCurlyBracket";
			case Token::Type::CloseCurlyBracket: return "CloseCurlyBracket";
			case Token::Type::OpenParenthesis: return "OpenParenthesis";
			case Token::Type::CloseParenthesis: return "CloseParenthesis";
			case Token::Type::OpenBracket: return "OpenBracket";
			case Token::Type::CloseBracket: return "CloseBracket";
			case Token::Type::Accessor: return "Accessor";
			case Token::Type::Comment: return "Comment";
			case Token::Type::BoolStatement: return "BoolStatement";
			case Token::Type::Operator: return "Operator";
			case Token::Type::ComparatorOperator: return "ComparatorOperator";
			case Token::Type::Return: return "Return";
			case Token::Type::Discard: return "Discard";
			case Token::Type::ConditionOperator: return "ConditionOperator";
			case Token::Type::IfStatement: return "IfStatement";
			case Token::Type::WhileStatement: return "WhileStatement";
			case Token::Type::ElseStatement: return "ElseStatement";
			case Token::Type::EndOfSentence: return "EndOfSentence";
			case Token::Type::Assignator: return "Assignator";
			case Token::Type::Incrementor: return "Incrementor";
			case Token::Type::Comma: return "Comma";
			default: return "Unknown";
			}
		}

		Lumina::Token operator + (const Lumina::Token& p_toAdd) const
		{
			Lumina::Token result = *this;

			result.append(p_toAdd);

			return (result);
		}
		Lumina::Token operator += (const Lumina::Token& p_toAdd)
		{
			append(p_toAdd);

			return (*this);
		}
		void append(const Lumina::Token& p_toAdd)
		{
			if (context.inputLine == "")
			{
				content = p_toAdd.content;
				context = p_toAdd.context;
			}
			else
			{
				if (p_toAdd.context.line != context.line)
				{
					context.inputLine += "\n" + p_toAdd.context.inputLine;
				}
				size_t tokenSize = p_toAdd.context.column + p_toAdd.content.size() - context.column;

				content = context.inputLine.substr(context.column, tokenSize);
			}
		}

		static Token merge(const std::vector<Token>& p_tokens, const Token::Type& p_type)
		{
			if (p_tokens.size() == 0)
				return (Token());

			size_t tokenColumn = p_tokens[0].context.column;
			size_t tokenSize = p_tokens.back().context.column + p_tokens.back().content.size() - p_tokens[0].context.column;

			return (Lumina::Token(
				p_tokens[0].context.inputLine.substr(tokenColumn, tokenSize),
				p_type,
				p_tokens[0].context
			));
		}
	};
}

namespace Lumina
{
	class TokenBasedError : public std::exception
	{
	private:
		std::string _what;

	public:
		TokenBasedError(const std::string& p_message, const Token& p_token)
		{
			size_t tokenSize = std::min(p_token.content.size(), p_token.context.inputLine.size());
			std::stringstream ss;

			ss << "In file [" << p_token.context.originFile.string() << "] : " << std::endl;

			// Check for invalid token (type == Unknow)
			if (p_token.type == Token::Type::Unknow)
			{
				ss << "    " << p_message << std::endl;
			}
			else
			{
				ss << "    Error on line[" << std::setw(5) << (p_token.context.line) << "] : " << p_message << std::endl;
				if (p_token.context.line == -1 || tokenSize == 0)
				{
					// Do nothing, no additional error information
				}
				else if (tokenSize == 1)
				{
					ss << std::string(14, ' ') << p_token.context.inputLine << std::endl;
					if (p_token.context.column != -1)
						ss << std::string(14, ' ') << std::string(p_token.context.column, ' ') << '|';
				}
				else
				{
					ss << std::string(14, ' ') << p_token.context.inputLine << std::endl;
					if (p_token.context.column != -1)
					{
						if (tokenSize == p_token.content.size())
							ss << std::string(14, ' ') << std::string(p_token.context.column, ' ') << '|' << std::string(tokenSize - 2, '-') << '|';
						else
							ss << std::string(14, ' ') << std::string(p_token.context.column, ' ') << '|' << std::string(tokenSize - 1, '-');
					}
				}
			}

			_what = ss.str();
		}

		virtual const char* what() const noexcept override
		{
			return (_what.c_str());
		}
	};

	template <typename TValueType>
	struct Expected
	{
		TValueType value;
		std::vector<TokenBasedError> errors;
	};

	template <>
	struct Expected<void>
	{
		std::vector<TokenBasedError> errors;
	};
}

namespace Lumina
{
	struct Tokenizer
	{
		static std::vector<Token> tokenize(const std::filesystem::path& p_path);
		static std::vector<Token> tokenizeString(const std::filesystem::path& p_path, const std::string& p_inputCode);
	};
}

namespace Lumina
{
	namespace
	{
		bool isIdentifierStart(char ch)
		{
			return std::isalpha(ch) || ch == '_';
		}

		bool isIdentifierChar(char ch)
		{
			return std::isalnum(ch) || ch == '_';
		}

		bool isDigit(char ch)
		{
			return std::isdigit(ch);
		}

		bool isHexDigit(char ch)
		{
			return std::isxdigit(ch);
		}

		std::string parseIdentifier(const std::string& code, size_t& index)
		{
			size_t start = index;
			while (index < code.size() && isIdentifierChar(code[index]))
			{
				index++;
			}
			return code.substr(start, index - start);
		}

		std::string parseNumber(const std::string& code, size_t& index)
		{
			size_t start = index;

			while (index < code.size() &&
				(isDigit(code[index]) ||
					code[index] == '.' ||
					code[index] == '-' ||
					code[index] == '+')
				)
			{
				index++;
			}
			if (index < code.size() && (code[index] == 'e' || code[index] == 'E'))
			{
				index++;
				if (code[index] == '+' || code[index] == '-')
				{
					index++;
				}
				while (index < code.size() && isDigit(code[index]))
				{
					index++;
				}
			}
			if (index < code.size() && code[index] == 'f')
			{
				index++;
			}
			return code.substr(start, index - start);
		}

		std::string parseStringLiteral(const std::string& code, size_t& index)
		{
			size_t start = index;
			index++; // Skip initial quote
			while (index < code.size() && (code[index] != '\"' || (index > 0 && code[index - 1] == '\\')))
			{
				index++;
			}
			index++; // Skip closing quote
			return code.substr(start, index - start);
		}

		std::string parseIncludeLiterals(const std::string& code, size_t& index)
		{
			size_t start = index;
			index++; // Skip initial '<'
			while (index < code.size() && code[index] != '>')
			{
				if (std::isspace(code[index]))
				{
					// If we encounter a space, this is not a valid input literal
					return "";
				}
				index++;
			}
			index++; // Skip closing '>'
			return code.substr(start, index - start);
		}

		std::string parseComment(const std::string& code, size_t& index)
		{
			size_t start = index;
			if (code[index + 1] == '/')
			{
				index += 2;
				while (index < code.size() && code[index] != '\n')
				{
					index++;
				}
			}
			else
			{
				index += 2;
				while (index + 1 < code.size() && !(code[index] == '*' && code[index + 1] == '/'))
				{
					index++;
				}
				index += 2;
			}
			return code.substr(start, index - start);
		}

		std::string parseSpecialToken(const std::string& code, size_t& index, const std::string& token)
		{
			index += token.size();
			return token;
		}
	}

	std::string getLine(const std::string& p_rawCode, const size_t& p_index)
	{
		size_t startIndex = p_index;
		size_t endIndex = p_index;
		while (startIndex > 0 && p_rawCode[startIndex - 1] != '\n')
		{
			startIndex--;
		}
		while (endIndex < p_rawCode.size() && p_rawCode[endIndex] != '\n')
		{
			endIndex++;
		}

		return (p_rawCode.substr(startIndex, endIndex - startIndex));
	}

	std::vector<Token> Tokenizer::tokenizeString(const std::filesystem::path& p_path, const std::string& p_inputCode)
	{
		std::vector<Token> result;
		size_t index = 0;
		int currentLineNumber = 0;
		int lineNumber = 1;
		int columnNumber = 0;
		std::string currentLine;

		while (index < p_inputCode.size())
		{
			if (currentLineNumber != lineNumber)
			{
				currentLine = getLine(p_inputCode, index);
				currentLineNumber = lineNumber;
			}

			if (std::isspace(p_inputCode[index]))
			{
				if (p_inputCode[index] == '\n')
				{
					lineNumber++;
					columnNumber = 0;
				}
				else if (p_inputCode[index] == '\t')
				{
					columnNumber += 4;
				}
				else
				{
					columnNumber++;
				}
				index++;
				continue;
			}

			std::string tokenStr;
			Token::Type tokenType = Token::Type::Unknow;
			int beginColumnNumber = columnNumber;
			int beginLineNumber = lineNumber;

			if (p_inputCode.substr(index, 8) == "#include")
			{
				tokenStr = parseSpecialToken(p_inputCode, index, "#include");
				tokenType = Token::Type::Include;
			}
			else if (p_inputCode[index] == '\"')
			{
				tokenStr = parseStringLiteral(p_inputCode, index);
				tokenType = Token::Type::StringLitteral;
			}
			else if (p_inputCode[index] == '/' && (p_inputCode[index + 1] == '/' || p_inputCode[index + 1] == '*'))
			{
				tokenStr = parseComment(p_inputCode, index);
				tokenType = Token::Type::Comment;
			}
			else if (p_inputCode[index] == '<')
			{
				size_t beginIndex = index;
				tokenStr = parseIncludeLiterals(p_inputCode, index);
				if (!tokenStr.empty())
				{
					tokenType = Token::Type::IncludeLitteral;
				}
				else
				{
					index = beginIndex;
					tokenStr = parseSpecialToken(p_inputCode, index, std::string(1, p_inputCode[index]));
					tokenType = Token::Type::ComparatorOperator;
				}
			}
			else if (isIdentifierStart(p_inputCode[index]))
			{
				tokenStr = parseIdentifier(p_inputCode, index);
				if (tokenStr == "Input" || tokenStr == "VertexPass" || tokenStr == "FragmentPass" || tokenStr == "Output")
				{
					tokenType = Token::Type::PipelineFlow;
				}
				else if (tokenStr == "struct")
				{
					tokenType = Token::Type::StructureBlock;
				}
				else if (tokenStr == "AttributeBlock")
				{
					tokenType = Token::Type::AttributeBlock;
				}
				else if (tokenStr == "ConstantBlock")
				{
					tokenType = Token::Type::ConstantBlock;
				}
				else if (tokenStr == "Texture")
				{
					tokenType = Token::Type::Texture;
				}
				else if (tokenStr == "namespace")
				{
					tokenType = Token::Type::Namespace;
				}
				else if (tokenStr == "if")
				{
					tokenType = Token::Type::IfStatement;
				}
				else if (tokenStr == "while")
				{
					tokenType = Token::Type::WhileStatement;
				}
				else if (tokenStr == "else")
				{
					tokenType = Token::Type::ElseStatement;
				}
				else if (tokenStr == "return")
				{
					tokenType = Token::Type::Return;
				}
				else if (tokenStr == "discard")
				{
					tokenType = Token::Type::Discard;
				}
				else if (tokenStr == "true" || tokenStr == "false")
				{
					tokenType = Token::Type::BoolStatement;
				}
				else
				{
					tokenType = Token::Type::Identifier;
				}
			}
			else if (isDigit(p_inputCode[index]) || ((p_inputCode[index] == '.' && isDigit(p_inputCode[index + 1])) ||
				(p_inputCode[index] == '+' && isDigit(p_inputCode[index + 1])) ||
				(p_inputCode[index] == '-' && isDigit(p_inputCode[index + 1]))))
			{
				tokenStr = parseNumber(p_inputCode, index);
				tokenType = Token::Type::Number;
			}
			else if (p_inputCode.substr(index, 2) == "->")
			{
				tokenStr = parseSpecialToken(p_inputCode, index, "->");
				tokenType = Token::Type::PipelineFlowSeparator;
			}
			else if (p_inputCode.substr(index, 2) == "::")
			{
				tokenStr = parseSpecialToken(p_inputCode, index, "::");
				tokenType = Token::Type::NamespaceSeparator;
			}
			else
			{
				// Handle operators and other tokens
				std::string operators[] = { "==", "!=", "<=", ">=" };
				bool foundOperator = false;
				for (const std::string& op : operators)
				{
					if (p_inputCode.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(p_inputCode, index, op);
						tokenType = Token::Type::ComparatorOperator;
						foundOperator = true;
						break;
					}
				}

				std::string conditionOperators[] = { "||", "&&" };
				bool foundConditionOperator = false;
				for (const std::string& op : conditionOperators)
				{
					if (p_inputCode.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(p_inputCode, index, op);
						tokenType = Token::Type::ConditionOperator;
						foundConditionOperator = true;
						break;
					}
				}

				std::string assignators[] = { "=", "+=", "-=", "*=", "/=", "%=" };
				bool foundAssignator = false;
				for (const std::string& op : assignators)
				{
					if (p_inputCode.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(p_inputCode, index, op);
						tokenType = Token::Type::Assignator;
						foundOperator = true;
						break;
					}
				}

				std::string incrementor[] = { "++", "--" };
				bool foundIncrementor = false;
				for (const std::string& op : incrementor)
				{
					if (p_inputCode.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(p_inputCode, index, op);
						tokenType = Token::Type::Incrementor;
						foundOperator = true;
						break;
					}
				}
				if (!foundOperator && !foundAssignator && !foundIncrementor && !foundConditionOperator)
				{
					switch (p_inputCode[index])
					{
					case '{':
						tokenStr = parseSpecialToken(p_inputCode, index, "{");
						tokenType = Token::Type::OpenCurlyBracket;
						break;
					case '}':
						tokenStr = parseSpecialToken(p_inputCode, index, "}");
						tokenType = Token::Type::CloseCurlyBracket;
						break;
					case '[':
						tokenStr = parseSpecialToken(p_inputCode, index, "[");
						tokenType = Token::Type::OpenBracket;
						break;
					case ']':
						tokenStr = parseSpecialToken(p_inputCode, index, "]");
						tokenType = Token::Type::CloseBracket;
						break;
					case '(':
						tokenStr = parseSpecialToken(p_inputCode, index, "(");
						tokenType = Token::Type::OpenParenthesis;
						break;
					case ')':
						tokenStr = parseSpecialToken(p_inputCode, index, ")");
						tokenType = Token::Type::CloseParenthesis;
						break;
					case '.':
						tokenStr = parseSpecialToken(p_inputCode, index, ".");
						tokenType = Token::Type::Accessor;
						break;
					case ';':
						tokenStr = parseSpecialToken(p_inputCode, index, ";");
						tokenType = Token::Type::EndOfSentence;
						break;
					case ':':
						tokenStr = parseSpecialToken(p_inputCode, index, ":");
						tokenType = Token::Type::Separator;
						break;
					case ',':
						tokenStr = parseSpecialToken(p_inputCode, index, ",");
						tokenType = Token::Type::Comma;
						break;
					case '<':
					case '>':
						tokenStr = parseSpecialToken(p_inputCode, index, std::string(1, p_inputCode[index]));
						tokenType = Token::Type::ComparatorOperator;
						break;
					case '+':
					case '-':
					case '*':
					case '/':
					case '%':
					case '!':
					case '&':
					case '|':
					case '^':
					case '~':
					case '?':
						tokenStr = parseSpecialToken(p_inputCode, index, std::string(1, p_inputCode[index]));
						tokenType = Token::Type::Operator;
						break;
					default:
						tokenStr = p_inputCode.substr(index, 1);
						index++;
						break;
					}
				}
			}

			// Calculate the line and column numbers
			for (char ch : tokenStr)
			{
				if (ch == '\n')
				{
					lineNumber++;
					columnNumber = 0;
					//currentLine.clear();
				}
				else
				{
					columnNumber++;
					//currentLine += ch;
				}
			}

			Token::Context context;

			context.column = beginColumnNumber;
			context.line = beginLineNumber;
			context.originFile = p_path;
			context.inputLine = currentLine;

			result.push_back(Token(tokenStr, tokenType, context));
			beginColumnNumber = columnNumber;
			beginLineNumber = lineNumber;
		}

		return result;
	}

	std::vector<Token> Tokenizer::tokenize(const std::filesystem::path& p_path)
	{
		return (tokenizeString(p_path, Lumina::readFileAsString(p_path)));
	}
}

namespace Lumina
{
	struct NameInfo
	{
		Lumina::Token value;
	};

	struct TypeInfo
	{
		std::vector<Lumina::Token> tokens;
	};

	struct ArrayInfo
	{
		std::vector<Lumina::Token> tokens;
	};

	struct VariableInfo
	{
		TypeInfo type;
		NameInfo name;
		ArrayInfo arraySizes;
	};

	struct PipelineFlowInfo
	{
		Lumina::Token input;
		Lumina::Token output;

		VariableInfo variable;
	};

	struct BlockInfo
	{
		NameInfo name;
		std::vector<VariableInfo> elements;
	};

	struct Instruction
	{
		enum class Type
		{
			VariableDeclaration,
			VariableAssignation,
			SymbolCall,
			IfStatement,
			WhileStatement,
			ReturnStatement,
			DiscardStatement
		};

		Type type;

		Instruction(const Type& p_type) :
			type(p_type)
		{

		}
	};

	struct SymbolBody
	{
		bool prototype = true;
		std::vector<std::shared_ptr<Instruction>> instructions;
	};

	struct NamespaceDesignation
	{
		std::vector<Lumina::Token> tokens;
	};

	struct Expression;

	struct VariableDesignation
	{
		struct Accessor
		{
			enum class Type
			{
				Array,
				Attribute
			};

			Type type;

			Accessor(Type p_type) :
				type(p_type)
			{

			}
		};

		struct ArrayAccessor : public Accessor
		{
			std::shared_ptr<Expression> expression;

			ArrayAccessor() :
				Accessor(Type::Array)
			{

			}
		};

		struct AttributeAccessor : public Accessor
		{
			NameInfo name;

			AttributeAccessor() :
				Accessor(Type::Attribute)
			{

			}
		};

		NamespaceDesignation nspace;
		NameInfo name;
		std::vector<std::shared_ptr<Accessor>> accessors;
	};

	struct Expression
	{
		struct Element
		{
			enum class Type
			{
				Number,
				Variable,
				Expression,
				Boolean,
				SymbolCall,
				Operator,
				Increment,
				ArrayDereferencement
			};

			Type type;

			Element(const Type& p_type) :
				type(p_type)
			{

			}
		};

		std::vector<std::shared_ptr<Element>> elements;

		struct NumberElement : public Expression::Element
		{
			Lumina::Token value;

			NumberElement() :
				Expression::Element(Type::Number)
			{

			}
		};

		struct VariableElement : public Expression::Element
		{
			VariableDesignation value;

			VariableElement() :
				Expression::Element(Type::Variable)
			{

			}
		};

		struct ExpressionElement : public Expression::Element
		{
			std::shared_ptr<Expression> value;

			ExpressionElement() :
				Expression::Element(Type::Expression)
			{

			}
		};

		struct BooleanElement : public Expression::Element
		{
			Lumina::Token value;

			BooleanElement() :
				Expression::Element(Type::Boolean)
			{

			}
		};

		struct SymbolCallElement : public Expression::Element
		{
			NamespaceDesignation nspace;
			NameInfo name;
			std::vector<std::shared_ptr<Expression>> parameters;

			SymbolCallElement() :
				Expression::Element(Type::SymbolCall)
			{

			}
		};

		struct IncrementElement : public Expression::Element
		{
			Lumina::Token value;

			IncrementElement() :
				Expression::Element(Type::Increment)
			{

			}
		};

		struct OperatorElement : public Expression::Element
		{
			Lumina::Token value;

			OperatorElement() :
				Expression::Element(Type::Operator)
			{

			}
		};

		struct ArrayDereferencementElement : public Expression::Element
		{
			std::vector<Expression::Element*> value;

			ArrayDereferencementElement() :
				Expression::Element(Type::ArrayDereferencement)
			{

			}
		};
	};

	struct VariableDeclaration : public Instruction
	{
		VariableInfo value;
		std::shared_ptr<Expression> initializer;

		VariableDeclaration() :
			Instruction(Instruction::Type::VariableDeclaration)
		{

		}
	};

	struct VariableAssignation : public Instruction
	{
		VariableDesignation variable;
		std::shared_ptr<Expression> value;

		VariableAssignation() :
			Instruction(Instruction::Type::VariableAssignation)
		{

		}
	};

	struct SymbolCall : public Instruction
	{
		NamespaceDesignation nspace;
		NameInfo name;
		std::vector<std::shared_ptr<Expression>> parameters;

		SymbolCall() :
			Instruction(Instruction::Type::SymbolCall)
		{

		}
	};

	struct ConditionnalBranch
	{
		std::shared_ptr<Expression> expression;
		SymbolBody body;
	};

	struct IfStatement : public Instruction
	{
		std::vector<ConditionnalBranch> conditonnalBranchs;

		IfStatement() :
			Instruction(Instruction::Type::IfStatement)
		{

		}
	};

	struct WhileStatement : public Instruction
	{
		std::shared_ptr<Expression> expression;
		SymbolBody body;

		WhileStatement() :
			Instruction(Instruction::Type::WhileStatement)
		{

		}
	};

	struct ReturnStatement : public Instruction
	{
		std::shared_ptr<Expression> value;

		ReturnStatement() :
			Instruction(Instruction::Type::ReturnStatement)
		{

		}
	};

	struct DiscardStatement : public Instruction
	{
		DiscardStatement() :
			Instruction(Instruction::Type::DiscardStatement)
		{

		}
	};

	struct FunctionInfo
	{
		struct ReturnType
		{
			TypeInfo type;
			ArrayInfo arraySizes;
		};

		ReturnType returnType;
		NameInfo name;
		std::vector<VariableInfo> parameters;
		SymbolBody body;
	};

	struct TextureInfo
	{
		NameInfo name;
	};

	struct NamespaceInfo
	{
		NameInfo name;

		std::vector<BlockInfo> structureBlock;
		std::vector<BlockInfo> attributeBlock;
		std::vector<BlockInfo> constantBlock;

		std::vector<TextureInfo> textures;

		std::vector<FunctionInfo> functions;

		std::vector<NamespaceInfo> innerNamespaces;
	};

	struct PipelineBodyInfo
	{
		SymbolBody body;
	};

	struct ShaderInfo
	{
		std::vector<PipelineFlowInfo> pipelineFlows;

		NamespaceInfo anonymNamespace;

		PipelineBodyInfo vertexPipelineBody;
		PipelineBodyInfo fragmentPipelineBody;
	};

	struct Parser
	{
		using Result = Lumina::Expected<ShaderInfo>;

		Result _result;
		std::vector<Token> _tokens;
		size_t _index;
		Token _errorToken;
		std::vector<NamespaceInfo*> _currentNamespace;

		TypeInfo parseTypeInfo()
		{
			TypeInfo result;

			if (currentToken().type == Token::Type::NamespaceSeparator)
			{
				result.tokens.push_back(currentToken());
			}

			while (tokenAtOffset(1).type == Token::Type::NamespaceSeparator)
			{
				result.tokens.push_back(expect(Token::Type::Identifier, "Expected a namespace name." + DEBUG_INFORMATION));
				result.tokens.push_back(expect(Token::Type::NamespaceSeparator, "Expected a namespace separator." + DEBUG_INFORMATION));
			}
			result.tokens.push_back(expect(Token::Type::Identifier, "Expected a type name." + DEBUG_INFORMATION));

			return (result);
		}

		ArrayInfo parseArrayInfo()
		{
			ArrayInfo result;

			while (currentToken().type == Token::Type::OpenBracket)
			{
				expect(Token::Type::OpenBracket, "Expected a '[' token." + DEBUG_INFORMATION);
				result.tokens.push_back(expect(Token::Type::Number, "Expected a array size token." + DEBUG_INFORMATION));
				expect(Token::Type::CloseBracket, "Expected a ']' token." + DEBUG_INFORMATION);
			}

			return (result);
		}

		NameInfo parseNameInfo()
		{
			NameInfo result;

			result.value = expect(Token::Type::Identifier, "Expected an identifier token." + DEBUG_INFORMATION);

			return (result);
		}

		VariableInfo parseVariableInfo()
		{
			VariableInfo result;

			result.type = parseTypeInfo();

			result.name = parseNameInfo();
			
			result.arraySizes = parseArrayInfo();

			return (result);
		}

		BlockInfo parseBlockInfo()
		{
			BlockInfo result;

			advance();

			result.name = parseNameInfo();
			expect(Lumina::Token::Type::OpenCurlyBracket, "Expected a '{' token." + DEBUG_INFORMATION);
			while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
			{
				result.elements.push_back(parseVariableInfo());
				expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token." + DEBUG_INFORMATION);
			}
			expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a '}' token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token." + DEBUG_INFORMATION);

			return (result);
		}

		bool describeVariableDeclaration()
		{
			size_t offset = 0;

			if (tokenAtOffset(offset).type == Lumina::Token::Type::NamespaceSeparator)
				offset++;

			while (tokenAtOffset(offset).type == Token::Type::Identifier && tokenAtOffset(offset + 1).type == Token::Type::NamespaceSeparator)
			{
				offset += 2;
			}

			return (tokenAtOffset(offset).type == Token::Type::Identifier && tokenAtOffset(offset + 1).type == Token::Type::Identifier);
		}

		bool describeVariableAssignation()
		{
			size_t offset = 0;

			if (tokenAtOffset(offset).type == Lumina::Token::Type::NamespaceSeparator)
				offset++;

			while (tokenAtOffset(offset).type == Token::Type::Identifier && tokenAtOffset(offset + 1).type == Token::Type::NamespaceSeparator)
			{
				offset += 2;
			}

			if (tokenAtOffset(offset).type != Token::Type::Identifier)
				return (false);

			offset++;

			while (tokenAtOffset(offset).type == Token::Type::Accessor && tokenAtOffset(offset + 1).type == Token::Type::NamespaceSeparator)
			{
				offset += 2;
			}

			return (tokenAtOffset(offset).type == Token::Type::Assignator);
		}

		std::shared_ptr<Expression::NumberElement> parseNumberElement()
		{
			std::shared_ptr<Expression::NumberElement> result = std::make_shared<Expression::NumberElement>();

			result->value = expect(Lumina::Token::Type::Number, "Expected a number token.");

			return (result);
		}

		std::shared_ptr<Expression::VariableElement> parseVariableElement()
		{
			std::shared_ptr<Expression::VariableElement> result = std::make_shared<Expression::VariableElement>();

			result->value = parseVariableDesignation();

			return (result);
		}

		std::shared_ptr<Expression::SymbolCallElement> parseSymbolCallElement()
		{
			std::shared_ptr<Expression::SymbolCallElement> result = std::make_shared<Expression::SymbolCallElement>();

			result->nspace = parseNamespaceDesignation();
			result->name = parseNameInfo();
			expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token.");
			while (currentToken().type != Token::Type::CloseParenthesis)
			{
				if (result->parameters.size() != 0)
				{
					expect(Lumina::Token::Type::Comma, "Expected a ',' token." + DEBUG_INFORMATION);
				}
				result->parameters.push_back(parseExpression());
			}
			expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token.");

			return (result);
		}

		std::shared_ptr<Expression::Element> parseExpressionElement()
		{
			if (currentToken().type == Token::Type::Number)
			{
				return (parseNumberElement());
			}
			if (currentToken().type == Token::Type::Identifier)
			{
				if (describeSymbolCall() == true)
				{
					return (parseSymbolCallElement());
				}
				else
				{
					return (parseVariableElement());
				}
			}
			
			throw TokenBasedError("Unknow expression element token." + DEBUG_INFORMATION, currentToken());

			return (nullptr);
		}

		std::shared_ptr<Expression::OperatorElement> parseExpressionOperatorElement()
		{
			std::shared_ptr<Expression::OperatorElement> result = std::make_shared<Expression::OperatorElement>();

			result->value = expect({ Token::Type::Operator, Token::Type::ComparatorOperator, Token::Type::ConditionOperator }, "Expected an operator token." + DEBUG_INFORMATION);

			return (result);
		}

		std::shared_ptr<Expression> parseExpression()
		{
			auto expression = std::make_shared<Expression>();
			
			if (currentToken() == "+" || currentToken() == "-")
			{
				expression->elements.push_back(parseExpressionOperatorElement());
			}
			expression->elements.push_back(parseExpressionElement());

			while (
					currentToken().type == Token::Type::Operator ||
					currentToken().type == Token::Type::ComparatorOperator ||
					currentToken().type == Token::Type::ConditionOperator
				)
			{
				expression->elements.push_back(parseExpressionOperatorElement());
				expression->elements.push_back(parseExpressionElement());
			}

			return expression;
		}

		std::shared_ptr<VariableDeclaration> parseVariableDeclaration()
		{
			std::shared_ptr<VariableDeclaration> result = std::make_shared<VariableDeclaration>();

			result->value = parseVariableInfo();
			if (currentToken().type != Lumina::Token::Type::EndOfSentence)
			{
				expect(Lumina::Token::Type::Assignator, "Expect a '=' token." + DEBUG_INFORMATION);
				result->initializer = parseExpression();
			}
			expect(Lumina::Token::Type::EndOfSentence, "Expect a ';' token." + DEBUG_INFORMATION);

			return (result);
		}

		NamespaceDesignation parseNamespaceDesignation()
		{
			NamespaceDesignation result;

			if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
				result.tokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Expected a namespace separator." + DEBUG_INFORMATION));

			while (currentToken().type == Token::Type::Identifier && tokenAtOffset(1).type == Token::Type::NamespaceSeparator)
			{
				result.tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected a namespace name." + DEBUG_INFORMATION));
				result.tokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Expected a namespace separator." + DEBUG_INFORMATION));
			}

			return (result);
		}

		std::shared_ptr<VariableDesignation::Accessor> parseAccessor()
		{
			if (currentToken().type == Token::Type::OpenBracket)
			{
				std::shared_ptr<VariableDesignation::ArrayAccessor> result = std::make_shared<VariableDesignation::ArrayAccessor>();

				expect(Token::Type::OpenBracket, "Expected a '[' token." + DEBUG_INFORMATION);
				result->expression = parseExpression();
				expect(Token::Type::CloseBracket, "Expected a ']' token." + DEBUG_INFORMATION);

				return (result);
			}
			else
			{
				std::shared_ptr<VariableDesignation::AttributeAccessor> result = std::make_shared<VariableDesignation::AttributeAccessor>();

				expect(Token::Type::Accessor, "Expected a '.' token." + DEBUG_INFORMATION);
				result->name = parseNameInfo();

				return (result);
			}
		}

		bool describeSymbolCall()
		{
			size_t offset = 0;

			if (tokenAtOffset(offset).type == Lumina::Token::Type::NamespaceSeparator)
				offset++;

			while (tokenAtOffset(offset).type == Token::Type::Identifier && tokenAtOffset(offset + 1).type == Token::Type::NamespaceSeparator)
			{
				offset += 2;
			}

			if (tokenAtOffset(offset).type != Token::Type::Identifier)
				return (false);

			offset++;

			return (tokenAtOffset(offset).type == Token::Type::OpenParenthesis);
		}

		std::shared_ptr<SymbolCall> parseSymbolCall()
		{
			std::shared_ptr<SymbolCall> result = std::make_shared<SymbolCall>();

			result->nspace = parseNamespaceDesignation();

			result->name = parseNameInfo();

			expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
			while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
			{
				if (result->parameters.size() != 0)
					expect(Lumina::Token::Type::Comma, "Expected a ',' token." + DEBUG_INFORMATION);
				result->parameters.push_back(parseExpression());
			}
			expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);

			return (result);
		}

		std::shared_ptr<ReturnStatement> parseReturn()
		{
			std::shared_ptr<ReturnStatement> result = std::make_shared<ReturnStatement>();

			expect(Lumina::Token::Type::Return, "Expected a 'return' token." + DEBUG_INFORMATION);
			result->value = parseExpression();
			expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token." + DEBUG_INFORMATION);

			return (result);
		}

		std::shared_ptr<DiscardStatement> parseDiscard()
		{
			std::shared_ptr<DiscardStatement> result = std::make_shared<DiscardStatement>();

			expect(Lumina::Token::Type::Discard, "Expected a 'discard' token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token." + DEBUG_INFORMATION);

			return (result);
		}

		std::shared_ptr<IfStatement> parseIfStatement()
		{
			std::shared_ptr<IfStatement> result = std::make_shared<IfStatement>();

			ConditionnalBranch ifBranch;

			expect(Lumina::Token::Type::IfStatement, "Expected a 'if' token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
			ifBranch.expression = parseExpression();
			expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);
			ifBranch.body = parseSymbolBody();

			result->conditonnalBranchs.push_back(ifBranch);

			while (currentToken().type == Lumina::Token::Type::ElseStatement && nextToken().type == Lumina::Token::Type::IfStatement)
			{
				ConditionnalBranch newBranch;

				expect(Lumina::Token::Type::ElseStatement, "Expected a 'else' token." + DEBUG_INFORMATION);
				expect(Lumina::Token::Type::IfStatement, "Expected a 'if' token." + DEBUG_INFORMATION);
				expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
				newBranch.expression = parseExpression();
				expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);
				newBranch.body = parseSymbolBody();

				result->conditonnalBranchs.push_back(newBranch);
			}

			if (currentToken().type == Lumina::Token::Type::ElseStatement)
			{
				ConditionnalBranch newBranch;

				expect(Lumina::Token::Type::ElseStatement, "Expected a 'else' token." + DEBUG_INFORMATION);
				newBranch.body = parseSymbolBody();

				result->conditonnalBranchs.push_back(newBranch);
			}
				
			return (result);
		}

		std::shared_ptr<WhileStatement> parseWhileStatement()
		{
			std::shared_ptr<WhileStatement> result = std::make_shared<WhileStatement>();

			expect(Lumina::Token::Type::WhileStatement, "Expected a 'while' token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
			result->expression = parseExpression();
			expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);
			result->body = parseSymbolBody();

			return (result);
		}

		SymbolBody parseSymbolBody()
		{
			SymbolBody result;

			result.prototype = false;

			expect(Lumina::Token::Type::OpenCurlyBracket, "Expected a '{' token." + DEBUG_INFORMATION);

			while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
			{
				try
				{
					switch (currentToken().type)
					{
					case Lumina::Token::Type::Identifier:
					{
						if (describeVariableDeclaration() == true)
						{
							result.instructions.push_back(parseVariableDeclaration());
							break;
						}
						else if (describeVariableAssignation() == true)
						{
							result.instructions.push_back(parseVariableAssignation());
							break;
						}
						else if (describeSymbolCall() == true)
						{
							result.instructions.push_back(parseSymbolCall());
							break;
						}
						else
						{
							throw TokenBasedError("Impossible to define the instruction type." + DEBUG_INFORMATION, currentToken());
						}
					}
					case Lumina::Token::Type::Return:
					{
						result.instructions.push_back(parseReturn());
						break;
					}
					case Lumina::Token::Type::Discard:
					{
						result.instructions.push_back(parseDiscard());
						break;
					}
					case Lumina::Token::Type::IfStatement:
					{
						result.instructions.push_back(parseIfStatement());
						break;
					}
					case Lumina::Token::Type::WhileStatement:
					{
						result.instructions.push_back(parseWhileStatement());
						break;
					}
					default:
						throw TokenBasedError("Unexpected token type [" + Lumina::Token::to_string(currentToken().type) + "]" + DEBUG_INFORMATION, currentToken());
					}
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
					skipLine();
				}
			}

			expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a '}' token." + DEBUG_INFORMATION);
			return (result);
		}

		Result _parse(const std::vector<Token>& p_tokens)
		{
			_result = Result();
			_tokens = p_tokens;
			_index = 0;
			_result.value.anonymNamespace.name.value = Lumina::Token("", Lumina::Token::Type::Identifier, 0, 0, _tokens[0].context.originFile, _tokens[0].context.inputLine);
			_currentNamespace.push_back(&(_result.value.anonymNamespace));

			while (hasTokenLeft() == true)
			{
				try
				{
					switch (currentToken().type)
					{
					case Token::Type::EndOfSentence:
					case Token::Type::Comment:
					{
						skipToken();
						break;
					}
					case Token::Type::Include:
					{
						parseInclude();
						break;
					}
					case Token::Type::PipelineFlow:
					{
						if (tokenAtOffset(1).type == Token::Type::PipelineFlowSeparator)
						{
							parsePipelineFlow();
							break;
						}
						else
						{
							parsePipelineBody();
							break;
						}
					}
					case Token::Type::Identifier:
					{
						parseFunction();
						break;
					}
					case Token::Type::StructureBlock:
					{
						parseStructureBlock();
						break;
					}
					case Token::Type::AttributeBlock:
					{
						parseAttributeBlock();
						break;
					}
					case Token::Type::ConstantBlock:
					{
						parseConstantBlock();
						break;
					}
					case Token::Type::Texture:
					{
						parseTexture();
						break;
					}
					case Token::Type::Namespace:
					{
						parseNamespace();
						break;
					}
					default:
						throw TokenBasedError("Unexpected token type [" + Lumina::Token::to_string(currentToken().type) + "]" + DEBUG_INFORMATION, currentToken());
					}
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
					skipLine();
				}
			}

			return (_result);
		}

		VariableDesignation parseVariableDesignation()
		{
			VariableDesignation result;

			result.nspace = parseNamespaceDesignation();
			result.name = parseNameInfo();
			while (currentToken().type == Lumina::Token::Type::OpenBracket ||
				currentToken().type == Lumina::Token::Type::Accessor)
			{
				result.accessors.push_back(parseAccessor());
			}

			return (result);
		}

		std::shared_ptr<VariableAssignation> parseVariableAssignation()
		{
			std::shared_ptr<VariableAssignation> result = std::make_shared<VariableAssignation>();

			result->variable = parseVariableDesignation();
			expect(Lumina::Token::Type::Assignator, "Expect a '=' token." + DEBUG_INFORMATION);
			result->value = parseExpression();
			expect(Lumina::Token::Type::EndOfSentence, "Expect a ';' token." + DEBUG_INFORMATION);

			return (result);
		}

		void parsePipelineBody()
		{
			PipelineBodyInfo result;

			expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);

			result.body = parseSymbolBody();
		}

		void parseFunction()
		{
			FunctionInfo newFunction;

			FunctionInfo::ReturnType returnType;

			returnType.type = parseTypeInfo();
			returnType.arraySizes = parseArrayInfo();

			newFunction.returnType = returnType;
			newFunction.name = parseNameInfo();
			expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
			while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
			{
				if (newFunction.parameters.size() != 0)
					expect(Lumina::Token::Type::Comma, "Expected a ',' token." + DEBUG_INFORMATION);
				newFunction.parameters.push_back(parseVariableInfo());
			}
			expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);

			if (currentToken().type == Lumina::Token::Type::EndOfSentence)
			{
				expect(Lumina::Token::Type::EndOfSentence, "Expect a ';' token." + DEBUG_INFORMATION);
			}
			else
			{
				newFunction.body = parseSymbolBody();
			}

			_currentNamespace.back()->functions.push_back(newFunction);
		}

		void parseTexture()
		{
			TextureInfo newTexture;

			expect(Lumina::Token::Type::Texture, "Expected a texture keyword." + DEBUG_INFORMATION);
			newTexture.name = parseNameInfo();
			expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token." + DEBUG_INFORMATION);

			_currentNamespace.back()->textures.push_back(newTexture);
		}

		void parsePipelineFlow()
		{
			PipelineFlowInfo newPipelineFlow;

			newPipelineFlow.input = expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::PipelineFlowSeparator, "Expected a '->' token." + DEBUG_INFORMATION);
			newPipelineFlow.output = expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::Separator, "Expected a ':' token." + DEBUG_INFORMATION);
			newPipelineFlow.variable = parseVariableInfo();
			expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token." + DEBUG_INFORMATION);
		}

		void parseStructureBlock()
		{
			_currentNamespace.back()->structureBlock.push_back(parseBlockInfo());
		}

		void parseAttributeBlock()
		{
			_currentNamespace.back()->attributeBlock.push_back(parseBlockInfo());
		}

		void parseConstantBlock()
		{
			_currentNamespace.back()->constantBlock.push_back(parseBlockInfo());
		}

		void parseNamespace()
		{
			advance();

			NamespaceInfo newNamespace;

			newNamespace.name = parseNameInfo();

			_result.value.anonymNamespace.innerNamespaces.push_back(newNamespace);

			_currentNamespace.push_back(&(_result.value.anonymNamespace.innerNamespaces.back()));

			expect(Lumina::Token::Type::OpenCurlyBracket, "Expected a '{' token." + DEBUG_INFORMATION);
		
			while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
			{
				try
				{
					switch (currentToken().type)
					{
					case Token::Type::EndOfSentence:
					case Token::Type::Comment:
					{
						skipToken();
						break;
					}
					case Token::Type::Identifier:
					{
						parseFunction();
						break;
					}
					case Token::Type::StructureBlock:
					{
						parseStructureBlock();
						break;
					}
					case Token::Type::AttributeBlock:
					{
						parseAttributeBlock();
						break;
					}
					case Token::Type::ConstantBlock:
					{
						parseConstantBlock();
						break;
					}
					case Token::Type::Texture:
					{
						parseTexture();
						break;
					}
					case Token::Type::Namespace:
					{
						parseNamespace();
						break;
					}
					default:
						throw TokenBasedError("Unexpected token type [" + Lumina::Token::to_string(currentToken().type) + "] inside namespace [" + newNamespace.name.value.content + "]" + DEBUG_INFORMATION, currentToken());
					}
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
					skipLine();
				}
			}
			expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a '}' token." + DEBUG_INFORMATION);

			_currentNamespace.pop_back();
		}

		void parseInclude()
		{
			const Token& includeToken = currentToken();
			advance();

			const Token& pathToken = expect(
				{ Token::Type::IncludeLitteral, Token::Type::StringLitteral },
				"Expected a file path after #include" + DEBUG_INFORMATION);

			if (pathToken.content.length() < 2)
			{
				throw TokenBasedError("Invalid include path." + DEBUG_INFORMATION, pathToken);
			}
			std::string rawPath = pathToken.content.substr(1, pathToken.content.size() - 2);

			std::filesystem::path includePath = composeFilePath(rawPath, { includeToken.context.originFile.parent_path() });

			if (!std::filesystem::exists(includePath))
			{
				throw TokenBasedError("Included file [" + includePath.string() + "] not found." + DEBUG_INFORMATION, pathToken);
			}

			std::vector<Token> includedTokens = Tokenizer::tokenize(includePath);

			Parser includedParser;
			Result includedResult = includedParser._parse(includedTokens);

			_result.errors.insert(_result.errors.end(), includedResult.errors.begin(), includedResult.errors.end());

			if ((includedResult.value.vertexPipelineBody.body.prototype == false) ||
				(includedResult.value.fragmentPipelineBody.body.prototype == false))
			{
				throw TokenBasedError("Included file [" + includePath.string() + "] can't contain pipeline body." + DEBUG_INFORMATION, pathToken);
			}
		}

		bool hasTokenLeft(const size_t& p_offset = 0) const
		{
			return ((_index + p_offset) < _tokens.size());
		}

		void advance()
		{
			_index++;
		}

		const Lumina::Token& currentToken() const
		{
			if (hasTokenLeft() == false)
				return (_errorToken);
			return (_tokens[_index]);
		}

		const Lumina::Token& nextToken() const
		{
			return (tokenAtOffset(1));
		}

		const Lumina::Token& tokenAtOffset(size_t p_offset) const
		{
			if (hasTokenLeft(p_offset) == false)
				return (_errorToken);
			return (_tokens[_index + p_offset]);
		}

		const Lumina::Token& expect(const Lumina::Token::Type& p_expectedType, const std::string& p_errorMessage = "Invalid token type")
		{
			if (hasTokenLeft() == false)
				throw TokenBasedError("Unexpected end of file.", _errorToken);
			if (currentToken().type != p_expectedType)
				throw TokenBasedError(p_errorMessage, currentToken());
			const Lumina::Token& result = currentToken();
			advance();
			return (result);
		}

		const Lumina::Token& expect(const std::vector<Lumina::Token::Type>& p_expectedTypes, const std::string& p_errorMessage = "Invalid token type")
		{
			if (hasTokenLeft() == false)
				throw TokenBasedError("Unexpected end of file.", _errorToken);

			bool found = false;

			for (const auto& type : p_expectedTypes)
			{
				if (currentToken().type == type)
					found = true;
			}

			if (found == false)
				throw TokenBasedError(p_errorMessage, currentToken());
			const Lumina::Token& result = currentToken();
			advance();
			return (result);
		}

		void skipToken()
		{
			advance();
		}

		void skipLine()
		{
			size_t currentLine = currentToken().context.line;

			while (currentToken().context.line == currentLine)
			{
				advance();
			}
		}

		static Result parse(const std::vector<Token>& p_tokens)
		{
			return (Parser()._parse(p_tokens));
		}
	};
}

#include <set>
#include <map>

namespace Lumina
{
	struct Type;

	struct Variable
	{
		const Type* type;
		std::string name;
		std::vector<size_t> arraySizes;

		bool operator<(const Variable& p_other) const
		{
			return name < p_other.name;
		}
	};

	struct Type
	{
		std::string name;
		std::set<Variable> attributes;

		bool operator<(const Type& p_other) const
		{
			return name < p_other.name;
		}
	};
}

namespace Lumina
{
	struct SemanticChecker
	{
	public:
		using Result = Expected<void>;

	private:
		Result _result;
		std::vector<std::string> _currentNamespaces;

		std::set<Type> _standardTypes;
		std::set<Type> _types;
		std::set<Type> _attributeTypes;
		std::set<Type> _constantTypes;

		std::set<Variable> _vertexVariables;
		std::set<Variable> _fragmentVariables;

		const Type* lookupTypeInNamespace(const std::string& typeName)
		{
			auto it = _types.find(Type{ typeName });
			if (it != _types.end())
			{
				return &(*it);
			}
			return nullptr;
		}

		const Type* getType(const Lumina::Token& p_nameToken)
		{
			const Type* type = lookupTypeInNamespace(p_nameToken.content);
			if (type != nullptr)
			{
				return type;
			}

			for (size_t i = 0; i < _currentNamespaces.size(); ++i)
			{
				std::string qualifiedName;
				for (size_t j = 0; j <= i; ++j)
				{
					if (!qualifiedName.empty())
					{
						qualifiedName += "::";
					}
					qualifiedName += _currentNamespaces[j];
				}
				qualifiedName += "::" + p_nameToken.content;

				type = lookupTypeInNamespace(qualifiedName);
				if (type != nullptr)
				{
					return type;
				}
			}

			throw TokenBasedError("Type '" + p_nameToken.content + "' not found in the current scope.", p_nameToken);
		}

		const Type* getType(const std::vector<Lumina::Token>& p_nameTokens)
		{
			Lumina::Token mergedToken = Lumina::Token::merge(p_nameTokens, Lumina::Token::Type::Identifier);

			return getType(mergedToken);
		}

		void parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow)
		{
			if (p_pipelineFlow.variable.arraySizes.tokens.size() != 0)
			{
				throw TokenBasedError("Pipeline flow variable cannot be array variable." + DEBUG_INFORMATION, p_pipelineFlow.variable.name.value);
			}
		}

		Variable parseVariable(const VariableInfo& p_variableInfo)
		{
			Variable result;

			result.name = p_variableInfo.name.value.content;
			result.type = getType(p_variableInfo.type.tokens);
			for (const auto& token : p_variableInfo.arraySizes.tokens)
			{
				int arraySize = std::stoi(token.content);

				if (arraySize <= 0)
				{
					throw TokenBasedError("Array size must be greater than 0. Invalid size found.", token);
				}

				result.arraySizes.push_back(static_cast<size_t>(arraySize));
			}

			return (result);
		}

		Type parseBlockInfo(const BlockInfo& p_block)
		{
			Type result;

			result.name = p_block.name.value.content;

			for (const auto& element : p_block.elements)
			{
				Variable newAttribute = parseVariable(element);

				if (result.attributes.contains(newAttribute) == true)
				{
					throw TokenBasedError("Attribute [" + newAttribute.name + "] already defined in structure [" + result.name + "].", element.name.value);
				}
			}

			if (_types.contains(result) == true)
			{
				throw TokenBasedError("Type [" + p_block.name.value.content + "] already defined.", p_block.name.value);
			}

			return (result);
		}

		void addStandardType(const Type& p_type)
		{
			_standardTypes.insert(p_type);
			_types.insert(p_type);
		}

		void addStructure(const Type& p_type)
		{
			_types.insert(p_type);
		}

		void addAttribute(const Type& p_type)
		{
			_types.insert(p_type);
			_attributeTypes.insert(p_type);

			_vertexVariables.insert({ lookupTypeInNamespace(p_type.name), p_type.name + "_Value", {} });
			_fragmentVariables.insert({ lookupTypeInNamespace(p_type.name), p_type.name + "_Value", {} });
		}

		void addConstant(const Type& p_type)
		{
			_types.insert(p_type);
			_constantTypes.insert(p_type);

			_vertexVariables.insert({ lookupTypeInNamespace(p_type.name), p_type.name + "_Value", {} });
			_fragmentVariables.insert({ lookupTypeInNamespace(p_type.name), p_type.name + "_Value", {} });
		}

		void parseTexture(const TextureInfo& p_texture)
		{

		}

		void parseFunction(const FunctionInfo& p_function)
		{

		}

		void parseNamespace(const NamespaceInfo& p_namespace)
		{
			_currentNamespaces.push_back(p_namespace.name.value.content);

			/*
			struct NamespaceInfo
			{
				NameInfo name;

				std::vector<BlockInfo> structureBlock;
				std::vector<BlockInfo> attributeBlock;
				std::vector<BlockInfo> constantBlock;

				std::vector<TextureInfo> textures;

				std::vector<FunctionInfo> functions;

				std::vector<NamespaceInfo> innerNamespaces;
			}
			*/
			for (const auto& blockInfo : p_namespace.structureBlock)
			{

				try
				{
					addStructure(parseBlockInfo(blockInfo));
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}

			}
			for (const auto& blockInfo : p_namespace.attributeBlock)
			{
				try
				{
					addAttribute(parseBlockInfo(blockInfo));
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}
			for (const auto& blockInfo : p_namespace.constantBlock)
			{
				try
				{
					addConstant(parseBlockInfo(blockInfo));
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			for (const auto& textureInfo : p_namespace.textures)
			{
				try
				{
					parseTexture(textureInfo);
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			for (const auto& functionInfo : p_namespace.functions)
			{
				try
				{
					parseFunction(functionInfo);
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			for (const auto& innerNamespace : p_namespace.innerNamespaces)
			{
				try
				{
					parseNamespace(innerNamespace);
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			_currentNamespaces.pop_back();
		}

		void parsePipelineBody(const PipelineBodyInfo& p_pipelineBody)
		{

		}

		Result _parse(const ShaderInfo& p_shaderInfo)
		{
			_result = Result();

			/*
			struct ShaderInfo
			{
				std::vector<PipelineFlowInfo> pipelineFlows;

				NamespaceInfo anonymNamespace;

				PipelineBodyInfo vertexPipelineBody;
				PipelineBodyInfo fragmentPipelineBody;
			};
			*/

			

			for (const auto& pipelineFlow : p_shaderInfo.pipelineFlows)
			{
				try
				{
					parsePipelineFlow(pipelineFlow);
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			try
			{
				parseNamespace(p_shaderInfo.anonymNamespace);
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}


			try
			{
				parsePipelineBody(p_shaderInfo.vertexPipelineBody);
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}


			try
			{
				parsePipelineBody(p_shaderInfo.fragmentPipelineBody);
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}


			return (_result);
		}

		void addScalarTypes()
		{
			addStandardType(
				{
					.name = "bool",
					.attributes = {}
				});

			addStandardType(
				{
					.name = "int",
					.attributes = {}
				});

			addStandardType(
				{
					.name = "float",
					.attributes = {}
				});

			addStandardType(
				{
					.name = "uint",
					.attributes = {}
				});
		}

		void addVector2Types()
		{
			addStandardType(
				{
					.name = "Vector2",
					.attributes = {
						{
							.type = lookupTypeInNamespace("float"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("float"),
							.name = "y",
							.arraySizes = {}
						}
					}
				});

			addStandardType(
				{
					.name = "Vector2Int",
					.attributes = {
						{
							.type = lookupTypeInNamespace("int"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("int"),
							.name = "y",
							.arraySizes = {}
						}
					}
				});

			addStandardType(
				{
					.name = "Vector2UInt",
					.attributes = {
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "y",
							.arraySizes = {}
						}
					}
				});
		}

		void addVector3Types()
		{
			addStandardType(
				{
					.name = "Vector3",
					.attributes = {
						{
							.type = lookupTypeInNamespace("float"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("float"),
							.name = "y",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("float"),
							.name = "z",
							.arraySizes = {}
						}
					}
				});

			addStandardType(
				{
					.name = "Vector3Int",
					.attributes = {
						{
							.type = lookupTypeInNamespace("int"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("int"),
							.name = "y",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("int"),
							.name = "z",
							.arraySizes = {}
						}
					}
				});

			addStandardType(
				{
					.name = "Vector3UInt",
					.attributes = {
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "y",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "z",
							.arraySizes = {}
						}
					}
				});
		}

		void addVector4Types()
		{
			addStandardType(
				{
					.name = "Vector4",
					.attributes = {
						{
							.type = lookupTypeInNamespace("float"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("float"),
							.name = "y",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("float"),
							.name = "z",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("float"),
							.name = "w",
							.arraySizes = {}
						}
					}
				});

			addStandardType(
				{
					.name = "Vector4Int",
					.attributes = {
						{
							.type = lookupTypeInNamespace("int"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("int"),
							.name = "y",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("int"),
							.name = "z",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("int"),
							.name = "w",
							.arraySizes = {}
						}
					}
				});

			addStandardType(
				{
					.name = "Vector4UInt",
					.attributes = {
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "x",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "y",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "z",
							.arraySizes = {}
						},
						{
							.type = lookupTypeInNamespace("uint"),
							.name = "w",
							.arraySizes = {}
						}
					}
				});
		}

		void addMatrixTypes()
		{
			addStandardType(
				{
					.name = "Matrix2x2",
					.attributes = {}  // No attributes
				});

			addStandardType(
				{
					.name = "Matrix3x3",
					.attributes = {}  // No attributes
				});

			addStandardType(
				{
					.name = "Matrix4x4",
					.attributes = {}  // No attributes
				});
		}

		SemanticChecker()
		{
			addScalarTypes();
			addVector2Types();
			addVector3Types();
			addVector4Types();
			addMatrixTypes();
		}


	public:
		static Result parse(const ShaderInfo& p_shaderInfo)
		{
			return (SemanticChecker()._parse(p_shaderInfo));
		}
	};
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1]);

	Lumina::Parser::Result instruction = Lumina::Parser::parse(tokens);

	if (instruction.errors.size() != 0)
	{
		for (const auto& error : instruction.errors)
		{
			std::cout << error.what() << std::endl;
		}

		return (-1);
	}

	Lumina::SemanticChecker::Result semanticErrors = Lumina::SemanticChecker::parse(instruction.value);

	if (semanticErrors.errors.size() != 0)
	{
		for (const auto& error : semanticErrors.errors)
		{
			std::cout << error.what() << std::endl;
		}

		return (-1);
	}

	return (0);
}
