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
			CloseBracket, // ']'
			Expression
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

		Lumina::Token tokens() const
		{
			return (value);
		}
	};

	struct TypeInfo
	{
		std::vector<Lumina::Token> typeTokens;

		Lumina::Token tokens() const
		{
			return (Lumina::Token::merge(typeTokens, Lumina::Token::Type::Identifier));
		}
	};

	struct ArrayInfo
	{
		std::vector<Lumina::Token> arrayTokens;

		Lumina::Token tokens() const
		{
			return (Lumina::Token::merge(arrayTokens, Lumina::Token::Type::Identifier));
		}
	};

	struct VariableInfo
	{
		TypeInfo type;
		NameInfo name;
		ArrayInfo arraySizes;

		Lumina::Token tokens() const
		{
			return (type.tokens() + name.tokens() + arraySizes.tokens());
		}
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

	struct SymbolBodyInfo
	{
		bool prototype = true;
		std::vector<std::shared_ptr<Instruction>> instructions;
	};

	struct NamespaceDesignation
	{
		std::vector<Lumina::Token> namespaceTokens;

		Lumina::Token tokens() const
		{
			return (Lumina::Token::merge(namespaceTokens, Lumina::Token::Type::Identifier));
		}

		std::string to_string() const
		{
			std::string result = "";

			for (const auto& token : namespaceTokens)
			{
				result += token.content;
			}

			return (result);
		}
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

			virtual Lumina::Token tokens() const = 0;
		};

		struct ArrayAccessor : public Accessor
		{
			std::shared_ptr<Expression> expression;

			ArrayAccessor() :
				Accessor(Type::Array)
			{

			}

			Lumina::Token tokens() const;
		};

		struct AttributeAccessor : public Accessor
		{
			NameInfo name;

			AttributeAccessor() :
				Accessor(Type::Attribute)
			{

			}
			
			Lumina::Token tokens() const
			{
				return (name.tokens());
			}
		};

		NamespaceDesignation nspace;
		NameInfo name;
		std::vector<std::shared_ptr<Accessor>> accessors;

		Lumina::Token tokens() const
		{
			Lumina::Token result = nspace.tokens() + name.tokens();

			for (const auto& accessor : accessors)
			{
				result += accessor->tokens();
			}

			return (result);
		}
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

			virtual Lumina::Token tokens() const = 0;
		};

		std::vector<std::shared_ptr<Element>> elements;

		Lumina::Token tokens() const
		{
			Lumina::Token result;

			result.type = Lumina::Token::Type::Expression;
			
			for (const auto& element : elements)
			{
				result += element->tokens();
			}

			return (result);
		}

		struct NumberElement : public Expression::Element
		{
			Lumina::Token value;

			NumberElement() :
				Expression::Element(Type::Number)
			{

			}

			Lumina::Token tokens() const
			{
				return (value);
			}
		};

		struct VariableElement : public Expression::Element
		{
			VariableDesignation value;

			VariableElement() :
				Expression::Element(Type::Variable)
			{

			}

			std::string variableName() const
			{
				std::string result;
				
				result += value.nspace.to_string();
				result += value.name.value.content;

				return (result);
			}

			Lumina::Token tokens() const
			{
				return (value.tokens());
			}
		};

		struct ExpressionElement : public Expression::Element
		{
			std::shared_ptr<Expression> value;

			ExpressionElement() :
				Expression::Element(Type::Expression)
			{

			}

			Lumina::Token tokens() const
			{
				return (value->tokens());
			}
		};

		struct BooleanElement : public Expression::Element
		{
			Lumina::Token value;

			BooleanElement() :
				Expression::Element(Type::Boolean)
			{

			}

			Lumina::Token tokens() const
			{
				return (value);
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

			Lumina::Token tokens() const
			{
				Lumina::Token result = nspace.tokens() + name.tokens();

				for (const auto& parameter : parameters)
				{
					result += parameter->tokens();
				}

				return (result);
			}
		};

		struct IncrementElement : public Expression::Element
		{
			Lumina::Token value;

			IncrementElement() :
				Expression::Element(Type::Increment)
			{

			}

			Lumina::Token tokens() const
			{
				return (value);
			}
		};

		struct OperatorElement : public Expression::Element
		{
			Lumina::Token value;

			OperatorElement() :
				Expression::Element(Type::Operator)
			{

			}

			Lumina::Token tokens() const
			{
				return (value);
			}
		};

		struct ArrayDereferencementElement : public Expression::Element
		{
			std::vector<Expression::Element*> values;

			ArrayDereferencementElement() :
				Expression::Element(Type::ArrayDereferencement)
			{

			}

			Lumina::Token tokens() const
			{
				Lumina::Token result;

				for (const auto& value : values)
				{
					result += value->tokens();
				}

				return (result);
			}
		};
	};

	Lumina::Token VariableDesignation::ArrayAccessor::tokens() const
	{
		return (expression->tokens());
	}

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
		SymbolBodyInfo body;
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
		SymbolBodyInfo body;

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
		SymbolBodyInfo body;
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
		Lumina::Token root;
		SymbolBodyInfo body;
	};

	struct ShaderInfo
	{
		Lumina::Token noToken;
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
		size_t _index = 0;
		Token _errorToken;
		std::vector<NamespaceInfo*> _currentNamespace;

		TypeInfo parseTypeInfo()
		{
			TypeInfo result;

			if (currentToken().type == Token::Type::NamespaceSeparator)
			{
				result.typeTokens.push_back(expect(Token::Type::NamespaceSeparator, "Expected a namespace separator." + DEBUG_INFORMATION));
			}

			while (tokenAtOffset(1).type == Token::Type::NamespaceSeparator)
			{
				result.typeTokens.push_back(expect(Token::Type::Identifier, "Expected a namespace name." + DEBUG_INFORMATION));
				result.typeTokens.push_back(expect(Token::Type::NamespaceSeparator, "Expected a namespace separator." + DEBUG_INFORMATION));
			}
			result.typeTokens.push_back(expect(Token::Type::Identifier, "Expected a type name." + DEBUG_INFORMATION));

			return (result);
		}

		ArrayInfo parseArrayInfo()
		{
			ArrayInfo result;

			while (currentToken().type == Token::Type::OpenBracket)
			{
				expect(Token::Type::OpenBracket, "Expected a '[' token." + DEBUG_INFORMATION);
				result.arrayTokens.push_back(expect(Token::Type::Number, "Expected a array size token." + DEBUG_INFORMATION));
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

		bool describeVariableDeclaration() const
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

		bool describeVariableAssignation() const
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
				result.namespaceTokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Expected a namespace separator." + DEBUG_INFORMATION));

			while (currentToken().type == Token::Type::Identifier && tokenAtOffset(1).type == Token::Type::NamespaceSeparator)
			{
				result.namespaceTokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected a namespace name." + DEBUG_INFORMATION));
				result.namespaceTokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Expected a namespace separator." + DEBUG_INFORMATION));
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

		bool describeSymbolCall() const
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
			ifBranch.body = parseSymbolBodyInfo();

			result->conditonnalBranchs.push_back(ifBranch);

			while (currentToken().type == Lumina::Token::Type::ElseStatement && nextToken().type == Lumina::Token::Type::IfStatement)
			{
				ConditionnalBranch newBranch;

				expect(Lumina::Token::Type::ElseStatement, "Expected a 'else' token." + DEBUG_INFORMATION);
				expect(Lumina::Token::Type::IfStatement, "Expected a 'if' token." + DEBUG_INFORMATION);
				expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
				newBranch.expression = parseExpression();
				expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);
				newBranch.body = parseSymbolBodyInfo();

				result->conditonnalBranchs.push_back(newBranch);
			}

			if (currentToken().type == Lumina::Token::Type::ElseStatement)
			{
				ConditionnalBranch newBranch;

				expect(Lumina::Token::Type::ElseStatement, "Expected a 'else' token." + DEBUG_INFORMATION);
				newBranch.body = parseSymbolBodyInfo();

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
			result->body = parseSymbolBodyInfo();

			return (result);
		}

		SymbolBodyInfo parseSymbolBodyInfo()
		{
			SymbolBodyInfo result;

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

			_result.value.noToken = Lumina::Token("", Lumina::Token::Type::Unknow, p_tokens[0].context);

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
			PipelineBodyInfo newPipelineBody;

			newPipelineBody.root = expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::OpenParenthesis, "Expected a '(' token." + DEBUG_INFORMATION);
			expect(Lumina::Token::Type::CloseParenthesis, "Expected a ')' token." + DEBUG_INFORMATION);

			newPipelineBody.body = parseSymbolBodyInfo();

			if (newPipelineBody.root == "VertexPass")
			{
				_result.value.vertexPipelineBody = newPipelineBody;
			}
			else
			{
				_result.value.fragmentPipelineBody = newPipelineBody;
			}
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
				newFunction.body = parseSymbolBodyInfo();
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

			_result.value.pipelineFlows.push_back(newPipelineFlow);
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
		const Type* type = nullptr;
		std::string name;
		std::vector<size_t> arraySizes;

		Variable() = default;
		Variable(const std::string& p_name) :
			type(nullptr),
			name(p_name),
			arraySizes({})
		{

		}
		Variable(const Type* p_type, const std::string& p_name, const std::vector<size_t>& p_arraySizes) :
			type(p_type),
			name(p_name),
			arraySizes(p_arraySizes)
		{

		}

		bool operator<(const Variable& p_other) const
		{
			return name < p_other.name;
		}

		bool isSame (const Variable& p_other) const
		{
			if (type != p_other.type)
				return (false);
			if (arraySizes.size() != p_other.arraySizes.size())
				return (false);
			for (size_t i = 0; i < arraySizes.size(); i++)
			{
				if (arraySizes[i] != p_other.arraySizes[i])
					return (false);
			}
			return (true);
		}
	};

	struct Type
	{
		std::string name;
		std::set<Variable> attributes;
		std::set<const Type*> compatibleTypes;

		bool operator<(const Type& p_other) const
		{
			return name < p_other.name;
		}

		bool isCompatible(const Type* p_other) const
		{
			if (p_other == this)
				return (true);

			return (compatibleTypes.contains(p_other));
		}
	};

	struct ExpressionType
	{
		const Type* type = nullptr;
		std::vector<size_t> arraySizes;

		std::string to_string() const
		{
			std::string result;

			result += (type == nullptr ? "Unknow" : type->name.substr(2));

			for (const auto& size : arraySizes)
			{
				result += "[" + std::to_string(size) + "]";
			}

			return (result);
		}

		static ExpressionType fromVariable(const Variable& p_variable)
		{
			ExpressionType result;

			result.type = p_variable.type;
			result.arraySizes = p_variable.arraySizes;

			return (result);
		}

		bool operator ==(const ExpressionType& p_other) const
		{
			if (type != p_other.type)
				return (false);
			if (arraySizes.size() != p_other.arraySizes.size())
				return (false);
			for (size_t i = 0; i < arraySizes.size(); i++)
			{
				if (arraySizes[i] != p_other.arraySizes[i])
					return (false);
			}
			return (true);
		}

		bool operator != (const ExpressionType& p_other) const
		{
			return (!(this->operator==(p_other)));
		}
	};

	struct Function
	{
		using ReturnType= ExpressionType;

		bool isPrototype = false;
		ReturnType returnType;
		std::string name;
		std::set<Variable> parameters;

		bool operator == (const Function& p_other) const
		{
			if (returnType != p_other.returnType)
				return (false);
			if (name != p_other.name)
				return (false);
			if (parameters.size() != p_other.parameters.size())
				return (false);

			auto it1 = parameters.begin();
			auto it2 = p_other.parameters.begin();

			while (it1 != parameters.end() && it2 != p_other.parameters.end())
			{
				if (!it1->isSame(*it2))
					return (false);
				++it1;
				++it2;
			}

			return (true);
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
		Lumina::Token _noToken;
		std::vector<std::string> _currentNamespaces = {""};

		std::string currentNamespace() const
		{
			std::string result = "";

			for (const auto& nspace : _currentNamespaces)
			{
				result += nspace;
				result += "::";
			}

			return (result);
		}

		std::set<Type> _standardTypes;
		std::set<Type> _types;

		bool _vertexPassParsed = false;
		bool _fragmentPassParsed = false;

		std::map<std::string, std::vector<Function>> _functions;

		std::set<std::string> _reservedNames;

		std::set<Variable> _globalVariables;
		std::set<Variable> _vertexVariables;
		std::set<Variable> _fragmentVariables;

		const Type* lookupTypeInNamespace(const std::string& p_typeName) const
		{
			auto it = _types.find(Type{ p_typeName });
			if (it != _types.end())
			{
				return &(*it);
			}
			return nullptr;
		}

		const Type* getType(const Lumina::Token& p_nameToken) const
		{
			const Type* type = lookupTypeInNamespace(p_nameToken.content);
			if (type != nullptr)
			{
				return type;
			}

			for (size_t i = 0; i < _currentNamespaces.size(); i++)
			{
				std::string qualifiedName = "";
				for (size_t j = 0; j <= i; ++j)
				{
					qualifiedName += "::";
					qualifiedName += _currentNamespaces[j];
				}
				qualifiedName += p_nameToken.content;

				type = lookupTypeInNamespace(qualifiedName);
				if (type != nullptr)
				{
					return type;
				}
			}

			throw TokenBasedError("Type '" + p_nameToken.content + "' not found in the current scope.", p_nameToken);
		}

		const Type* getType(const std::vector<Lumina::Token>& p_nameTokens) const
		{
			Lumina::Token mergedToken = Lumina::Token::merge(p_nameTokens, Lumina::Token::Type::Identifier);

			return getType(mergedToken);
		}


		Type* lookupTypeInNamespace(const std::string& p_typeName)
		{
			return const_cast<Type*>(static_cast<const SemanticChecker*>(this)->lookupTypeInNamespace(p_typeName));
		}

		Type* getType(const Lumina::Token& p_nameTokens)
		{
			return const_cast<Type*>(static_cast<const SemanticChecker*>(this)->getType(p_nameTokens));
		}

		Type* getType(const std::vector<Lumina::Token>& p_nameTokens)
		{
			return const_cast<Type*>(static_cast<const SemanticChecker*>(this)->getType(p_nameTokens));
		}

		std::vector<size_t> parseArrayInfo(const ArrayInfo& p_arraySizeInfo)
		{
			std::vector<size_t> result;

			for (const auto& token : p_arraySizeInfo.arrayTokens)
			{
				int arraySize = std::stoi(token.content);

				if (arraySize <= 0)
				{
					throw TokenBasedError("Array size must be greater than 0. Invalid size found.", token);
				}

				result.push_back(static_cast<size_t>(arraySize));
			}

			return (result);
		}

		Function::ReturnType parseReturnType(const FunctionInfo::ReturnType& p_returnTypeInfo)
		{
			Function::ReturnType result;

			result.type = getType(p_returnTypeInfo.type.typeTokens);
			result.arraySizes = parseArrayInfo(p_returnTypeInfo.arraySizes);

			return (result);
		}

		Variable parseVariable(const VariableInfo& p_variableInfo)
		{
			Variable result;

			result.name = p_variableInfo.name.value.content;
			result.type = getType(p_variableInfo.type.typeTokens);
			result.arraySizes = parseArrayInfo(p_variableInfo.arraySizes);

			if (_reservedNames.contains(result.name) == true)
			{
				throw TokenBasedError("Can't use identifier [" + result.name + "] : identifier already used.", p_variableInfo.name.value);
			}

			return (result);
		}

		Type parseBlockInfo(const BlockInfo& p_block)
		{
			Type result;

			std::string namespaceName = currentNamespace();

			result.name = namespaceName + p_block.name.value.content;

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
				if (namespaceName == "")
				{
					throw TokenBasedError("Type [" + p_block.name.value.content + "] already defined.", p_block.name.value);
				}
				else
				{
					throw TokenBasedError("Type [" + p_block.name.value.content + "] already defined in namespace [" + namespaceName.substr(2) + "].", p_block.name.value);
				}
			}

			if (_reservedNames.contains(result.name) == true)
			{
				throw TokenBasedError("Can't use identifier [" + p_block.name.value.content + "] : identifier already used in namespace [" + namespaceName.substr(2) + "].", p_block.name.value);
			}

			return (result);
		}

		Function parseFunctionInfo(const FunctionInfo& p_functionInfo)
		{
			Function result;

			std::string namespaceName = currentNamespace();

			result.isPrototype = p_functionInfo.body.prototype;
			result.returnType = parseReturnType(p_functionInfo.returnType);
			result.name = namespaceName + p_functionInfo.name.value.content;

			for (const auto& parameterInfo : p_functionInfo.parameters)
			{
				Variable newParameter = parseVariable(parameterInfo);

				if (result.parameters.contains(newParameter) == true)
				{
					throw TokenBasedError("Parameter variable [" + newParameter.name + "] already declared.", parameterInfo.name.value);
				}

				result.parameters.insert(newParameter);
			}

			return (result);
		}

		void parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow)
		{
			enum class PipelineFlowType
			{
				ToVertex,
				ToFragment,
				ToOutput
			};

			PipelineFlowType type;

			if (p_pipelineFlow.input == "Input" && p_pipelineFlow.output == "VertexPass")
			{
				type = PipelineFlowType::ToVertex;
			}
			else if (p_pipelineFlow.input == "VertexPass" && p_pipelineFlow.output == "FragmentPass")
			{
				type = PipelineFlowType::ToFragment;
			}
			else if (p_pipelineFlow.input == "FragmentPass" && p_pipelineFlow.output == "Output")
			{
				type = PipelineFlowType::ToOutput;
			}
			else
			{
				throw TokenBasedError("Invalid pipeline flow definition.", p_pipelineFlow.input + p_pipelineFlow.output);
			}

			if (p_pipelineFlow.variable.arraySizes.arrayTokens.size() != 0)
			{
				throw TokenBasedError("Pipeline flow variable cannot be array variable." + DEBUG_INFORMATION, p_pipelineFlow.variable.name.value);
			}

			Variable newPipelineVariable = parseVariable(p_pipelineFlow.variable);

			if (_standardTypes.contains(*(newPipelineVariable.type)) == false)
			{
				throw TokenBasedError("Pipeline flow variable can only be of a standard types." + DEBUG_INFORMATION, Lumina::Token::merge(p_pipelineFlow.variable.type.typeTokens, Lumina::Token::Type::Identifier));
			}

			_reservedNames.insert(newPipelineVariable.name);

			switch (type)
			{
			case PipelineFlowType::ToVertex:
			{
				_vertexVariables.insert(newPipelineVariable);
				break;
			}
			case PipelineFlowType::ToFragment:
			{
				_vertexVariables.insert(newPipelineVariable);
				_fragmentVariables.insert(newPipelineVariable);
				break;
			}
			case PipelineFlowType::ToOutput:
			{
				_fragmentVariables.insert(newPipelineVariable);
				break;
			}
			}
		}

		void addStandardType(const Type& p_type)
		{
			_standardTypes.insert(p_type);
			_types.insert(p_type);

			_reservedNames.insert(p_type.name);
		}

		void addStructure(const BlockInfo& p_blockInfo)
		{
			Type newStructure = parseBlockInfo(p_blockInfo);

			_types.insert(newStructure);
			_reservedNames.insert(newStructure.name);
		}

		void addAttribute(const BlockInfo& p_blockInfo)
		{
			std::string nspace = currentNamespace();
			Type newAttributeType = parseBlockInfo(p_blockInfo);

			_types.insert(newAttributeType);

			Variable newAttribute = Variable(
				lookupTypeInNamespace(newAttributeType.name),
				nspace + p_blockInfo.name.value.content,
				{}
			);

			_globalVariables.insert(newAttribute);
			_reservedNames.insert(newAttribute.name);
		}

		void addConstant(const BlockInfo& p_blockInfo)
		{
			std::string nspace = currentNamespace();
			Type newConstantType = parseBlockInfo(p_blockInfo);

			_types.insert(newConstantType);

			Variable newConstant = Variable(
				lookupTypeInNamespace(newConstantType.name),
				nspace + p_blockInfo.name.value.content,
				{}
			);

			_globalVariables.insert(newConstant);
			_reservedNames.insert(newConstant.name);
		}

		void parseTexture(const TextureInfo& p_texture)
		{
			std::string nspace = currentNamespace();

			Variable newTexture = Variable(
				lookupTypeInNamespace("::Texture"),
				nspace + p_texture.name.value.content,
				{}
			);

			if (_globalVariables.contains(newTexture) == true)
			{
				if (nspace == "")
				{
					throw TokenBasedError("Type [" + p_texture.name.value.content + "] already defined.", p_texture.name.value);
				}
				else
				{
					throw TokenBasedError("Type [" + p_texture.name.value.content + "] already defined in namespace [" + nspace.substr(2) + "].", p_texture.name.value);
				}
			}

			_globalVariables.insert(newTexture);
			_reservedNames.insert(newTexture.name);
		}

		void handleBoolExpressionElement(ExpressionType& currentExpressionType, std::shared_ptr<Expression::BooleanElement> p_element)
		{
			ExpressionType elementType = ExpressionType(lookupTypeInNamespace("::bool"));

			if (currentExpressionType.type == nullptr)
			{
				currentExpressionType = elementType;
			}
			else
			{
				if (elementType.type != currentExpressionType.type || currentExpressionType.arraySizes.size() != 0)
				{
					throw TokenBasedError("No convertion found between [" + elementType.to_string() + "] and [" + currentExpressionType.to_string() + "].", p_element->tokens());
				}
			}
		}

		void handleNumberExpressionElement(ExpressionType& currentExpressionType, std::shared_ptr<Expression::NumberElement> p_element)
		{
			const Type* numberType = lookupTypeInNamespace(p_element->value.content.find(',') != std::string::npos ? "::float" : "::int");
			ExpressionType elementType = { numberType, {} };

			if (currentExpressionType.type == nullptr)
			{
				currentExpressionType = elementType;
			}
			else
			{
				if (elementType.type->isCompatible(currentExpressionType.type) == false || currentExpressionType.arraySizes.size() != 0)
				{
					throw TokenBasedError("No conversion found between [" + elementType.to_string() + "] and expected [" + currentExpressionType.to_string() + "] type.", p_element->tokens());
				}
			}
		}

		const Variable& getVariable(const VariableDesignation& p_variableDesignation, const std::set<Variable>& p_availableVariables)
		{
			std::string variableName = p_variableDesignation.nspace.to_string() + p_variableDesignation.name.value.content;

			{
				auto it = p_availableVariables.find(Variable{nullptr, variableName, {} });
				if (it != p_availableVariables.end())
				{
					return (*it);
				}
			}

			for (size_t i = 0; i < _currentNamespaces.size(); i++)
			{
				std::string qualifiedName = "";
				for (size_t j = 0; j <= i; ++j)
				{
					qualifiedName += "::";
					qualifiedName += _currentNamespaces[j];
				}
				qualifiedName += variableName;

				{
					auto it = p_availableVariables.find(Variable{ nullptr, qualifiedName, {} });
					if (it != p_availableVariables.end())
					{
						return (*it);
					}
				}
			}

			throw TokenBasedError("No variable [" + variableName + "] declared in this scope.", p_variableDesignation.nspace.tokens() + p_variableDesignation.name.value);
		}

		void handleVariableExpressionElement(ExpressionType& currentExpressionType, std::shared_ptr<Expression::VariableElement> p_element, const std::set<Variable>& p_availableVariables)
		{
			ExpressionType elementType = ExpressionType::fromVariable(getVariable(p_element->value, p_availableVariables));

			if (currentExpressionType.type == nullptr)
			{
				currentExpressionType = elementType;
			}
			else
			{
				if (elementType.type == nullptr)
				{
					throw TokenBasedError("Unknow expression type.", p_element->tokens());
				}

				if (elementType.type->isCompatible(currentExpressionType.type) == false || elementType.arraySizes.size() != currentExpressionType.arraySizes.size())
				{
					throw TokenBasedError("No conversion found between [" + elementType.to_string() + "] and expected [" + currentExpressionType.to_string() + "] type.", p_element->tokens());
				}
			}
		}

		ExpressionType evaluateExpressionType(std::shared_ptr<Expression> p_expression, const std::set<Variable>& p_availableVariables)
		{
			ExpressionType result;

			for (const auto& element : p_expression->elements)
			{
				switch (element->type)
				{
				case Expression::Element::Type::Boolean:
				{
					handleBoolExpressionElement(result, static_pointer_cast<Expression::BooleanElement>(element));
					break;
				}
				case Expression::Element::Type::Number:
				{
					handleNumberExpressionElement(result, static_pointer_cast<Expression::NumberElement>(element));
					break;
				}
				case Expression::Element::Type::Variable:
				{
					handleVariableExpressionElement(result, static_pointer_cast<Expression::VariableElement>(element), p_availableVariables);
					break;
				}
				case Expression::Element::Type::Expression:
				{

					break;
				}
				case Expression::Element::Type::SymbolCall:
				{

					break;
				}
				case Expression::Element::Type::Operator:
				{

					break;
				}
				case Expression::Element::Type::Increment:
				{

					break;
				}
				case Expression::Element::Type::ArrayDereferencement:
				{

					break;
				}
				}				
			}

			return (result);
		}

		void parseVariableDeclaration(std::shared_ptr<VariableDeclaration> p_variableDeclaration, std::set<Variable>& p_availableVariables)
		{
			Variable newVariable = parseVariable(p_variableDeclaration->value);

			ExpressionType variableType = ExpressionType::fromVariable(newVariable);
			ExpressionType initializerType = variableType;
			if (p_variableDeclaration->initializer != nullptr)
			{
				initializerType = evaluateExpressionType(p_variableDeclaration->initializer, p_availableVariables);
			}

			if (p_availableVariables.contains(newVariable) == true)
			{
				throw TokenBasedError("Variable [" + newVariable.name + "] already defined in this scope.", p_variableDeclaration->value.name.value);
			}

			if (initializerType.type == nullptr)
			{
				throw TokenBasedError("Impossible to evaluated assignator type.", p_variableDeclaration->initializer->tokens());
			}

			if (initializerType.type->isCompatible(variableType.type) == false || initializerType.arraySizes.size() != variableType.arraySizes.size())
			{
				throw TokenBasedError("Convertion from [" + initializerType.to_string() + "] to [" + variableType.to_string() + "] is not defined", p_variableDeclaration->initializer->tokens());
			}
		}

		void parseVariableAssignation(std::shared_ptr<VariableAssignation> p_variableAssignation, const std::set<Variable>& p_availableVariables)
		{

		}

		void parseSymbolCall(std::shared_ptr<SymbolCall> p_symbolCall, const std::set<Variable>& p_availableVariables)
		{

		}

		void parseIfStatement(std::shared_ptr<IfStatement> p_ifStatement, const std::set<Variable>& p_availableVariables)
		{

		}

		void parseWhileStatement(std::shared_ptr<WhileStatement> p_whileStatement, const std::set<Variable>& p_availableVariables)
		{

		}

		void parseReturnStatement(std::shared_ptr<ReturnStatement> p_returnStatement, const std::set<Variable>& p_availableVariables)
		{

		}

		void parseDiscardStatement(std::shared_ptr<DiscardStatement> p_discardStatement)
		{

		}


		void parseSymbolBody(const SymbolBodyInfo& p_symbolBodyInfo, std::set<Variable> p_availableVariables)
		{
			for (const auto& instruction : p_symbolBodyInfo.instructions)
			{
				try
				{
					switch (instruction->type)
					{
					case Instruction::Type::VariableDeclaration:
					{
						parseVariableDeclaration(std::static_pointer_cast<VariableDeclaration>(instruction), p_availableVariables);
						break;
					}
					case Instruction::Type::VariableAssignation:
					{
						parseVariableAssignation(std::static_pointer_cast<VariableAssignation>(instruction), p_availableVariables);
						break;
					}
					case Instruction::Type::SymbolCall:
					{
						parseSymbolCall(std::static_pointer_cast<SymbolCall>(instruction), p_availableVariables);
						break;
					}
					case Instruction::Type::IfStatement:
					{
						parseIfStatement(std::static_pointer_cast<IfStatement>(instruction), p_availableVariables);
						break;
					}
					case Instruction::Type::WhileStatement:
					{
						parseWhileStatement(std::static_pointer_cast<WhileStatement>(instruction), p_availableVariables);
						break;
					}
					case Instruction::Type::ReturnStatement:
					{
						parseReturnStatement(std::static_pointer_cast<ReturnStatement>(instruction), p_availableVariables);
						break;
					}
					case Instruction::Type::DiscardStatement:
					{
						parseDiscardStatement(std::static_pointer_cast<DiscardStatement>(instruction));
						break;
					}
					default:
						throw TokenBasedError("Unknown instruction type.", _noToken);
					}
				}
				catch (TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}
		}


		void parseFunction(const FunctionInfo& p_functionInfo)
		{
			Function newFunction = parseFunctionInfo(p_functionInfo);

			if (_functions.contains(newFunction.name) == true)
			{
				std::vector<Function>& currentFunctions = _functions[newFunction.name];

				if (currentFunctions.front().returnType != newFunction.returnType)
				{
					throw TokenBasedError("Symbol already defined with a different return type.", p_functionInfo.name.value);
				}
				
				bool needInsertion = true;
				
				for (size_t i = 0; i < currentFunctions.size(); i++)
				{
					if (currentFunctions[i] == newFunction)
					{
						if (currentFunctions[i].isPrototype == true)
						{
							currentFunctions[i] = newFunction;
							needInsertion = false;
							break;
						}
						else if (newFunction.isPrototype == false)
						{
							throw TokenBasedError("Symbol already defined.", p_functionInfo.name.value);
						}
					}
				}

				if (needInsertion == true)
				{
					currentFunctions.push_back(newFunction);
				}
			}
			else
			{
				if (_reservedNames.contains(newFunction.name) == true)
				{
					throw TokenBasedError("Can't use identifier [" + newFunction.name + "] : identifier already used.", p_functionInfo.name.value);
				}

				_functions[newFunction.name].push_back(newFunction);
				_reservedNames.insert(newFunction.name);
			}

			std::set<Variable> allAvailableVariables = _globalVariables;
			allAvailableVariables.insert(newFunction.parameters.begin(), newFunction.parameters.end());

			parseSymbolBody(p_functionInfo.body, allAvailableVariables);
		}

		void parseNamespace(const NamespaceInfo& p_namespace)
		{
			if (p_namespace.name.value.content != "")
				_currentNamespaces.push_back(p_namespace.name.value.content);

			for (const auto& blockInfo : p_namespace.structureBlock)
			{

				try
				{
					addStructure(blockInfo);
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
					addAttribute(blockInfo);
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
					addConstant(blockInfo);
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

			if (p_namespace.name.value.content != "")
				_currentNamespaces.pop_back();
		}

		enum class PipelineStep
		{
			Vertex,
			Fragment
		};

		void parsePipelineBody(PipelineStep p_step, const PipelineBodyInfo& p_pipelineBody)
		{
			if (p_pipelineBody.body.prototype == true)
				return;

			std::set<Variable> allAvailableVariables = _globalVariables;

			switch (p_step)
			{
			case PipelineStep::Vertex:
			{
				if (_vertexPassParsed == true)
				{
					throw TokenBasedError("VertexPass already parsed.", p_pipelineBody.root);
				}
				_vertexPassParsed = true;
				allAvailableVariables.insert(_vertexVariables.begin(), _vertexVariables.end());
				break;
			}
			case PipelineStep::Fragment:
			{
				if (_fragmentPassParsed == true)
				{
					throw TokenBasedError("FragmentPass already parsed.", p_pipelineBody.root);
				}
				_fragmentPassParsed = true;
				allAvailableVariables.insert(_fragmentVariables.begin(), _fragmentVariables.end());
				break;
			}
			}

			parseSymbolBody(p_pipelineBody.body, allAvailableVariables);
		}

		Result _parse(const ShaderInfo& p_shaderInfo)
		{
			_result = Result();

			_noToken = p_shaderInfo.noToken;

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
				parsePipelineBody(PipelineStep::Vertex, p_shaderInfo.vertexPipelineBody);
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}

			try
			{
				parsePipelineBody(PipelineStep::Fragment, p_shaderInfo.fragmentPipelineBody);
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}

			if (_vertexPassParsed == false)
			{
				_result.errors.push_back(TokenBasedError("No vertex pass defined.", p_shaderInfo.noToken));
			}

			if (_fragmentPassParsed == false)
			{
				_result.errors.push_back(TokenBasedError("No fragment pass defined.", p_shaderInfo.noToken));
			}

			return (_result);
		}

		void addScalarTypes()
		{
			_types.insert(
				{
					.name = "::void",
					.attributes = {}
				});

			addStandardType(
				{
					.name = "::bool",
					.attributes = {}
				});

			addStandardType(
				{
					.name = "::int",
					.attributes = {}
				});

			addStandardType(
				{
					.name = "::float",
					.attributes = {}
				});

			addStandardType(
				{
					.name = "::uint",
					.attributes = {}
				});

			lookupTypeInNamespace("::bool")->compatibleTypes.insert(lookupTypeInNamespace("::int"));
			lookupTypeInNamespace("::bool")->compatibleTypes.insert(lookupTypeInNamespace("::uint"));
			lookupTypeInNamespace("::bool")->compatibleTypes.insert(lookupTypeInNamespace("::float"));


			lookupTypeInNamespace("::int")->compatibleTypes.insert(lookupTypeInNamespace("::bool"));
			lookupTypeInNamespace("::int")->compatibleTypes.insert(lookupTypeInNamespace("::uint"));
			lookupTypeInNamespace("::int")->compatibleTypes.insert(lookupTypeInNamespace("::float"));


			lookupTypeInNamespace("::uint")->compatibleTypes.insert(lookupTypeInNamespace("::bool"));
			lookupTypeInNamespace("::uint")->compatibleTypes.insert(lookupTypeInNamespace("::int"));
			lookupTypeInNamespace("::uint")->compatibleTypes.insert(lookupTypeInNamespace("::float"));


			lookupTypeInNamespace("::float")->compatibleTypes.insert(lookupTypeInNamespace("::bool"));
			lookupTypeInNamespace("::float")->compatibleTypes.insert(lookupTypeInNamespace("::int"));
			lookupTypeInNamespace("::float")->compatibleTypes.insert(lookupTypeInNamespace("::uint"));
		}

		void addVector2Types()
		{
			addStandardType(
				{
					.name = "::Vector2",
					.attributes = {
						Variable(
							lookupTypeInNamespace("float"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"y",
							{}
						)
					}
				});

			addStandardType(
				{
					.name = "::Vector2Int",
					.attributes = {
						Variable(
							lookupTypeInNamespace("int"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("int"),
							"y",
							{}
						)
					}
				});

			addStandardType(
				{
					.name = "::Vector2UInt",
					.attributes = {
						Variable(
							lookupTypeInNamespace("uint"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("uint"),
							"y",
							{}
						)
					}
				});
		}

		void addVector3Types()
		{
			addStandardType(
				{
					.name = "::Vector3",
					.attributes = {
						Variable(
							lookupTypeInNamespace("float"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"y",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"z",
							{}
						)
					}
				});

			addStandardType(
				{
					.name = "::Vector3Int",
					.attributes = {
						Variable(
							lookupTypeInNamespace("int"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("int"),
							"y",
							{}
						),
						Variable(
							lookupTypeInNamespace("int"),
							"z",
							{}
						)
					}
				});

			addStandardType(
				{
					.name = "::Vector3UInt",
					.attributes = {
						Variable(
							lookupTypeInNamespace("uint"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("uint"),
							"y",
							{}
						),
						Variable(
							lookupTypeInNamespace("uint"),
							"z",
							{}
						)
					}
				});
		}

		void addVector4Types()
		{
			addStandardType(
				{
					.name = "::Vector4",
					.attributes = {
						Variable(
							lookupTypeInNamespace("float"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"y",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"z",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"w",
							{}
						)
					}
				});

			addStandardType(
				{
					.name = "::Vector4Int",
					.attributes = {
						Variable(
							lookupTypeInNamespace("int"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("int"),
							"y",
							{}
						),
						Variable(
							lookupTypeInNamespace("int"),
							"z",
							{}
						),
						Variable(
							lookupTypeInNamespace("int"),
							"w",
							{}
						)
					}
				});

			addStandardType(
				{
					.name = "::Vector4UInt",
					.attributes = {
						Variable(
							lookupTypeInNamespace("uint"),
							"x",
							{}
						),
						Variable(
							lookupTypeInNamespace("uint"),
							"y",
							{}
						),
						Variable(
							lookupTypeInNamespace("uint"),
							"z",
							{}
						),
						Variable(
							lookupTypeInNamespace("uint"),
							"w",
							{}
						)
					}
				});
		}

		void addLuminaTypes()
		{
			addStandardType(
				{
					.name = "::Color",
					.attributes = {
						Variable(
							lookupTypeInNamespace("float"),
							"r",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"g",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"b",
							{}
						),
						Variable(
							lookupTypeInNamespace("float"),
							"a",
							{}
						)
					}
				});

			addStandardType(
				{
					.name = "::Texture",
					.attributes = {
					}
				});
		}

		void addMatrixTypes()
		{
			_types.insert(
				{
					.name = "::Matrix2x2",
					.attributes = {}  // No attributes
				});

			_types.insert(
				{
					.name = "::Matrix3x3",
					.attributes = {}  // No attributes
				});

			_types.insert(
				{
					.name = "::Matrix4x4",
					.attributes = {}  // No attributes
				});
		}

		void addPipelineFlowVariables()
		{
			Variable pixelPosition = Variable(lookupTypeInNamespace("::Vector4"), "pixelPosition", {});
			Variable pixelColor = Variable(lookupTypeInNamespace("::Color"), "pixelColor", {});

			_reservedNames.insert(pixelPosition.name);
			_vertexVariables.insert(pixelPosition);

			_reservedNames.insert(pixelColor.name);
			_fragmentVariables.insert(pixelColor);
		}

		SemanticChecker()
		{
			addScalarTypes();
			addVector2Types();
			addVector3Types();
			addVector4Types();
			addLuminaTypes();
			addMatrixTypes();
		
			addPipelineFlowVariables();
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

	if (tokens.size() == 0)
	{
		std::cout << "Empty source file [" << argv[1] << "]" << std::endl;
		return (-1);
	}

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
