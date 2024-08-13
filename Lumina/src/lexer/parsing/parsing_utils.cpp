#include "lumina_lexer.hpp"

namespace Lumina
{
	bool LexerChecker::hasTokenLeft() const
	{
		return (_index < _tokens->size());
	}

	void LexerChecker::backOff()
	{
		_index--;
	}

	void LexerChecker::advance()
	{
		_index++;
	}

	const Lumina::Token& LexerChecker::currentToken() const
	{
		return (_tokens->operator[](_index));
	}

	const Lumina::Token& LexerChecker::tokenAtIndex(size_t p_index) const
	{
		if (_index + p_index >= _tokens->size())
			return noToken;
		return (_tokens->operator[](_index + p_index));
	}

	const Lumina::Token& LexerChecker::nextToken() const
	{
		return (tokenAtIndex(1));
	}

	void LexerChecker::skipToken()
	{
		_index++;
	}

	void LexerChecker::skipLine()
	{
		int currentLine = currentToken().context.line;
		while (hasTokenLeft() == true &&
			currentLine == currentToken().context.line)
		{
			skipToken();
		}
	}

	const Lumina::Token& LexerChecker::expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage)
	{
		if (currentToken().type != p_expectedType)
		{
			throw Lumina::TokenBasedError(_file, p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return (result);
	}

	const Lumina::Token& LexerChecker::expect(std::vector<Lumina::Token::Type> p_expectedTypes, const std::string& p_errorMessage)
	{
		bool found = false;

		for (const auto& type : p_expectedTypes)
		{
			if (currentToken().type == type)
			{
				found = true;
			}
		}

		if (found == false)
		{
			throw Lumina::TokenBasedError(_file, p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return (result);
	}
}