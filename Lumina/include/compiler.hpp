#pragma once

#include "parser.hpp"

namespace Lumina
{
	struct ShaderImpl
	{
		std::string value = "Default string";

		friend std::ostream& operator << (std::ostream& p_os, const ShaderImpl& p_shader);
	};

	struct Compiler
	{
	private:
		ShaderImpl _product;

		Compiler() = default;

		ShaderImpl _compile(const Parser::Output& p_input);

	public:
		static ShaderImpl compile(const Parser::Output& p_input);
	};
}