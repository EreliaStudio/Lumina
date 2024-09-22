#include "lumina_metatokenizer.hpp"
#include "lumina_instruction.hpp"

namespace Lumina
{
	bool MetaTokenizer::isDeclaration() const
	{
		size_t currentIndex = 0;

		if (tokenAtIndex(currentIndex).type == TokenType::NamespaceSeparator)
			currentIndex++;

		while (tokenAtIndex(currentIndex).type == TokenType::Identifier &&
			tokenAtIndex(currentIndex + 1).type == TokenType::NamespaceSeparator)
		{
			currentIndex += 2;
		}

		if (tokenAtIndex(currentIndex).type == TokenType::Identifier &&
			tokenAtIndex(currentIndex + 1).type == TokenType::Identifier)
			return (true);

		return (false);
	}

	bool MetaTokenizer::isAssignation() const
	{
		size_t currentIndex = 0;

		if (tokenAtIndex(currentIndex).type == TokenType::NamespaceSeparator)
			currentIndex++;

		while (tokenAtIndex(currentIndex).type == TokenType::Identifier &&
			tokenAtIndex(currentIndex + 1).type == TokenType::NamespaceSeparator)
		{
			currentIndex += 2;
		}

		if (tokenAtIndex(currentIndex).type == TokenType::Identifier &&
			tokenAtIndex(currentIndex + 1).type == TokenType::Assignator)
			return (true);

		return (false);
	}

	bool MetaTokenizer::isSymbolCall() const
	{
		size_t currentIndex = 0;

		if (tokenAtIndex(currentIndex).type == TokenType::NamespaceSeparator)
			currentIndex++;

		while (tokenAtIndex(currentIndex).type == TokenType::Identifier &&
			tokenAtIndex(currentIndex + 1).type == TokenType::NamespaceSeparator)
		{
			currentIndex += 2;
		}

		if (tokenAtIndex(currentIndex).type == TokenType::Identifier &&
			tokenAtIndex(currentIndex + 1).type == TokenType::OpenParenthesis)
			return (true);

		return (false);
	}

	SymbolBody MetaTokenizer::parseSymbolBody()
	{
		SymbolBody result;

		expect(TokenType::OpenCurlyBracket, "Expected a '{' token.");

		while (hasTokenLeft() && currentToken().type != TokenType::CloseCurlyBracket)
		{
			try
			{
				// Determine the type of instruction to parse
				switch (currentToken().type)
				{
				case TokenType::Identifier:
				{
					if (isDeclaration())
					{
						result.instructions.push_back(parseVariableDeclaration());
					}
					else if (isAssignation())
					{
						result.instructions.push_back(parseVariableAssignation());
					}
					else if (isSymbolCall())
					{
						result.instructions.push_back(parseSymbolCall());
					}
					else
					{
						throw TokenBasedError("Unrecognized identifier instruction.", currentToken());
					}
					break;
				}

				case TokenType::IfStatement:
					result.instructions.push_back(parseIfStatement());
					break;

				case TokenType::WhileStatement:
					result.instructions.push_back(parseWhileStatement());
					break;

				case TokenType::ForStatement:
					result.instructions.push_back(parseForStatement());
					break;

				case TokenType::Return:
					result.instructions.push_back(parseReturnStatement());
					break;

				case TokenType::Discard:
					result.instructions.push_back(parseDiscardStatement());
					break;

				default:
					// Throw an error for any unexpected token
					throw TokenBasedError("Unexpected token type in function body.", currentToken());
				}
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
				skipLine(); // Skip to the next line on error
			}
		}

		expect(TokenType::CloseCurlyBracket, "Expected a '}' token.");
		return result;
	}

	std::shared_ptr<VariableDeclaration> MetaTokenizer::parseVariableDeclaration()
	{
		auto declaration = std::make_shared<VariableDeclaration>();

		// Parse type and name
		declaration->descriptor.type = parseTypeDescriptor();
		declaration->descriptor.name = expect(TokenType::Identifier, "Expected variable name.");

		// Check for optional initial value
		if (currentToken().type == TokenType::Assignator)
		{
			expect(TokenType::Assignator, "Expected '=' for variable initialization.");
			declaration->initialValue = parseExpression();
		}

		expect(TokenType::EndOfSentence, "Expected ';' at the end of variable declaration.");

		return declaration;
	}

	std::shared_ptr<VariableAssignation> MetaTokenizer::parseVariableAssignation()
	{
		std::shared_ptr<VariableAssignation> result = std::make_shared<VariableAssignation>();

		result->target = parseVariableDesignation();
		result->value = parseExpression();

		return (result);
	}

