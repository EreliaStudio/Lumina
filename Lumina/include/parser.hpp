#pragma once

#include "lexer.hpp"

namespace Lumina
{
	struct Parser
	{
	public:
		using Output = ShaderInfo;
		using Result = Lumina::Expected<Output>;

	private:
		Result _result;

		Parser() = default;

		Result _parse(const Lexer::Output& p_input)
		{
			return (_result);
		}

	public:
		static Result parse(const Lexer::Output& p_input)
		{
			return (Parser()._parse(p_input));
		}
	};
}