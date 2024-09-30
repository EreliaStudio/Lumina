#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

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
	struct TypeInfo
	{
		Lumina::Token value;
		std::vector<Lumina::Token> arraySizes;
	};

	struct VariableInfo
	{
		TypeInfo type;
		Lumina::Token name;
	};

	struct PipelineFlowInfo
	{
		Lumina::Token input;
		Lumina::Token output;

		VariableInfo variable;
	};

	struct BlockInfo
	{
		Lumina::Token name;
		std::vector<VariableInfo> elements;
	};

	struct Instruction
	{
		enum class Type
		{
			VariableDeclaration,
			VariableAssignation,
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
		std::vector<std::shared_ptr<Instruction>> instructions;
	};

	struct NamespaceDesignation
	{
		std::vector<Lumina::Token> tokens;
	};

	struct Accessor
	{
		std::vector<Lumina::Token> tokens;
	};

	struct VariableDesignation
	{
		NamespaceDesignation nspace;
		Lumina::Token name;
		Accessor accessor;
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
				Increment
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
			std::vector<Expression::Element*> value;

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
			Lumina::Token value;

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
	};

	struct VariableDeclaration : public Instruction
	{
		VariableInfo value;
		std::optional<Expression> initializer;
	};

	struct VariableAssignation : public Instruction
	{
		VariableDesignation variable;
		Expression value;
	};

	struct ConditionnalBranch : public Instruction
	{
		Expression expression;
		SymbolBody body;
	};

	struct IfStatement : public Instruction
	{
		std::vector<ConditionnalBranch> conditonnalBranch;
	};

	struct WhileStatement : public Instruction
	{
		Expression expression;
		SymbolBody body;
	};

	struct ReturnStatement : public Instruction
	{
		Expression value;
	};

	struct DiscardStatement : public Instruction
	{

	};

	struct FunctionInfo
	{
		TypeInfo returnType;
		Lumina::Token name;
		std::vector<VariableInfo> parameters;
		SymbolBody body;
	};

	struct NamespaceInfo
	{
		Lumina::Token name;

		std::vector<BlockInfo> structureBlock;
		std::vector<BlockInfo> attributeBlock;
		std::vector<BlockInfo> constantBlock;

		std::vector<FunctionInfo> functions;

		std::vector<NamespaceInfo> innerNamespaces;
	};

	struct PipelineBody
	{
		bool exist = false;
		SymbolBody body;
	};

	struct ShaderInfo
	{
		std::vector<PipelineFlowInfo> pipelineFlows;

		NamespaceInfo anonymNamespace;

		PipelineBody vertexPipelineBody;
		PipelineBody fragmentPipelineBody;
	};

	struct Parser
	{
		using Result = Lumina::Expected<ShaderInfo>;

		Result _result;
		std::vector<Token> _tokens;
		size_t _index;
		Token _errorToken;
		std::vector<NamespaceInfo*> _currentNamespace;

		VariableInfo parseVariableInfo()
		{
			VariableInfo result;

			if (currentToken().type == Token::Type::NamespaceSeparator)
			{
				result.type.value += currentToken();
			}

			while (tokenAtOffset(1).type == Token::Type::NamespaceSeparator)
			{
				result.type.value += expect(Token::Type::Identifier, "Expected a namespace name.");
				result.type.value += expect(Token::Type::NamespaceSeparator, "Expected a namespace separator.");
			}
			result.type.value += expect(Token::Type::Identifier, "Expected a type name.");

			result.name = expect(Token::Type::Identifier, "Expected a variable name.");
			
			while (currentToken().type == Token::Type::OpenBracket)
			{
				expect(Token::Type::OpenBracket, "Expected a '[' token.");
				result.type.arraySizes.push_back(expect(Token::Type::Number, "Expected a array size token."));
				expect(Token::Type::CloseBracket, "Expected a ']' token.");
			}

			return (result);
		}

		BlockInfo parseBlockInfo()
		{
			BlockInfo result;

			advance();
			result.name = expect(Lumina::Token::Type::Identifier, "Expected a block name.");
			expect(Lumina::Token::Type::OpenCurlyBracket, "Expected a '{' token.");
			while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
			{
				result.elements.push_back(parseVariableInfo());
				expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token.");
			}
			expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a '}' token.");
			expect(Lumina::Token::Type::EndOfSentence, "Expected a ';' token.");

			return (result);
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

		Result _parse(const std::vector<Token>& p_tokens)
		{
			_result = Result();
			_tokens = p_tokens;
			_index = 0;
			_result.value.anonymNamespace.name = Lumina::Token("", Lumina::Token::Type::Identifier, 0, 0, _tokens[0].context.originFile, _tokens[0].context.inputLine);
			_currentNamespace.push_back(&(_result.value.anonymNamespace));

			while (hasTokenLeft() == true)
			{
				try
				{
					switch (currentToken().type)
					{
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

					case Token::Type::Namespace:
					{
						parseNamespace();
						break;
					}
					default:
						throw TokenBasedError("Unexpected token type [" + Lumina::Token::to_string(currentToken().type) + "]", currentToken());
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

		void parseNamespace()
		{
			advance();

			NamespaceInfo newNamespace;

			newNamespace.name = expect(Lumina::Token::Type::Identifier, "Expected a namespace name.");

			_result.value.anonymNamespace.innerNamespaces.push_back(newNamespace);

			_currentNamespace.push_back(&(_result.value.anonymNamespace.innerNamespaces.back()));
		
			while (hasTokenLeft() == true)
			{
				try
				{
					switch (currentToken().type)
					{
					case Token::Type::Comment:
					{
						skipToken();
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
					case Token::Type::Namespace:
					{
						parseNamespace();
						break;
					}
					default:
						throw TokenBasedError("Unexpected token type [" + Lumina::Token::to_string(currentToken().type) + "] inside namespace [" + newNamespace.name.content + "]", currentToken());
					}
				}
				catch (const TokenBasedError& e)
				{
					_result.errors.push_back(e);
					skipLine();
				}
			}

			_currentNamespace.pop_back();
		}

		void parseInclude()
		{
			const Token& includeToken = currentToken();
			advance();

			const Token& pathToken = expect(
				{ Token::Type::IncludeLitteral, Token::Type::StringLitteral },
				"Expected a file path after #include");

			if (pathToken.content.length() < 2)
			{
				throw TokenBasedError("Invalid include path.", pathToken);
			}
			std::string rawPath = pathToken.content.substr(1, pathToken.content.size() - 2);

			std::filesystem::path includePath = composeFilePath(rawPath, { includeToken.context.originFile.parent_path() });

			if (!std::filesystem::exists(includePath))
			{
				throw TokenBasedError("Included file [" + includePath.string() + "] not found.", pathToken);
			}

			std::vector<Token> includedTokens = Tokenizer::tokenize(includePath);

			Parser includedParser;
			Result includedResult = includedParser._parse(includedTokens);

			_result.errors.insert(_result.errors.end(), includedResult.errors.begin(), includedResult.errors.end());

			if ((includedResult.value.vertexPipelineBody.exist == true) ||
				(includedResult.value.fragmentPipelineBody.exist == true))
			{
				throw TokenBasedError("Included file [" + includePath.string() + "] can't contain pipeline body.", pathToken);
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

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1]);

	Lumina::Parser::Result instructions = Lumina::Parser::parse(tokens);

	if (instructions.errors.size() != 0)
	{
		for (const auto& error : instructions.errors)
		{
			std::cout << error.what() << std::endl;
		}

		return (-1);
	}

	return (0);
}
