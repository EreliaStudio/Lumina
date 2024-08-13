#include "lumina_exception.hpp"

namespace Lumina
{
	TokenBasedError::TokenBasedError(const std::filesystem::path& p_file, const std::string& p_message, const Token& p_token)
	{
		size_t tokenSize = std::min(p_token.content.size(), p_token.context.inputLine.size());
		std::stringstream ss;

		ss << "In file [" << p_file.string() << "] : " << std::endl;

		// Check for invalid token (type == Unknow)
		if (p_token.type == Token::Type::Unknow)
		{
			ss << "    " << p_message << std::endl;
		}
		else
		{
			ss << "    Error on line[" << std::setw(5) << (p_token.context.line) << "] : " << p_message << std::endl;
			if (p_token.context.line == -1 || tokenSize == 0)
			{
				// Do nothing, no additional error information
			}
			else if (tokenSize == 1)
			{
				ss << std::string(14, ' ') << p_token.context.inputLine << std::endl;
				if (p_token.context.column != -1)
					ss << std::string(14, ' ') << std::string(p_token.context.column, ' ') << '|';
			}
			else
			{
				ss << std::string(14, ' ') << p_token.context.inputLine << std::endl;
				if (p_token.context.column != -1)
				{
					if (tokenSize == p_token.content.size())
						ss << std::string(14, ' ') << std::string(p_token.context.column, ' ') << '|' << std::string(tokenSize - 2, '-') << '|';
					else
						ss << std::string(14, ' ') << std::string(p_token.context.column, ' ') << '|' << std::string(tokenSize - 1, '-');
				}
			}
		}

		_what = ss.str();
	}


	const char* TokenBasedError::what() const noexcept
	{
		return (_what.c_str());
	}
}