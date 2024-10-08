#include "compiler.hpp"

namespace Lumina
{
	std::ostream& operator << (std::ostream& p_os, const ShaderImpl& p_shader)
	{
		p_os << p_shader.value;
		return (p_os);
	}

	ShaderImpl Compiler::_compile(const Parser::Output& p_input)
	{
		return (_product);
	}

	ShaderImpl Compiler::compile(const Parser::Output& p_input)
	{
		return (Compiler()._compile(p_input));
	}
}