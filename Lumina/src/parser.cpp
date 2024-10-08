#include "parser.hpp"

namespace Lumina
{
	Parser::Product Parser::_parse(const Lexer::Output& p_input)
	{
		return (_product);
	}

	Parser::Product Parser::parse(const Lexer::Output& p_input)
	{
		return (Parser()._parse(p_input));
	}
}