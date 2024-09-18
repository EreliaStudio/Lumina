#include "lumina_lexer.hpp"

namespace Lumina
{

	std::shared_ptr<ComparatorOperatorExpression> LexerChecker::parseComparatorOperatorExpression()
	{
		std::shared_ptr<ComparatorOperatorExpression> result = std::make_shared<ComparatorOperatorExpression>();

		result->token = expect(Lumina::Token::Type::ComparatorOperator, "Expected a comparator operator token." + DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<OperatorExpression> LexerChecker::parseOperatorExpression()
	{
		std::shared_ptr<OperatorExpression> result = std::make_shared<OperatorExpression>();

		result->token = expect(Lumina::Token::Type::Operator, "Expected an operator token." + DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<Lumina::BoolExpressionValueInstruction> LexerChecker::parseBoolExpressionValueInstruction()
	{
		std::shared_ptr<BoolExpressionValueInstruction> result = std::make_shared<BoolExpressionValueInstruction>();

		result->token = expect(Lumina::Token::Type::BoolStatement, "Expected a bool statement token [true] or [false]." + DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<NumberExpressionValueInstruction> LexerChecker::parseNumberExpressionValueInstruction()
	{
		std::shared_ptr<NumberExpressionValueInstruction> result = std::make_shared<NumberExpressionValueInstruction>();

		result->token = expect(Lumina::Token::Type::Number, "Expected a number token."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<StringLiteralsExpressionValueInstruction> LexerChecker::parseStringLiteralsExpressionValueInstruction()
	{
		std::shared_ptr<StringLiteralsExpressionValueInstruction> result = std::make_shared<StringLiteralsExpressionValueInstruction>();

		result->token = expect(Lumina::Token::Type::StringLitteral, "Expected a string literal token."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<VariableExpressionValueInstruction> LexerChecker::parseVariableExpressionValueInstruction()
	{
		std::shared_ptr<VariableExpressionValueInstruction> result = std::make_shared<VariableExpressionValueInstruction>();

		result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		while (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			result->tokens.push_back(expect(Lumina::Token::Type::NamespaceSeparator, "Expected a namespace separator token."+ DEBUG_INFORMATION));
			result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		}
		while (currentToken().type == Lumina::Token::Type::Accessor)
		{
			expect(Lumina::Token::Type::Accessor, "Expected an accessor token."+ DEBUG_INFORMATION);
			result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		}
		if (currentToken().type == Lumina::Token::Type::OpenBracket)
		{
			expect(Lumina::Token::Type::OpenBracket, "Expected an opening bracket token." + DEBUG_INFORMATION);
			result->arrayAccessorExpression = parseExpression();
			expect(Lumina::Token::Type::CloseBracket, "Expected a closing bracket token." + DEBUG_INFORMATION);
		}

		return result;
	}

	std::shared_ptr<Expression> LexerChecker::parseExpression()
	{
		std::shared_ptr<Expression> result = std::make_shared<Expression>();

		bool isParsing = true;
		while (isParsing == true)
		{
			switch (currentToken().type)
			{
			case Lumina::Token::Type::Operator:
			{
				result->elements.push_back(parseOperatorExpression());
				break;
			}
			case Lumina::Token::Type::ComparatorOperator:
			{
				result->elements.push_back(parseComparatorOperatorExpression());
				break;
			}
			case Lumina::Token::Type::BoolStatement:
			{
				result->elements.push_back(parseBoolExpressionValueInstruction());
				break;
			}
			case Lumina::Token::Type::Number:
			{
				result->elements.push_back(parseNumberExpressionValueInstruction());
				break;
			}
			case Lumina::Token::Type::StringLitteral:
			{
				result->elements.push_back(parseStringLiteralsExpressionValueInstruction());
				break;
			}
			case Lumina::Token::Type::Identifier:
			case Lumina::Token::Type::NamespaceSeparator:
			{
				if (describeSymbolCallName() == true)
				{
					result->elements.push_back(parseSymbolCallInstruction());
				}
				else
				{
					result->elements.push_back(parseVariableExpressionValueInstruction());
				}
				break;
			}
			default:
			{
				isParsing = false;
				break;
			}
			}
		}

		return result;
	}

}