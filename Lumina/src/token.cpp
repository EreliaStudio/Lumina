#include "token.hpp"

namespace Lumina
{
	Token::Token(const std::string& p_content, Type p_type, int p_line, int p_column, const std::filesystem::path& p_originFile, const std::string& p_inputLine) :
		type(p_type),
		content(p_content),
		context{ p_line, p_column, p_originFile, p_inputLine }
	{

	}

	Token::Token(Type p_type) :
		type(p_type)
	{

	}

	Token::Token(const std::string& p_content, Type p_type, const Context& p_context) :
		type(p_type),
		content(p_content),
		context(p_context)
	{

	}

	bool Token::operator == (const std::string& p_string) const
	{
		return (content == p_string);
	}

	bool Token::operator != (const std::string& p_string) const
	{
		return (content != p_string);
	}

	std::ostream& operator << (std::ostream& p_os, const Token::Type& p_type)
	{
		p_os << Token::to_string(p_type);
		return p_os;
	}

	std::ostream& operator << (std::ostream& p_os, const Token& p_token)
	{
		p_os << "[" << std::setw(25) << p_token.type << "] | [" << std::setw(3) << p_token.context.line << "::" << std::left << std::setw(3) << p_token.context.column << std::right << "] | " << p_token.content;

		return (p_os);
	}

	std::string Token::to_string(Token::Type p_type)
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

	Lumina::Token Token::operator + (const Lumina::Token& p_toAdd) const
	{
		Lumina::Token result = *this;

		result.append(p_toAdd);

		return (result);
	}
	Lumina::Token Token::operator += (const Lumina::Token& p_toAdd)
	{
		append(p_toAdd);

		return (*this);
	}
	void Token::append(const Lumina::Token& p_toAdd)
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

	Token Token::merge(const std::vector<Token>& p_tokens, const Token::Type& p_type)
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

	TokenBasedError::TokenBasedError(const std::string& p_message, const Token& p_token)
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

	const char* TokenBasedError::what() const noexcept
	{
		return (_what.c_str());
	}
}