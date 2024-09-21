#include "lumina_metatokenizer.hpp"
#include "lumina_instruction.hpp"

namespace Lumina
{
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
					if (isType(currentToken()))
					{
						result.instructions.push_back(parseVariableDeclaration());
					}
					else
					{
						result.instructions.push_back(parseVariableAssignationOrSymbolCall());
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
			declaration->initialValue = std::make_shared<Expression>(parseExpression());
		}

		expect(TokenType::EndOfSentence, "Expected ';' at the end of variable declaration.");

		return declaration;
	}

	std::shared_ptr<Instruction> MetaTokenizer::parseVariableAssignationOrSymbolCall()
	{
		size_t startIndex = _index; // Save the current index to backtrack if needed

		try
		{
			auto variableAssignation = std::make_shared<VariableAssignation>();

			// Parse the target variable
			variableAssignation->target = std::make_shared<Expression::VariableDesignationElement>(parseVariableDesignation());

			// Check for assignment '='
			expect(TokenType::Assignator, "Expected '=' for variable assignation.");
			variableAssignation->value = std::make_shared<Expression>(parseExpression());

			expect(TokenType::EndOfSentence, "Expected ';' at the end of variable assignation.");
			return variableAssignation;
		}
		catch (const TokenBasedError&)
		{
			_index = startIndex; // Backtrack and try parsing as a symbol call
		}

		// Parse symbol call as fallback
		auto symbolCall = std::make_shared<SymbolCall>();
		symbolCall->functionName = expect(TokenType::Identifier, "Expected function name.");
		expect(TokenType::OpenParenthesis, "Expected '(' after function name.");

		while (currentToken().type != TokenType::CloseParenthesis)
		{
			if (!symbolCall->parameters.empty())
			{
				expect(TokenType::Comma, "Expected ',' between parameters.");
			}
			symbolCall->parameters.push_back(std::make_shared<Expression>(parseExpression()));
		}

		expect(TokenType::CloseParenthesis, "Expected ')' after parameters.");
		expect(TokenType::EndOfSentence, "Expected ';' at the end of function call.");

		return symbolCall;
	}

	std::shared_ptr<Expression::VariableDesignationElement> MetaTokenizer::parseVariableDesignation()
	{
		auto designation = std::make_shared<Expression::VariableDesignationElement>();

		// Parse namespace chain
		while (currentToken().type == TokenType::Identifier && nextToken().type == TokenType::NamespaceSeparator)
		{
			designation->namespaceChain.push_back(currentToken());
			advance(); // Skip the identifier
			advance(); // Skip the '::'
		}

		designation->name = expect(TokenType::Identifier, "Expected variable name.");

		// Optional array indexing
		if (currentToken().type == TokenType::OpenBracket)
		{
			advance(); // Skip '['
			designation->index = std::make_shared<Expression>(parseExpression());
			expect(TokenType::CloseBracket, "Expected ']' after array index.");
		}

		return designation;
	}

	std::shared_ptr<Expression> MetaTokenizer::parseExpression()
	{
		auto expression = std::make_shared<Expression>();

		// Parse individual elements of the expression
		while (currentToken().type != TokenType::EndOfSentence && currentToken().type != TokenType::CloseParenthesis)
		{
			switch (currentToken().type)
			{
			case TokenType::Number:
				expression->elements.push_back(std::make_shared<Expression::NumberElement>());
				break;

			case TokenType::BoolStatement:
				expression->elements.push_back(std::make_shared<Expression::BooleanElement>());
				break;

			case TokenType::Identifier:
				expression->elements.push_back(std::make_shared<Expression::VariableDesignationElement>(parseVariableDesignation()));
				break;

			case TokenType::Operator:
				expression->elements.push_back(std::make_shared<Expression::OperatorElement>());
				break;

			case TokenType::OpenParenthesis:
				expression->elements.push_back(parseExpression());
				break;

			default:
				throw TokenBasedError("Unexpected token in expression.", currentToken());
			}

			advance();
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
		forStatement->increment = parseVariableAssignationOrSymbolCall();

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
			returnStatement->returnValue = std::make_shared<Expression>(parseExpression());
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
a