#include "lexer.hpp"
#include "token.hpp"

#include "utils.hpp"

namespace Lumina
{
	int Lexer::computeOperatorPriority(const Token& p_token)
	{
		if (p_token.type == Lumina::Token::Type::Operator)
		{
			if (p_token.content == "*" || p_token.content == "/" || p_token.content == "%")
			{
				return 5;
			}
			if (p_token.content == "+" || p_token.content == "-")
			{
				return 4;
			}
			if (p_token.content == "<" || p_token.content == ">" || p_token.content == "<=" || p_token.content == ">=")
			{
				return 3;
			}
			if (p_token.content == "==" || p_token.content == "!=")
			{
				return 2;
			}
			if (p_token.content == "&&")
			{
				return 1;
			}
			if (p_token.content == "||")
			{
				return 0;
			}
		}
		return -1;
	}

	SymbolBodyInfo Lexer::parseSymbolBodyInfo()
	{
		SymbolBodyInfo result;

		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start symbol body." + DEBUG_INFORMATION);

		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				if (currentToken().type == Lumina::Token::Type::Comment)
				{
					skipToken();
				}
				else if (currentToken().type == Lumina::Token::Type::EndOfSentence)
				{
					skipToken();
				}
				else
				{
					result.statements.push_back(parseStatementInfo());
				}
			}
			catch (TokenBasedError& e)
			{
				_product.errors.push_back(e);
				skipLine();
			}
		}

		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to end symbol body." + DEBUG_INFORMATION);

		return result;
	}

	StatementInfo Lexer::parseStatementInfo()
	{
		if (currentToken().type == Lumina::Token::Type::Return)
		{
			return parseReturnStatementInfo();
		}
		else if (currentToken().type == Lumina::Token::Type::Discard)
		{
			return parseDiscardStatementInfo();
		}
		else if (currentToken().type == Lumina::Token::Type::IfStatement)
		{
			return parseIfStatementInfo();
		}
		else if (currentToken().type == Lumina::Token::Type::WhileStatement)
		{
			return parseWhileStatementInfo();
		}
		else if (currentToken().type == Lumina::Token::Type::ForStatement)
		{
			return parseForStatementInfo();
		}
		else if (currentToken().type == Lumina::Token::Type::OpenCurlyBracket)
		{
			return parseCompoundStatementInfo();
		}
		else if (isVariableDeclaration())
		{
			return parseVariableDeclarationStatementInfo();
		}
		else if (isAssignmentStatement())
		{
			return parseAssignmentStatementInfo();
		}
		else
		{
			return parseExpressionStatementInfo();
		}
	}

	bool Lexer::isVariableDeclaration()
	{
		size_t offset = 0;

		if (tokenAtOffset(offset).type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset++;
		}

		while (tokenAtOffset(offset).type == Lumina::Token::Type::Identifier &&
			tokenAtOffset(offset + 1).type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset += 2;
		}

		if (tokenAtOffset(offset).type != Lumina::Token::Type::Identifier)
		{
			return false;
		}
		offset++;

		if (tokenAtOffset(offset).type != Lumina::Token::Type::Identifier)
		{
			return false;
		}

		return true;
	}

	bool Lexer::isAssignmentStatement()
	{
		size_t savedIndex = _index;
		try
		{
			auto targetExpr = parseExpressionInfo();
			if (currentToken().type == Lumina::Token::Type::Assignator)
			{
				_index = savedIndex;
				return true;
			}
		}
		catch (...)
		{

		}
		_index = savedIndex;
		return false;
	}

	VariableDeclarationStatementInfo Lexer::parseVariableDeclarationStatementInfo()
	{
		VariableDeclarationStatementInfo result;
		result.variable = parseVariableInfo();

		if (currentToken().type == Lumina::Token::Type::Assignator)
		{
			advance();
			result.initializer = parseExpressionInfo();
		}

		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after variable declaration." + DEBUG_INFORMATION);

		return result;
	}

	ExpressionStatementInfo Lexer::parseExpressionStatementInfo()
	{
		ExpressionStatementInfo result;
		result.expression = parseExpressionInfo();
		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after expression statement." + DEBUG_INFORMATION);
		return result;
	}

	AssignmentStatementInfo Lexer::parseAssignmentStatementInfo()
	{
		AssignmentStatementInfo result;
		result.target = parseExpressionInfo();

		result.operatorToken = expect(Lumina::Token::Type::Assignator, "Expected '=' token." + DEBUG_INFORMATION);

		result.value = parseExpressionInfo();

		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after assignment statement." + DEBUG_INFORMATION);

		return result;
	}

	ReturnStatementInfo Lexer::parseReturnStatementInfo()
	{
		ReturnStatementInfo result;
		expect(Lumina::Token::Type::Return, "Expected 'return' keyword." + DEBUG_INFORMATION);

		if (currentToken().type != Lumina::Token::Type::EndOfSentence)
		{
			result.expression = parseExpressionInfo();
		}

		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after return statement." + DEBUG_INFORMATION);

		return result;
	}

	DiscardStatementInfo Lexer::parseDiscardStatementInfo()
	{
		DiscardStatementInfo result;
		expect(Lumina::Token::Type::Discard, "Expected 'discard' keyword." + DEBUG_INFORMATION);
		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after discard statement." + DEBUG_INFORMATION);
		return result;
	}

	IfStatementInfo Lexer::parseIfStatementInfo()
	{
		IfStatementInfo result;
		expect(Lumina::Token::Type::IfStatement, "Expected 'if' keyword." + DEBUG_INFORMATION);

		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'if'." + DEBUG_INFORMATION);
		auto condition = parseExpressionInfo();
		expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after condition." + DEBUG_INFORMATION);

		ConditionalBranch branch;
		branch.condition = condition;
		branch.body = parseSymbolBodyInfo();
		result.branches.push_back(branch);

		while (currentToken().type == Lumina::Token::Type::ElseStatement)
		{
			advance();
			if (currentToken().type == Lumina::Token::Type::IfStatement)
			{
				advance();
				expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'else if'." + DEBUG_INFORMATION);
				auto elseIfCondition = parseExpressionInfo();
				expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after condition." + DEBUG_INFORMATION);

				ConditionalBranch elseIfBranch;
				elseIfBranch.condition = elseIfCondition;
				elseIfBranch.body = parseSymbolBodyInfo();
				result.branches.push_back(elseIfBranch);
			}
			else
			{
				result.elseBody = parseSymbolBodyInfo();
				break;
			}
		}

		return result;
	}

	WhileStatementInfo Lexer::parseWhileStatementInfo()
	{
		WhileStatementInfo result;
		expect(Lumina::Token::Type::WhileStatement, "Expected 'while' keyword." + DEBUG_INFORMATION);
		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'while'." + DEBUG_INFORMATION);
		auto condition = parseExpressionInfo();
		expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after condition." + DEBUG_INFORMATION);

		ConditionalBranch loop;
		loop.condition = condition;
		loop.body = parseSymbolBodyInfo();
		result.loop = loop;

		return result;
	}

	ForStatementInfo Lexer::parseForStatementInfo()
	{
		ForStatementInfo result;
		expect(Lumina::Token::Type::ForStatement, "Expected 'for' keyword." + DEBUG_INFORMATION);
		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'for'." + DEBUG_INFORMATION);

		if (currentToken().type != Lumina::Token::Type::EndOfSentence)
		{
			if (isVariableDeclaration())
			{
				result.initializer = std::make_shared<StatementInfo>(parseVariableDeclarationStatementInfo());
			}
			else
			{
				result.initializer = std::make_shared<StatementInfo>(parseExpressionStatementInfo());
			}
		}

		if (currentToken().type != Lumina::Token::Type::EndOfSentence)
		{
			result.condition = parseExpressionInfo();
		}
		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after loop condition." + DEBUG_INFORMATION);

		if (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			result.increment = parseExpressionInfo();
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after for loop control." + DEBUG_INFORMATION);

		result.body = parseSymbolBodyInfo();

		return result;
	}

	CompoundStatementInfo Lexer::parseCompoundStatementInfo()
	{
		CompoundStatementInfo result;

		result.body = parseSymbolBodyInfo();

		return result;
	}

	std::shared_ptr<ExpressionInfo> Lexer::parseExpressionInfo()
	{
		return parseAssignmentExpressionInfo();
	}

	std::shared_ptr<ExpressionInfo> Lexer::parseAssignmentExpressionInfo()
	{
		auto left = parseBinaryExpressionInfo(0);

		if (currentToken().type == Lumina::Token::Type::Assignator)
		{
			Token opToken = currentToken();
			advance();
			auto right = parseAssignmentExpressionInfo();

			BinaryExpressionInfo binExpr;
			binExpr.left = left;
			binExpr.operatorToken = opToken;
			binExpr.right = right;

			return std::make_shared<ExpressionInfo>(binExpr);
		}
		else
		{
			return left;
		}
	}

	std::shared_ptr<ExpressionInfo> Lexer::parseBinaryExpressionInfo(int minPrecedence)
	{
		auto left = parseUnaryExpressionInfo();

		while (true)
		{
			Token opToken = currentToken();
			int precedence = computeOperatorPriority(opToken);
			if (precedence < minPrecedence)
			{
				break;
			}

			advance();

			auto right = parseBinaryExpressionInfo(precedence + 1);

			BinaryExpressionInfo binExpr;
			binExpr.left = left;
			binExpr.operatorToken = opToken;
			binExpr.right = right;

			left = std::make_shared<ExpressionInfo>(binExpr);
		}

		return left;
	}

	std::shared_ptr<ExpressionInfo> Lexer::parseUnaryExpressionInfo()
	{
		if (currentToken().type == Lumina::Token::Type::Operator &&
			(currentToken().content == "-" || currentToken().content == "+"))
		{
			Token opToken = currentToken();
			advance();
			UnaryExpressionInfo unaryExpr;
			unaryExpr.operatorToken = opToken;
			unaryExpr.operand = parseUnaryExpressionInfo();
			return std::make_shared<ExpressionInfo>(unaryExpr);
		}
		else
		{
			return parsePostfixExpressionInfo();
		}
	}

	std::shared_ptr<ExpressionInfo> Lexer::parsePostfixExpressionInfo()
	{
		auto expr = parsePrimaryExpressionInfo();

		while (true)
		{
			if (currentToken().type == Lumina::Token::Type::Accessor)
			{
				advance();
				if (tokenAtOffset(1).type == Lumina::Token::Type::OpenParenthesis)
				{
					MethodCallExpressionInfo methodExpr;
					methodExpr.object = expr;
					methodExpr.name = expect(Lumina::Token::Type::Identifier, "Expected method name after '.'." + DEBUG_INFORMATION);

					expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after method name." + DEBUG_INFORMATION);
					while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
					{
						if (methodExpr.arguments.size() != 0)
						{
							expect(Lumina::Token::Type::Comma, "Expected ',' between arguments." + DEBUG_INFORMATION);
						}
						methodExpr.arguments.push_back(parseExpressionInfo());
					}
					expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after method name." + DEBUG_INFORMATION);

					expr = std::make_shared<ExpressionInfo>(methodExpr);
				}
				else
				{
					MemberAccessExpressionInfo memberExpr;
					memberExpr.object = expr;
					memberExpr.memberName = expect(Lumina::Token::Type::Identifier, "Expected member name after '.'." + DEBUG_INFORMATION);
					expr = std::make_shared<ExpressionInfo>(memberExpr);
				}
				
			}
			else if (currentToken().type == Lumina::Token::Type::OpenBracket)
			{
				advance();
				auto indexExpr = parseExpressionInfo();
				expect(Lumina::Token::Type::CloseBracket, "Expected ']' after array index." + DEBUG_INFORMATION);
				ArrayAccessExpressionInfo arrayAccessExpr;
				arrayAccessExpr.array = expr;
				arrayAccessExpr.index = indexExpr;
				expr = std::make_shared<ExpressionInfo>(arrayAccessExpr);
			}
			else if (currentToken().type == Lumina::Token::Type::Incrementor)
			{
				Token opToken = currentToken();
				advance();

				PostfixExpressionInfo postfixExpr;
				postfixExpr.operand = expr;
				postfixExpr.operatorToken = opToken;

				expr = std::make_shared<ExpressionInfo>(postfixExpr);
			}
			else
			{
				break;
			}
		}

		return expr;
	}

	std::shared_ptr<ExpressionInfo> Lexer::parsePrimaryExpressionInfo()
	{
		if (currentToken().type == Lumina::Token::Type::Number ||
			currentToken().type == Lumina::Token::Type::StringLitteral ||
			currentToken().type == Lumina::Token::Type::BoolStatement)
		{
			LiteralExpressionInfo literalExpr;
			literalExpr.value = currentToken();
			advance();
			return std::make_shared<ExpressionInfo>(literalExpr);
		}
		else if (currentToken().type == Lumina::Token::Type::Identifier ||
				 currentToken().type == Lumina::Token::Type::ThisKeyword)
		{
			return parseVariableOrFunctionCallExpressionInfo();
		}
		else if (currentToken().type == Lumina::Token::Type::OpenCurlyBracket)
		{
			ArrayDefinitionExpressionInfo arrayInfo;

			advance();
			while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
			{
				if (arrayInfo.elements.size() != 0)
				{
					expect(Lumina::Token::Type::Comma, "Expected ',' between array definition elements." + DEBUG_INFORMATION);
				}
				arrayInfo.elements.push_back(parseExpressionInfo());
			}
			expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a '}' to close the array definition expression.");

			return (std::make_shared<ExpressionInfo>(arrayInfo));
		}
		else if (currentToken().type == Lumina::Token::Type::OpenParenthesis)
		{
			advance();
			auto expr = parseExpressionInfo();
			expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after expression." + DEBUG_INFORMATION);
			return expr;
		}
		else
		{
			throw TokenBasedError("Unexpected token in expression." + DEBUG_INFORMATION, currentToken());
			return (nullptr);
		}
	}

	std::shared_ptr<ExpressionInfo> Lexer::parseVariableOrFunctionCallExpressionInfo()
	{
		NamespaceDesignation nspace = parseNamespaceDesignation();

		Token nameToken = expect({ Lumina::Token::Type::Identifier, Lumina::Token::Type::ThisKeyword }, "Expected identifier." + DEBUG_INFORMATION);

		if (currentToken().type == Lumina::Token::Type::OpenParenthesis)
		{
			FunctionCallExpressionInfo funcCallExpr;

			funcCallExpr.namespacePath = nspace;
			funcCallExpr.functionName = nameToken;

			advance();

			while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
			{
				if (funcCallExpr.arguments.size() != 0)
				{
					expect(Lumina::Token::Type::Comma, "Expected ',' between arguments." + DEBUG_INFORMATION);
				}
				std::shared_ptr<ExpressionInfo> expression = parseExpressionInfo();

				funcCallExpr.arguments.push_back(expression);
			}

			expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after function arguments." + DEBUG_INFORMATION);

			return std::make_shared<ExpressionInfo>(funcCallExpr);
		}
		else
		{
			VariableExpressionInfo varExpr;

			varExpr.namespacePath = nspace;
			varExpr.variableName = nameToken;

			return std::make_shared<ExpressionInfo>(varExpr);
		}
	}
}
