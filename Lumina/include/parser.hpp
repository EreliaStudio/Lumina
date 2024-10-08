#pragma once

#include "lexer.hpp"

namespace Lumina
{
	struct Parser
	{
	public:
		using Output = ShaderInfo;
		using Product = Lumina::Expected<Output>;

	private:
		Product _product;

		Parser() = default;

		Product _parse(const Lexer::Output& p_input);

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}