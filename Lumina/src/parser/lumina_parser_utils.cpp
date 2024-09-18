#include "lumina_parser.hpp"

namespace Lumina
{
	Parser::Parser() :
		_tokens(nullptr),
		_index(0)
	{
	
	}

	bool Parser::hasTokenLeft() const
	{
		return (_index < _tokens->size());
	}

	void Parser::backOff()
	{
		if (_index > 0)
		{
			_index--;
		}
	}

	void Parser::advance()
	{
		if (hasTokenLeft())
		{
			_index++;
		}
	}

	const Token& Parser::currentToken() const
	{
		if (!hasTokenLeft())
		{
			Token unknowToken;
			unknowToken.context.file = (*_tokens)[0].context.file;
			throw TokenBasedError("Unexpected end of input", unknowToken);
		}
		return (_tokens->operator[](_index));
	}

	const Token& Parser::tokenAtIndex(size_t p_index) const
	{
		if (_index + p_index >= _tokens->size())
		{
			throw TokenBasedError("Index out of bounds", Token());
		}
		return (_tokens->operator[](_index + p_index));
	}

	const Token& Parser::nextToken() const
	{
		return tokenAtIndex(1);
	}

	void Parser::skipToken()
	{
		advance();
	}

	void Parser::skipLine()
	{
		if (hasTokenLeft() == false)
			return;

		int currentLine = currentToken().context.line;
		while (hasTokenLeft() && currentToken().context.line == currentLine)
		{
			skipToken();
		}
	}

	const Token& Parser::expect(Token::Type p_expectedType, const std::string& p_errorMessage)
	{
		if (currentToken().type != p_expectedType)
		{
			throw TokenBasedError(p_errorMessage, currentToken());
		}
		const Token& result = currentToken();
		advance();
		return result;
	}

	const Token& Parser::expect(const std::vector<Token::Type>& p_expectedTypes, const std::string& p_errorMessage)
	{
		bool found = false;

		for (const auto& type : p_expectedTypes)
		{
			if (currentToken().type == type)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			throw TokenBasedError(p_errorMessage, currentToken());
		}
		const Token& result = currentToken();
		advance();
		return result;
	}
}
