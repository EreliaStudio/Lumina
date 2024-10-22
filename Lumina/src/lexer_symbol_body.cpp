#include "lexer.hpp"
#include "token.hpp"

namespace Lumina
{
    int Lexer::computeOperatorPriority(const Token& token)
    {
        if (token.type == Lumina::Token::Type::Operator)
        {
            if (token.content == "*" || token.content == "/")
                return 5;
            if (token.content == "+" || token.content == "-")
                return 4;
            if (token.content == "<" || token.content == ">" || token.content == "<=" || token.content == ">=")
                return 3;
            if (token.content == "==" || token.content == "!=")
                return 2;
            if (token.content == "&&")
                return 1;
            if (token.content == "||")
                return 0;
        }
        return -1;
    }

    SymbolBodyInfo Lexer::parseSymbolBody()
    {
        SymbolBodyInfo result;

        expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start symbol body.");

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
                    result.statements.push_back(parseStatement());
                }
            }
            catch (TokenBasedError& e)
            {
                _product.errors.push_back(e);
                skipLine();
            }
        }

        expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to end symbol body.");

        return result;
    }

    Statement Lexer::parseStatement()
    {
        if (currentToken().type == Lumina::Token::Type::Return)
        {
            return parseReturnStatement();
        }
        else if (currentToken().type == Lumina::Token::Type::Discard)
        {
            return parseDiscardStatement();
        }
        else if (currentToken().type == Lumina::Token::Type::IfStatement)
        {
            return parseIfStatement();
        }
        else if (currentToken().type == Lumina::Token::Type::WhileStatement)
        {
            return parseWhileStatement();
        }
        else if (currentToken().type == Lumina::Token::Type::ForStatement)
        {
            return parseForStatement();
        }
        else if (currentToken().type == Lumina::Token::Type::OpenCurlyBracket)
        {
            return parseCompoundStatement();
        }
        else if (isVariableDeclaration())
        {
            return parseVariableDeclarationStatement();
        }
        else if (isAssignmentStatement())
        {
            return parseAssignmentStatement();
        }
        else
        {
            return parseExpressionStatement();
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
            auto targetExpr = parseExpression();
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

    VariableDeclarationStatement Lexer::parseVariableDeclarationStatement()
    {
        VariableDeclarationStatement result;
        result.variable = parseVariableInfo();

        if (currentToken().type == Lumina::Token::Type::Assignator)
        {
            advance();
            result.initializer = parseExpression();
        }

        expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after variable declaration.");

        return result;
    }

    ExpressionStatement Lexer::parseExpressionStatement()
    {
        ExpressionStatement result;
        result.expression = parseExpression();
        expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after expression statement.");
        return result;
    }

    AssignmentStatement Lexer::parseAssignmentStatement()
    {
        AssignmentStatement result;
        result.target = parseExpression();

        result.operatorToken = expect(Lumina::Token::Type::Assignator, "Expected '=' token.");

        result.value = parseExpression();

        expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after assignment statement.");

        return result;
    }

    ReturnStatement Lexer::parseReturnStatement()
    {
        ReturnStatement result;
        expect(Lumina::Token::Type::Return, "Expected 'return' keyword.");

        if (currentToken().type != Lumina::Token::Type::EndOfSentence)
        {
            result.expression = parseExpression();
        }

        expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after return statement.");

        return result;
    }

    DiscardStatement Lexer::parseDiscardStatement()
    {
        DiscardStatement result;
        expect(Lumina::Token::Type::Discard, "Expected 'discard' keyword.");
        expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after discard statement.");
        return result;
    }

    IfStatement Lexer::parseIfStatement()
    {
        IfStatement result;
        expect(Lumina::Token::Type::IfStatement, "Expected 'if' keyword.");

        expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'if'.");
        auto condition = parseExpression();
        expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after condition.");

        ConditionalBranch branch;
        branch.condition = condition;
        branch.body = parseSymbolBody();
        result.branches.push_back(branch);

        while (currentToken().type == Lumina::Token::Type::ElseStatement)
        {
            advance();
            if (currentToken().type == Lumina::Token::Type::IfStatement)
            {
                advance();
                expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'else if'.");
                auto elseIfCondition = parseExpression();
                expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after condition.");

                ConditionalBranch elseIfBranch;
                elseIfBranch.condition = elseIfCondition;
                elseIfBranch.body = parseSymbolBody();
                result.branches.push_back(elseIfBranch);
            }
            else
            {
                result.elseBody = parseSymbolBody();
                break;
            }
        }

        return result;
    }

    WhileStatement Lexer::parseWhileStatement()
    {
        WhileStatement result;
        expect(Lumina::Token::Type::WhileStatement, "Expected 'while' keyword.");
        expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'while'.");
        auto condition = parseExpression();
        expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after condition.");

        ConditionalBranch loop;
        loop.condition = condition;
        loop.body = parseSymbolBody();
        result.loop = loop;

        return result;
    }

    ForStatement Lexer::parseForStatement()
    {
        ForStatement result;
        expect(Lumina::Token::Type::ForStatement, "Expected 'for' keyword.");
        expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' after 'for'.");

        if (currentToken().type != Lumina::Token::Type::EndOfSentence)
        {
            if (isVariableDeclaration())
            {
                result.initializer = std::make_shared<Statement>(parseVariableDeclarationStatement());
            }
            else
            {
                result.initializer = std::make_shared<Statement>(parseExpressionStatement());
            }
        }

        if (currentToken().type != Lumina::Token::Type::EndOfSentence)
        {
            result.condition = parseExpression();
        }
        expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after loop condition.");

        if (currentToken().type != Lumina::Token::Type::CloseParenthesis)
        {
            result.increment = parseExpression();
        }
        expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after for loop control.");

        result.body = parseSymbolBody();

        return result;
    }

    CompoundStatement Lexer::parseCompoundStatement()
    {
        CompoundStatement result;
        expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start compound statement.");

        while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
        {
            if (currentToken().type == Lumina::Token::Type::EndOfSentence)
            {
                skipToken();
            }
            else
            {
                result.statements.push_back(parseStatement());
            }
        }

        expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to end compound statement.");

        return result;
    }

    std::shared_ptr<Expression> Lexer::parseExpression()
    {
        return parseAssignmentExpression();
    }

    std::shared_ptr<Expression> Lexer::parseAssignmentExpression()
    {
        auto left = parseBinaryExpression(0);

        if (currentToken().type == Lumina::Token::Type::Assignator)
        {
            Token opToken = currentToken();
            advance();
            auto right = parseAssignmentExpression();

            BinaryExpression binExpr;
            binExpr.left = left;
            binExpr.operatorToken = opToken;
            binExpr.right = right;

            return std::make_shared<Expression>(binExpr);
        }
        else
        {
            return left;
        }
    }

    std::shared_ptr<Expression> Lexer::parseBinaryExpression(int minPrecedence)
    {
        auto left = parseUnaryExpression();

        while (true)
        {
            Token opToken = currentToken();
            int precedence = computeOperatorPriority(opToken);
            if (precedence < minPrecedence)
                break;

            advance();

            auto right = parseBinaryExpression(precedence + 1);

            BinaryExpression binExpr;
            binExpr.left = left;
            binExpr.operatorToken = opToken;
            binExpr.right = right;

            left = std::make_shared<Expression>(binExpr);
        }

        return left;
    }

    std::shared_ptr<Expression> Lexer::parseUnaryExpression()
    {
        if (currentToken().type == Lumina::Token::Type::Operator &&
            (currentToken().content == "-" || currentToken().content == "+" || currentToken().content == "!" || currentToken().content == "~"))
        {
            Token opToken = currentToken();
            advance();
            UnaryExpression unaryExpr;
            unaryExpr.operatorToken = opToken;
            unaryExpr.operand = parseUnaryExpression();
            return std::make_shared<Expression>(unaryExpr);
        }
        else
        {
            return parsePostfixExpression();
        }
    }

    std::shared_ptr<Expression> Lexer::parsePostfixExpression()
    {
        auto expr = parsePrimaryExpression();

        while (true)
        {
            if (currentToken().type == Lumina::Token::Type::Accessor)
            {
                advance();
                Token memberName = expect(Lumina::Token::Type::Identifier, "Expected member name after '.'.");
                MemberAccessExpression memberExpr;
                memberExpr.object = expr;
                memberExpr.memberName = memberName;
                expr = std::make_shared<Expression>(memberExpr);
            }
            else if (currentToken().type == Lumina::Token::Type::OpenBracket)
            {
                advance();
                auto indexExpr = parseExpression();
                expect(Lumina::Token::Type::CloseBracket, "Expected ']' after array index.");
                ArrayAccessExpression arrayAccessExpr;
                arrayAccessExpr.array = expr;
                arrayAccessExpr.index = indexExpr;
                expr = std::make_shared<Expression>(arrayAccessExpr);
            }
            else if (currentToken().type == Lumina::Token::Type::Incrementor)
            {
                Token opToken = currentToken();
                advance();

                PostfixExpression postfixExpr;
                postfixExpr.operand = expr;
                postfixExpr.operatorToken = opToken;

                expr = std::make_shared<Expression>(postfixExpr);
            }
            else
            {
                break;
            }
        }

        return expr;
    }

    std::shared_ptr<Expression> Lexer::parsePrimaryExpression()
    {
        if (currentToken().type == Lumina::Token::Type::Number ||
            currentToken().type == Lumina::Token::Type::StringLitteral ||
            currentToken().type == Lumina::Token::Type::BoolStatement)
        {
            LiteralExpression literalExpr;
            literalExpr.value = currentToken();
            advance();
            return std::make_shared<Expression>(literalExpr);
        }
        else if (currentToken().type == Lumina::Token::Type::Identifier ||
                 currentToken().type == Lumina::Token::Type::ThisKeyword)
        {
            return parseVariableOrFunctionCallExpression();
        }
        else if (currentToken().type == Lumina::Token::Type::OpenParenthesis)
        {
            advance();
            auto expr = parseExpression();
            expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after expression.");
            return expr;
        }
        else
        {
            throw TokenBasedError("Unexpected token in expression.", currentToken());
        }
    }

    std::shared_ptr<Expression> Lexer::parseVariableOrFunctionCallExpression()
    {
        NamespaceDesignation nspace = parseNamespaceDesignation();

        Token nameToken = expect({ Lumina::Token::Type::Identifier, Lumina::Token::Type::ThisKeyword }, "Expected identifier.");

        if (currentToken().type == Lumina::Token::Type::OpenParenthesis)
        {
            FunctionCallExpression funcCallExpr;

            funcCallExpr.namespacePath = nspace;
            funcCallExpr.functionName = nameToken;

            advance();

            while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
            {
                if (funcCallExpr.arguments.size() != 0)
                {
                    expect(Lumina::Token::Type::Comma, "Expected ',' between arguments.");
                }
                funcCallExpr.arguments.push_back(parseExpression());
            }

            expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after function arguments.");

            return std::make_shared<Expression>(funcCallExpr);
        }
        else
        {
            VariableExpression varExpr;

            varExpr.namespacePath = nspace;
            varExpr.variableName = nameToken;

            return std::make_shared<Expression>(varExpr);
        }
    }
}
