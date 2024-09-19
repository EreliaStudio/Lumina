#pragma once

#include <exception>
#include <string>
#include <filesystem>

#include "lumina_token.hpp"

namespace Lumina
{
	class TokenBasedError : public std::exception
	{
	private:
		std::string _what;

	public:
		TokenBasedError(const std::string& p_message, const Token& p_token);

		virtual const char* what() const noexcept override;
	};

	template <typename TValueType>
	struct Expected
	{
		TValueType value;
		std::vector<TokenBasedError> errors;
	};
}