	std::shared_ptr<SymbolCall> MetaTokenizer::parseSymbolCall()
	{
		auto result = std::make_shared<SymbolCall>();

		if (currentToken().type == TokenType::NamespaceSeparator)
		{
			result->namespaceChain.push_back(expect(TokenType::NamespaceSeparator, "Expected a namespace separator"));
		}

		while (currentToken().type == TokenType::Identifier && nextToken().type == TokenType::NamespaceSeparator)
		{
			result->namespaceChain.push_back(expect(TokenType::Identifier, "Expected a namespace identifier"));
			result->namespaceChain.push_back(expect(TokenType::NamespaceSeparator, "Expected a namespace separator"));
		}

		result->functionName = expect(TokenType::Identifier, "Expected function name.");
		expect(TokenType::OpenParenthesis, "Expected '(' after function name.");

		while (currentToken().type != TokenType::CloseParenthesis)
		{
			if (result->parameters.size() != 0)
			{
				expect(TokenType::Comma, "Expected ',' between parameters.");
			}
			result->parameters.push_back(parseExpression());
		}

		expect(TokenType::CloseParenthesis, "Expected ')' after parameters.");
		expect(TokenType::EndOfSentence, "Expected ';' at the end of function call.");

		return (result);
	}

	std::shared_ptr<Expression::VariableDesignationElement> MetaTokenizer::parseVariableDesignation()
	{
		auto designation = std::make_shared<Expression::VariableDesignationElement>();

		if (currentToken().type == TokenType::NamespaceSeparator)
		{
			designation->namespaceChain.push_back(expect(TokenType::NamespaceSeparator, "Expected a namespace separator"));
		}

		while (currentToken().type == TokenType::Identifier && nextToken().type == TokenType::NamespaceSeparator)
		{
			designation->namespaceChain.push_back(expect(TokenType::Identifier, "Expected a namespace identifier"));
			designation->namespaceChain.push_back(expect(TokenType::NamespaceSeparator, "Expected a namespace separator"));
		}

		designation->name = expect(TokenType::Identifier, "Expected variable name.");

		while (currentToken().type == TokenType::Accessor || currentToken().type == TokenType::OpenBracket)
		{
			if (currentToken().type == TokenType::Accessor)
			{
				expect(TokenType::Accessor, "Expected a '.' token.");
				auto newAccessor = std::make_shared<Expression::VariableDesignationElement::AccessorElement>();

				newAccessor->name = expect(TokenType::Identifier, "Expected an identifier token.");

				designation->accessors.push_back(newAccessor);
			}
			if (currentToken().type == TokenType::OpenBracket)
			{
				expect(TokenType::OpenBracket, "Expected '[' before array index.");
				designation->accessors.push_back(parseExpression());
				expect(TokenType::CloseBracket, "Expected ']' after array index.");
			}
		}

		return designation;
	}
	std::shared_ptr<Expression::NumberElement> MetaTokenizer::parseNumberElement()
	{
		auto result = std::make_shared<Expression::NumberElement>();

		result->value = expect(TokenType::Number, "Expected a valid number token.");

		return (result);
	}

	std::shared_ptr<Expression::BooleanElement> MetaTokenizer::parseBooleanElement()
	{
		auto result = std::make_shared<Expression::BooleanElement>();

		result->value = expect(TokenType::BoolStatement, "Expected a valid boolean value");

		return (result);
	}

	std::shared_ptr<Expression::OperatorElement> MetaTokenizer::parseOperatorElement()
	{
		auto result = std::make_shared<Expression::OperatorElement>();

		result->operatorToken = expect(TokenType::Operator, "Expected an operator token.");

		return (result);
	}

	std::shared_ptr<Expression::SymbolCallElement> MetaTokenizer::parseSymbolCallElement()
	{
		auto result = std::make_shared<Expression::SymbolCallElement>();

		if (currentToken().type == TokenType::NamespaceSeparator)
		{
			result->namespaceChain.push_back(expect(TokenType::NamespaceSeparator, "Expected a namespace separator"));
		}

		while (currentToken().type == TokenType::Identifier && nextToken().type == TokenType::NamespaceSeparator)
		{
			result->namespaceChain.push_back(expect(TokenType::Identifier, "Expected a namespace identifier"));
			result->namespaceChain.push_back(expect(TokenType::NamespaceSeparator, "Expected a namespace separator"));
		}

		result->functionName = expect(TokenType::Identifier, "Expected function name.");
		expect(TokenType::OpenParenthesis, "Expected '(' after function name.");

		while (currentToken().type != TokenType::CloseParenthesis)
		{
			if (result->parameters.size() != 0)
			{
				expect(TokenType::Comma, "Expected ',' between parameters.");
			}
			result->parameters.push_back(parseExpression());
		}

		expect(TokenType::CloseParenthesis, "Expected ')' after parameters.");

		return (result);
	}

