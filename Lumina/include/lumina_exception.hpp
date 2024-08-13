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
		TokenBasedError(const std::filesystem::path& p_file, const std::string& p_message, const Token& p_token);

		virtual const char* what() const noexcept override;
	};
}