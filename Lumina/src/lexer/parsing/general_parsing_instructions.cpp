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
	
	bool ArrayDefinition::isOnlyNumber() const
	{
		for (const auto& element : expression->elements)
		{
			switch (element->type)
			{
			case Instruction::Type::NumberExpressionValue:
			case Instruction::Type::OperatorExpression:
			{
				break;
			}
			default:
				return (false);
			}
		}
		return (true);
	}

	std::shared_ptr<ArrayDefinition> LexerChecker::parseArrayDefinition()
	{
		std::shared_ptr<ArrayDefinition> result = std::make_shared<ArrayDefinition>();

		expect(Lumina::Token::Type::OpenBracket, "Unexpected token found." + DEBUG_INFORMATION);
		
		result->expression = parseExpression();

		expect(Lumina::Token::Type::CloseBracket, "Unexpected token found." + DEBUG_INFORMATION);

		return (result);
	}

	Lumina::Token ArrayExpressionValueInstruction::mergedToken() const
	{
		std::vector<Token> expressionTokens;

		for (const auto& expression : elements)
		{
			expressionTokens.push_back(expression->mergedToken());
		}

		return (Lumina::Token::merge(expressionTokens, Lumina::Token::Type::Identifier));
	}

	std::shared_ptr<ArrayExpressionValueInstruction> LexerChecker::parseArrayExpressionValueInstruction()
	{
		std::shared_ptr<ArrayExpressionValueInstruction> result = std::make_shared<ArrayExpressionValueInstruction>();

		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' token.");

		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			if (result->elements.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected a ',' token.");
			}
			result->elements.push_back(parseExpression());
		}

		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' token.");

		return (result);
	}
}