	std::shared_ptr<Instruction> MetaTokenizer::parseExpressionElement()
	{
		switch (currentToken().type)
		{
		case TokenType::Number:
			return (parseNumberElement());
			break;

		case TokenType::BoolStatement:
			return (parseBooleanElement());
			break;

		case TokenType::Identifier:
			if (isSymbolCall() == true)
			{
				return (parseSymbolCallElement());
			}
			else
			{
				return (parseVariableDesignation());
			}
			break;

		case TokenType::Operator:
			return (parseOperatorElement());
			break;

		case TokenType::OpenParenthesis:
			return (parseExpression());
			break;

		default:
			throw TokenBasedError("Unexpected token in expression.", currentToken());
		}
	}

	std::shared_ptr<Expression> MetaTokenizer::parseExpression()
	{
		auto expression = std::make_shared<Expression>();

		expression->elements.push_back(parseExpressionElement());

		while (currentToken().type == TokenType::Operator)
		{
			expression->elements.push_back(parseOperatorElement());
			expression->elements.push_back(parseExpressionElement());
		}

		return expression;
	}

	std::shared_ptr<IfStatement> MetaTokenizer::parseIfStatement()
	{
		auto ifStatement = std::make_shared<IfStatement>();

		ConditionalBranch branch;
		expect(TokenType::IfStatement, "Expected 'if'.");
		expect(TokenType::OpenParenthesis, "Expected '(' after 'if'.");
		branch.condition = parseExpression();
		expect(TokenType::CloseParenthesis, "Expected ')' after condition.");

		branch.body = parseSymbolBody().instructions;

		ifStatement->branches.push_back(branch);

		while (currentToken().type == TokenType::ElseStatement)
		{
			advance();

			if (currentToken().type == TokenType::IfStatement)
			{
				advance();
				expect(TokenType::OpenParenthesis, "Expected '(' after 'else if'.");
				branch.condition = parseExpression();
				expect(TokenType::CloseParenthesis, "Expected ')' after condition.");
			}
			else
			{
				branch.condition = nullptr;
			}

			branch.body = parseSymbolBody().instructions;
			ifStatement->branches.push_back(branch);
		}

		return ifStatement;
	}

	std::shared_ptr<WhileStatement> MetaTokenizer::parseWhileStatement()
	{
		auto whileStatement = std::make_shared<WhileStatement>();

		expect(TokenType::WhileStatement, "Expected 'while'.");
		expect(TokenType::OpenParenthesis, "Expected '(' after 'while'.");
		whileStatement->condition = parseExpression();
		expect(TokenType::CloseParenthesis, "Expected ')' after condition.");
		whileStatement->body = parseSymbolBody().instructions;

		return whileStatement;
	}

	std::shared_ptr<ForStatement> MetaTokenizer::parseForStatement()
	{
		auto forStatement = std::make_shared<ForStatement>();

		expect(TokenType::ForStatement, "Expected 'for'.");
		expect(TokenType::OpenParenthesis, "Expected '(' after 'for'.");

		// Parse initializer, condition, and increment
		forStatement->initializer = parseVariableDeclaration();
		forStatement->condition = parseExpression();
		expect(TokenType::EndOfSentence, "Expected ';' after condition.");
		forStatement->increment = parseExpression();

		expect(TokenType::CloseParenthesis, "Expected ')' after for-loop header.");
		forStatement->body = parseSymbolBody().instructions;

		return forStatement;
	}

	std::shared_ptr<ReturnStatement> MetaTokenizer::parseReturnStatement()
	{
		auto returnStatement = std::make_shared<ReturnStatement>();

		expect(TokenType::Return, "Expected 'return'.");
		if (currentToken().type != TokenType::EndOfSentence)
		{
			returnStatement->returnValue = parseExpression();
		}

		expect(TokenType::EndOfSentence, "Expected ';' after return statement.");

		return returnStatement;
	}

	std::shared_ptr<DiscardStatement> MetaTokenizer::parseDiscardStatement()
	{
		auto discardStatement = std::make_shared<DiscardStatement>();

		expect(TokenType::Discard, "Expected 'discard'.");
		expect(TokenType::EndOfSentence, "Expected ';' after discard statement.");

		return discardStatement;
	}
}
