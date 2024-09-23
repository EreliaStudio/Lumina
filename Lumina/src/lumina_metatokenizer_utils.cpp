#include "lumina_metatokenizer.hpp"
#include "lumina_utils.hpp"
#include "lumina_tokenizer.hpp"

namespace Lumina
{
	MetaTokenizer::MetaTokenizer() {}

	Lumina::Token MetaTokenizer::composeToken(size_t p_startingIndex, size_t p_endIndex, Lumina::Token::Type p_type)
	{
		std::vector<Lumina::Token> tokenList;

		for (size_t i = p_startingIndex; i < p_endIndex; i++)
		{
			tokenList.push_back(_tokens[i]);
		}

		return (Lumina::Token::merge(tokenList, p_type));
	}

	bool MetaTokenizer::hasTokenLeft() const
	{
		return (_index < _tokens.size());
	}

	void MetaTokenizer::backOff()
	{
		_index--;
	}

	void MetaTokenizer::advance()
	{
		_index++;
	}

	const Lumina::Token& MetaTokenizer::currentToken() const
	{
		if (hasTokenLeft() == false)
			return (noToken);
		return (_tokens[_index]);
	}

	const Lumina::Token& MetaTokenizer::tokenAtIndex(size_t p_index) const
	{
		if (_index + p_index >= _tokens.size())
			return noToken;
		return (_tokens[_index + p_index]);
	}

	const Lumina::Token& MetaTokenizer::nextToken() const
	{
		return tokenAtIndex(1);
	}

	void MetaTokenizer::skipToken()
	{
		_index++;
	}

	void MetaTokenizer::skipLine()
	{
		int currentLine = currentToken().context.line;
		while (hasTokenLeft() && currentLine == currentToken().context.line)
		{
			skipToken();
		}
	}

	void MetaTokenizer::skipUntilReach(const TokenType& p_type)
	{
		while (hasTokenLeft() && currentToken().type != p_type)
		{
			skipToken();
		}
		if (hasTokenLeft())
			skipToken();
	}

	void MetaTokenizer::skipUntilReach(const std::vector<TokenType>& p_types)
	{
		while (hasTokenLeft())
		{
			for (const auto& type : p_types)
			{
				if (currentToken().type == type)
				{
					skipToken();
					return;
				}
			}
			skipToken();
		}
	}

	const Lumina::Token& MetaTokenizer::expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage)
	{
		if (currentToken().type != p_expectedType)
		{
			throw Lumina::TokenBasedError(p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return result;
	}

	const Lumina::Token& MetaTokenizer::expect(std::vector<Lumina::Token::Type> p_expectedTypes, const std::string& p_errorMessage)
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
			throw Lumina::TokenBasedError(p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return result;
	}

	void MetaTokenizer::expendInclude()
	{
		expect(TokenType::Include, "Expected a '#include' token.");
		Lumina::Token pathToken = expect({ TokenType::IncludeLitteral, TokenType::StringLitteral }, "Expected an include file path.");

		std::filesystem::path filePath = Lumina::composeFilePath(
			pathToken.content.substr(1, pathToken.content.size() - 2),
			{ pathToken.context.originFile.parent_path() }
		);

		std::vector<Lumina::Token> includeContent = Lumina::Tokenizer::tokenize(filePath);

		_tokens.insert(_tokens.begin() + _index, includeContent.begin(), includeContent.end());
	}
}