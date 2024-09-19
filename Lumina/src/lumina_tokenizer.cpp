#include "lumina_tokenizer.hpp"

#include <cctype>
#include <string>

#include "lumina_utils.hpp"

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
			while (index < code.size() && (isDigit(code[index]) || code[index] == '.'))
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

	std::vector<Token> Tokenizer::tokenize(const std::filesystem::path& p_path)
	{
		std::vector<Token> result;
		size_t index = 0;
		int currentLineNumber = 0;
		int lineNumber = 1;
		int columnNumber = 0;
		std::string currentLine;

		std::string rawInput = Lumina::readFileAsString(p_path);

		while (index < rawInput.size())
		{
			if (currentLineNumber != lineNumber)
			{
				currentLine = getLine(rawInput, index);
				currentLineNumber = lineNumber;
			}

			if (std::isspace(rawInput[index]))
			{
				if (rawInput[index] == '\n')
				{
					lineNumber++;
					columnNumber = 0;
				}
				else if (rawInput[index] == '\t')
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

			if (rawInput.substr(index, 8) == "#include")
			{
				tokenStr = parseSpecialToken(rawInput, index, "#include");
				tokenType = Token::Type::Include;
			}
			else if (rawInput[index] == '\"')
			{
				tokenStr = parseStringLiteral(rawInput, index);
				tokenType = Token::Type::StringLitteral;
			}
			else if (rawInput[index] == '/' && (rawInput[index + 1] == '/' || rawInput[index + 1] == '*'))
			{
				tokenStr = parseComment(rawInput, index);
				tokenType = Token::Type::Comment;
			}
			else if (rawInput[index] == '<')
			{
				size_t beginIndex = index;
				tokenStr = parseIncludeLiterals(rawInput, index);
				if (!tokenStr.empty())
				{
					tokenType = Token::Type::IncludeLitteral;
				}
				else
				{
					index = beginIndex;
					tokenStr = parseSpecialToken(rawInput, index, std::string(1, rawInput[index]));
					tokenType = Token::Type::ComparatorOperator;
				}
			}
			else if (isIdentifierStart(rawInput[index]))
			{
				tokenStr = parseIdentifier(rawInput, index);
				if (tokenStr == "Input" || tokenStr == "VertexPass" || tokenStr == "FragmentPass")
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
				else if (tokenStr == "for")
				{
					tokenType = Token::Type::ForStatement;
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
			else if (isDigit(rawInput[index]) || (rawInput[index] == '.' && isDigit(rawInput[index + 1])))
			{
				tokenStr = parseNumber(rawInput, index);
				tokenType = Token::Type::Number;
			}
			else if (rawInput.substr(index, 2) == "->")
			{
				tokenStr = parseSpecialToken(rawInput, index, "->");
				tokenType = Token::Type::PipelineFlowSeparator;
			}
			else if (rawInput.substr(index, 2) == "::")
			{
				tokenStr = parseSpecialToken(rawInput, index, "::");
				tokenType = Token::Type::NamespaceSeparator;
			}
			else
			{
				// Handle operators and other tokens
				std::string operators[] = { "==", "!=", "<=", ">=" };
				bool foundOperator = false;
				for (const std::string& op : operators)
				{
					if (rawInput.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(rawInput, index, op);
						tokenType = Token::Type::ComparatorOperator;
						foundOperator = true;
						break;
					}
				}

				std::string conditionOperators[] = { "||", "&&" };
				bool foundConditionOperator = false;
				for (const std::string& op : conditionOperators)
				{
					if (rawInput.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(rawInput, index, op);
						tokenType = Token::Type::ConditionOperator;
						foundConditionOperator = true;
						break;
					}
				}

				std::string assignators[] = { "=", "+=", "-=", "*=", "/=", "%=" };
				bool foundAssignator = false;
				for (const std::string& op : assignators)
				{
					if (rawInput.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(rawInput, index, op);
						tokenType = Token::Type::Assignator;
						foundOperator = true;
						break;
					}
				}

				std::string incrementor[] = { "++", "--" };
				bool foundIncrementor = false;
				for (const std::string& op : incrementor)
				{
					if (rawInput.substr(index, op.size()) == op)
					{
						tokenStr = parseSpecialToken(rawInput, index, op);
						tokenType = Token::Type::Incrementor;
						foundOperator = true;
						break;
					}
				}
				if (!foundOperator && !foundAssignator && !foundIncrementor && !foundConditionOperator)
				{
					switch (rawInput[index])
					{
					case '{':
						tokenStr = parseSpecialToken(rawInput, index, "{");
						tokenType = Token::Type::OpenCurlyBracket;
						break;
					case '}':
						tokenStr = parseSpecialToken(rawInput, index, "}");
						tokenType = Token::Type::CloseCurlyBracket;
						break;
					case '[':
						tokenStr = parseSpecialToken(rawInput, index, "[");
						tokenType = Token::Type::OpenBracket;
						break;
					case ']':
						tokenStr = parseSpecialToken(rawInput, index, "]");
						tokenType = Token::Type::CloseBracket;
						break;
					case '(':
						tokenStr = parseSpecialToken(rawInput, index, "(");
						tokenType = Token::Type::OpenParenthesis;
						break;
					case ')':
						tokenStr = parseSpecialToken(rawInput, index, ")");
						tokenType = Token::Type::CloseParenthesis;
						break;
					case '.':
						tokenStr = parseSpecialToken(rawInput, index, ".");
						tokenType = Token::Type::Accessor;
						break;
					case ';':
						tokenStr = parseSpecialToken(rawInput, index, ";");
						tokenType = Token::Type::EndOfSentence;
						break;
					case ':':
						tokenStr = parseSpecialToken(rawInput, index, ":");
						tokenType = Token::Type::Separator;
						break;
					case ',':
						tokenStr = parseSpecialToken(rawInput, index, ",");
						tokenType = Token::Type::Comma;
						break;
					case '<':
					case '>':
						tokenStr = parseSpecialToken(rawInput, index, std::string(1, rawInput[index]));
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
						tokenStr = parseSpecialToken(rawInput, index, std::string(1, rawInput[index]));
						tokenType = Token::Type::Operator;
						break;
					default:
						tokenStr = rawInput.substr(index, 1);
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
}
