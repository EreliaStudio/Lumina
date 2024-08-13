#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<Lumina::ConditionElementInstruction> LexerChecker::parseConditionElementInstruction()
	{
		std::shared_ptr<ConditionElementInstruction> result = std::make_shared<ConditionElementInstruction>();

		result->lhs = parseExpression();
		if (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			result->comparatorToken = expect(Lumina::Token::Type::ConditionOperator, "Expected a valid comparator operator token." + DEBUG_INFORMATION);
			result->rhs = parseExpression();
		}

		return result;
	}

	std::shared_ptr<ConditionInstruction> LexerChecker::parseConditionInstruction()
	{
		std::shared_ptr<ConditionInstruction> result = std::make_shared<ConditionInstruction>();

		result->elements.push_back(parseConditionElementInstruction());

		while (currentToken().type == Token::Type::ComparatorOperator)
		{
			expect(Token::Type::ComparatorOperator, "Expected a valid comparator operator." + DEBUG_INFORMATION);
			result->elements.push_back(parseConditionElementInstruction());
		}

		return result;
	}

	std::shared_ptr<IfStatementInstruction> LexerChecker::parseIfStatementInstruction()
	{
		std::shared_ptr<IfStatementInstruction> result = std::make_shared<IfStatementInstruction>();

		expect(Lumina::Token::Type::IfStatement, "Expected an 'if' statement token."+ DEBUG_INFORMATION);

		expect(Lumina::Token::Type::OpenParenthesis, "Expected an open parenthesis."+ DEBUG_INFORMATION);
		result->condition = parseConditionInstruction();
		expect(Lumina::Token::Type::CloseParenthesis, "Expected a close parenthesis."+ DEBUG_INFORMATION);

		result->body = parseSymbolBodyInstruction();

		while (currentToken().type == Lumina::Token::Type::ElseStatement)
		{
			expect(Lumina::Token::Type::ElseStatement, "Expected an 'else' statement token."+ DEBUG_INFORMATION);

			std::shared_ptr<ElseInstruction> elseInstr = std::make_shared<ElseInstruction>();

			if (currentToken().type == Lumina::Token::Type::IfStatement)
			{
				expect(Lumina::Token::Type::IfStatement, "Expected an 'if' statement token."+ DEBUG_INFORMATION);
				expect(Lumina::Token::Type::OpenParenthesis, "Expected an open parenthesis."+ DEBUG_INFORMATION);
				elseInstr->condition = parseConditionInstruction();
				expect(Lumina::Token::Type::CloseParenthesis, "Expected a close parenthesis."+ DEBUG_INFORMATION);
			}

			elseInstr->body = parseSymbolBodyInstruction();
			result->elseBlocks.push_back(elseInstr);
		}

		return result;
	}

	std::shared_ptr<WhileLoopInstruction> LexerChecker::parseWhileLoopInstruction()
	{
		std::shared_ptr<WhileLoopInstruction> result = std::make_shared<WhileLoopInstruction>();

		expect(Lumina::Token::Type::WhileStatement, "Expected a 'while' statement token."+ DEBUG_INFORMATION);

		expect(Lumina::Token::Type::OpenParenthesis, "Expected an open parenthesis."+ DEBUG_INFORMATION);
		result->condition = parseConditionInstruction();
		expect(Lumina::Token::Type::CloseParenthesis, "Expected a close parenthesis."+ DEBUG_INFORMATION);

		result->body = parseSymbolBodyInstruction();

		return result;
	}

	std::shared_ptr<ForLoopInstruction> LexerChecker::parseForLoopInstruction()
	{
		std::shared_ptr<ForLoopInstruction> result = std::make_shared<ForLoopInstruction>();

		expect(Lumina::Token::Type::ForStatement, "Expected a 'for' statement token."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::OpenParenthesis, "Expected an open parenthesis."+ DEBUG_INFORMATION);

		if (currentToken().type != Lumina::Token::Type::EndOfSentence)
		{
			if (describeVariableDeclarationInstruction())
			{
				result->initializer = parseVariableDeclarationInstruction();
			}
			else
			{
				result->initializer = parseExpression();
			}
		}
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		if (currentToken().type != Lumina::Token::Type::EndOfSentence)
		{
			result->condition = parseConditionInstruction();
		}
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		if (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			result->increment = parseExpression();
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected a close parenthesis."+ DEBUG_INFORMATION);

		result->body = parseSymbolBodyInstruction();

		return result;
	}
}