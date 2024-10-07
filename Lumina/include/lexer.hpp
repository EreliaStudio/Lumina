#pragma once

#include "expected.hpp"
#include "token.hpp"

#include "shader_info.hpp"

namespace Lumina
{
	struct Lexer
	{
	public:
		using Output = ShaderInfo;
		using Result = Lumina::Expected<Output>;

	private:
		Result _result;

		Lexer() = default;

		Result _lex(const std::vector<Lumina::Token>& p_tokens)
		{
			return (_result);
		}

	public:
		static Result lex(const std::vector<Lumina::Token>& p_tokens)
		{
			return (Lexer()._lex(p_tokens));
		}
	};
}