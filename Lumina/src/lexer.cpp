#include "lexer.hpp"

namespace Lumina
{
	Lexer::Result Lexer::_lex(const std::vector<Lumina::Token>& p_tokens)
	{
		return (_result);
	}

	Lexer::Result Lexer::lex(const std::vector<Lumina::Token>& p_tokens)
	{
		return (Lexer()._lex(p_tokens));
	}
}