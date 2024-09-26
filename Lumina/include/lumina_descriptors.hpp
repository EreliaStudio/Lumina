#pragma once

#include "lumina_token.hpp"

namespace Lumina
{
	struct TypeDescriptor
	{
		Lumina::Token value;

		void append(const Lumina::Token& p_newToken);
	};

	struct VariableDescriptor
	{
		TypeDescriptor type;
		Lumina::Token name;
		std::vector<size_t> arraySizes;
	};

	struct ReturnTypeDescriptor
	{
		TypeDescriptor type;
		std::vector<size_t> arraySizes;
	};
}