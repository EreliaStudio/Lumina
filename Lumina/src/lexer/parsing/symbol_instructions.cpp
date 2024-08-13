#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<VariableDeclarationInstruction> LexerChecker::parseVariableDeclarationInstruction()
	{
		std::shared_ptr<VariableDeclarationInstruction> result = std::make_shared<VariableDeclarationInstruction>();

		result->type = parseTypeInstruction();
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an identifier name");

		if (currentToken().type != Lumina::Token::Type::EndOfSentence)
		{
			expect(Lumina::Token::Type::Assignator, "Expected an assignator token."+ DEBUG_INFORMATION);
			result->initializer = parseExpression();
			if (result->initializer->elements.size() == 0)
			{
				throw TokenBasedError(_file, "Expected an assignation value." + DEBUG_INFORMATION, currentToken());
			}
		}
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<ReturnInstruction> LexerChecker::parseReturnInstruction()
	{
		std::shared_ptr<ReturnInstruction> result = std::make_shared<ReturnInstruction>();

		expect(Lumina::Token::Type::Return, "Expected a return token."+ DEBUG_INFORMATION);
		result->argument = parseExpression();
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<DiscardInstruction> LexerChecker::parseDiscardInstruction()
	{
		std::shared_ptr<DiscardInstruction> result = std::make_shared<DiscardInstruction>();

		expect(Lumina::Token::Type::Discard, "Expected a discard token."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<VariableDesignationInstruction> LexerChecker::parseVariableDesignationInstruction()
	{
		std::shared_ptr<VariableDesignationInstruction> result = std::make_shared<VariableDesignationInstruction>();

		result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		while (currentToken().type == Lumina::Token::Type::Accessor)
		{
			expect(Lumina::Token::Type::Accessor, "Expected an accessor token."+ DEBUG_INFORMATION);
			result->tokens.push_back(expect(Lumina::Token::Type::Identifier, "Expected an identifier token."+ DEBUG_INFORMATION));
		}

		return result;
	}

	std::shared_ptr<VariableAssignationInstruction> LexerChecker::parseVariableAssignationInstruction()
	{
		std::shared_ptr<VariableAssignationInstruction> result = std::make_shared<VariableAssignationInstruction>();

		result->name = parseVariableDesignationInstruction();
		expect(Lumina::Token::Type::Assignator, "Expected an assignator token."+ DEBUG_INFORMATION);
		result->initializer = parseExpression();
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<SymbolBodyInstruction> LexerChecker::parseSymbolBodyInstruction()
	{
		std::shared_ptr<SymbolBodyInstruction> result = std::make_shared<SymbolBodyInstruction>();

		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected an open curly bracket."+ DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				switch (currentToken().type)
				{
				case Lumina::Token::Type::Comment:
					skipToken();
					break;
				case Lumina::Token::Type::Identifier:
				case Lumina::Token::Type::NamespaceSeparator:
					if (describeVariableDeclarationInstruction() == true)
					{
						result->elements.push_back(parseVariableDeclarationInstruction());
					}
					else if (describeSymbolCallInstruction() == true)
					{
						result->elements.push_back(parseSymbolCallInstruction());
						expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);
					}
					else if (describeVariableAssignationInstruction() == true)
					{
						result->elements.push_back(parseVariableAssignationInstruction());
					}
					else
					{
						throw Lumina::TokenBasedError(_file, "Unexpected token type: " + to_string(currentToken().type)+ DEBUG_INFORMATION, currentToken());
					}
					break;
				case Lumina::Token::Type::Return:
					result->elements.push_back(parseReturnInstruction());
					break;
				case Lumina::Token::Type::Discard:
					result->elements.push_back(parseDiscardInstruction());
					break;
				case Lumina::Token::Type::IfStatement:
					result->elements.push_back(parseIfStatementInstruction());
					break;
				case Lumina::Token::Type::WhileStatement:
					result->elements.push_back(parseWhileLoopInstruction());
					break;
				case Lumina::Token::Type::ForStatement:
					result->elements.push_back(parseForLoopInstruction());
					break;
				default:
					throw Lumina::TokenBasedError(_file, "Unexpected token type: " + to_string(currentToken().type)+ DEBUG_INFORMATION, currentToken());
				}
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);
				skipLine();
			}
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a close curly bracket."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<SymbolInstruction> LexerChecker::parseSymbolInstruction()
	{
		std::shared_ptr<SymbolInstruction> result = std::make_shared<SymbolInstruction>();

		result->returnType = parseTypeInstruction();
		result->name = expect(Lumina::Token::Type::Identifier, "Expect an identifier." + DEBUG_INFORMATION);
		expect(Lumina::Token::Type::OpenParenthesis, "Expected an open parenthesis."+ DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			result->parameters.push_back(parseSymbolParameterInstruction());
			if (currentToken().type != Lumina::Token::Type::CloseParenthesis)
				expect(Lumina::Token::Type::Comma, "Expected a comma."+ DEBUG_INFORMATION);
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected a close parenthesis."+ DEBUG_INFORMATION);
		result->body = parseSymbolBodyInstruction();

		return result;
	}
}