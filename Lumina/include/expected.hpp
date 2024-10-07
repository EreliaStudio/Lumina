#pragma once

#include <vector>
#include "token.hpp"

namespace Lumina
{
	template <typename TValueType>
	struct Expected
	{
		TValueType value;
		std::vector<TokenBasedError> errors;
	};

	template <>
	struct Expected<void>
	{
		std::vector<TokenBasedError> errors;
	};
}