#pragma once

#include "parser.hpp"

namespace Lumina
{
	struct ShaderImpl
	{
		std::string value = "Default string";

		friend std::ostream& operator << (std::ostream& p_os, const ShaderImpl& p_shader)
		{
			p_os << p_shader.value;
			return (p_os);
		}
	};

	struct Compiler
	{
	private:
		ShaderImpl _result;

		Compiler() = default;

		ShaderImpl _compile(const Parser::Output& p_input)
		{
			return (_result);
		}

	public:
		static ShaderImpl compile(const Parser::Output& p_input)
		{
			return (Compiler()._compile(p_input));
		}
	};
}