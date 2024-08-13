#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<SymbolParameterInstruction> LexerChecker::parseSymbolParameterInstruction()
	{
		std::shared_ptr<SymbolParameterInstruction> result = std::make_shared<SymbolParameterInstruction>();

		result->type = parseTypeInstruction();
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an identifier token." + DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<SymbolNameInstruction> LexerChecker::parseSymbolCallNameInstruction()
	{
		std::shared_ptr<SymbolNameInstruction> result = std::make_shared<SymbolNameInstruction>();

		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
			result->tokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Expected an identifier token."+ DEBUG_INFORMATION));

		result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		while (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			result->tokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Expected a namespace separator token."+ DEBUG_INFORMATION));
			result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		}

		return result;
	}

	std::shared_ptr<SymbolCallInstruction> LexerChecker::parseSymbolCallInstruction()
	{
		std::shared_ptr<SymbolCallInstruction> result = std::make_shared<SymbolCallInstruction>();

		result->name = parseSymbolCallNameInstruction();

		expect(Lumina::Token::Type::OpenParenthesis, "Expected an open parenthesis."+ DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			if (result->arguments.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected a comma."+ DEBUG_INFORMATION);
			}
			result->arguments.push_back(parseExpression());
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected a close parenthesis."+ DEBUG_INFORMATION);

		if (currentToken().type == Token::Type::Accessor)
		{
			result->resultAccessor = std::make_shared<SymbolCallInstruction::ResultAccessor>();

			while (currentToken().type == Lumina::Token::Type::Accessor)
			{
				expect(Lumina::Token::Type::Accessor, "Expected an accessor token." + DEBUG_INFORMATION);
				result->resultAccessor->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token." + DEBUG_INFORMATION));
			}
		}

		return result;
	}
}