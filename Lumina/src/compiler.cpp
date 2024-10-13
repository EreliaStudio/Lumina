#include "compiler.hpp"

namespace Lumina
{
	
	Compiler::Product Compiler::_compile(const Parser::Output& p_input)
	{
		_product = Product();

		return (_product);
	}

	Compiler::Product Compiler::compile(const Parser::Output& p_input)
	{
		return (Compiler()._compile(p_input));
	}
}