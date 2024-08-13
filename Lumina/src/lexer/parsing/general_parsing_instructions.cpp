#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<TypeInstruction> LexerChecker::parseTypeInstruction()
	{
		std::shared_ptr<TypeInstruction> result = std::make_shared<TypeInstruction>();

		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
			result->tokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Unexpected token found."+ DEBUG_INFORMATION));
		result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		while (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			result->tokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Unexpected token found."+ DEBUG_INFORMATION));
			result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		}

		return result;
	}
}