#include "parser.hpp"

namespace Lumina
{
	Parser::Result Parser::_parse(const Lexer::Output& p_input)
	{
		return (_result);
	}

	Parser::Result Parser::parse(const Lexer::Output& p_input)
	{
		return (Parser()._parse(p_input));
	}
}