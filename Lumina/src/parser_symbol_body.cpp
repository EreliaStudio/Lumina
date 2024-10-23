#include "parser.hpp"

#include <unordered_map>

namespace Lumina
{
    Parser::ArithmeticOperator Parser::_stringToOperator(const std::string& opStr)
    {
        static const std::unordered_map<std::string, ArithmeticOperator> operatorMap = {
            { "+", ArithmeticOperator::Plus },
            { "-", ArithmeticOperator::Minus },
            { "*", ArithmeticOperator::Multiply },
            { "/", ArithmeticOperator::Divide },
            { "%", ArithmeticOperator::Modulo },
            { "==", ArithmeticOperator::Equal },
            { "!=", ArithmeticOperator::NotEqual },
            { "<", ArithmeticOperator::Less },
            { ">", ArithmeticOperator::Greater },
            { "<=", ArithmeticOperator::LessEqual },
            { ">=", ArithmeticOperator::GreaterEqual },
            { "&&", ArithmeticOperator::LogicalAnd },
            { "||", ArithmeticOperator::LogicalOr },
            { "=", ArithmeticOperator::Equal },
            { "+=", ArithmeticOperator::PlusEqual },
            { "-=", ArithmeticOperator::MinusEqual },
            { "*=", ArithmeticOperator::MultiplyEqual },
            { "/=", ArithmeticOperator::DivideEqual },
            { "%=", ArithmeticOperator::ModuloEqual }
        };

        auto it = operatorMap.find(opStr);
        if (it != operatorMap.end())
        {
            return it->second;
        }
        else
        {
            std::cout << "Token content [" << opStr << "]" << std::endl;
            throw std::runtime_error("Unknown operator: " + opStr);
        }
    }

    Parser::UnaryOperator Parser::_stringToUnaryOperator(const std::string& opStr)
    {
        static const std::unordered_map<std::string, UnaryOperator> unaryOperatorMap = {
            { "++", UnaryOperator::Increment },
            { "--", UnaryOperator::Decrement }
        };

        auto it = unaryOperatorMap.find(opStr);
        if (it != unaryOperatorMap.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("Unknown unary operator: " + opStr);
        }
    }

    Parser::AssignatorOperator Parser::_stringToAssignatorOperator(const std::string& opStr)
    {
        static const std::unordered_map<std::string, AssignatorOperator> assignatorOperatorMap = {
            { "=", AssignatorOperator::Equal },
            { "+=", AssignatorOperator::PlusEqual },
            { "-=", AssignatorOperator::MinusEqual },
            { "*=", AssignatorOperator::MultiplyEqual },
            { "/=", AssignatorOperator::DivideEqual },
            { "%=", AssignatorOperator::ModuloEqual }
        };

        auto it = assignatorOperatorMap.find(opStr);
        if (it != assignatorOperatorMap.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("Unknown unary operator: " + opStr);
        }
    }

    Parser::SymbolBody Parser::_composeSymbolBody(const SymbolBodyInfo& p_symbolBodyInfo)
    {
        SymbolBody result;

        for (const auto& statementVariant : p_symbolBodyInfo.statements)
        {
            auto statement = _composeStatement(statementVariant);
            result.statements.push_back(std::move(statement));
        }

        return result;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeStatement(const StatementInfo& p_statementInfo)
    {
        std::shared_ptr<Parser::Statement> result = nullptr;

        switch (p_statementInfo.index())
        {
        case 0: // VariableDeclarationStatementInfo
        {
            result = _composeVariableDeclarationStatement(std::get<0>(p_statementInfo));
            break;
        }
        case 1: // ExpressionStatementInfo
        {
            result = _composeExpressionStatement(std::get<1>(p_statementInfo));
            break;
        }
        case 2: // AssignmentStatementInfo
        {
            result = _composeAssignmentStatement(std::get<2>(p_statementInfo));
            break;
        }
        case 3: // ReturnStatementInfo
        {
            result = _composeReturnStatement(std::get<3>(p_statementInfo));
            break;
        }
        case 4: // DiscardStatementInfo
        {
            result = _composeDiscardStatement(std::get<4>(p_statementInfo));
            break;
        }
        case 5: // IfStatementInfo
        {
            result = _composeIfStatement(std::get<5>(p_statementInfo));
            break;
        }
        case 6: // WhileStatementInfo
        {
            result = _composeWhileStatement(std::get<6>(p_statementInfo));
            break;
        }
        case 7: // ForStatementInfo
        {
            result = _composeForStatement(std::get<7>(p_statementInfo));
            break;
        }
        case 8: // RaiseExceptionStatementInfo
        {
            result = _composeRaiseExceptionStatement(std::get<8>(p_statementInfo));
            break;
        }
        case 9: // CompoundStatementInfo
        {
            result = _composeCompoundStatement(std::get<9>(p_statementInfo));
            break;
        }
        default:
        {
            break;
        }
        }

        return (result);
    }

    std::shared_ptr<Parser::Expression> Parser::_composeExpression(const ExpressionInfo& p_expressionInfo)
    {
        std::shared_ptr<Parser::Expression> result = nullptr;

        switch (p_expressionInfo.index())
        {
        case 0: // LiteralExpressionInfo
        {
            result = _composeLiteralExpression(std::get<LiteralExpressionInfo>(p_expressionInfo));
            break;
        }
        case 1: // VariableExpressionInfo
        {
            result = _composeVariableExpression(std::get<VariableExpressionInfo>(p_expressionInfo));
            break;
        }
        case 2: // BinaryExpressionInfo
        {
            result = _composeBinaryExpression(std::get<BinaryExpressionInfo>(p_expressionInfo));
            break;
        }
        case 3: // UnaryExpressionInfo
        {
            result = _composeUnaryExpression(std::get<UnaryExpressionInfo>(p_expressionInfo));
            break;
        }
        case 4: // PostfixExpressionInfo
        {
            result = _composeUnaryExpression(std::get<PostfixExpressionInfo>(p_expressionInfo));
            break;
        }
        case 5: // FunctionCallExpressionInfo
        {
            result = _composeFunctionCallExpression(std::get<FunctionCallExpressionInfo>(p_expressionInfo));
            break;
        }
        case 6: // MemberAccessExpressionInfo
        {
            result = _composeMemberAccessExpression(std::get<MemberAccessExpressionInfo>(p_expressionInfo));
            break;
        }
        case 7: // ArrayAccessExpressionInfo
        {
            result = _composeArrayAccessExpression(std::get<ArrayAccessExpressionInfo>(p_expressionInfo));
            break;
        }
        case 8: // CastExpressionInfo
        {
            result = _composeCastExpression(std::get<CastExpressionInfo>(p_expressionInfo));
            break;
        }
        default:
            // Handle unexpected cases if necessary
            break;
        }

        return result;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeVariableDeclarationStatement(const VariableDeclarationStatementInfo& p_variableDeclarationStatementInfo)
    {
        auto statement = std::make_shared<VariableDeclarationStatement>();

        // Compose the variable
        statement->variable = _composeVariable(p_variableDeclarationStatementInfo.variable);

        // Compose the initializer if it exists
        if (p_variableDeclarationStatementInfo.initializer)
        {
            statement->initializer = _composeExpression(*p_variableDeclarationStatementInfo.initializer);
        }

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeExpressionStatement(const ExpressionStatementInfo& p_expressionStatementInfo)
    {
        auto statement = std::make_shared<ExpressionStatement>();

        statement->expression = _composeExpression(*p_expressionStatementInfo.expression);

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeAssignmentStatement(const AssignmentStatementInfo& p_assignmentStatementInfo)
    {
        auto statement = std::make_shared<AssignmentStatement>();

        statement->target = _composeExpression(*p_assignmentStatementInfo.target);
        statement->op = _stringToAssignatorOperator(p_assignmentStatementInfo.operatorToken.content);
        statement->value = _composeExpression(*p_assignmentStatementInfo.value);

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeReturnStatement(const ReturnStatementInfo& p_returnStatementInfo)
    {
        auto statement = std::make_shared<ReturnStatement>();

        if (p_returnStatementInfo.expression)
        {
            statement->expression = _composeExpression(*p_returnStatementInfo.expression);
        }

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeDiscardStatement(const DiscardStatementInfo& p_discardStatementInfo)
    {
        return std::make_shared<DiscardStatement>();
    }

    std::shared_ptr<Parser::Statement> Parser::_composeIfStatement(const IfStatementInfo& p_ifStatementInfo)
    {
        auto statement = std::make_shared<IfStatement>();

        for (const auto& branchInfo : p_ifStatementInfo.branches)
        {
            IfStatement::ConditionalBranch branch;

            branch.condition = _composeExpression(*branchInfo.condition);
            branch.body = _composeSymbolBody(branchInfo.body);

            statement->branches.push_back(std::move(branch));
        }

        statement->elseBody = _composeSymbolBody(p_ifStatementInfo.elseBody);

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeWhileStatement(const WhileStatementInfo& p_whileStatementInfo)
    {
        auto statement = std::make_shared<WhileStatement>();

        statement->condition = _composeExpression(*(p_whileStatementInfo.loop.condition));
        statement->body = _composeSymbolBody(p_whileStatementInfo.loop.body);

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeForStatement(const ForStatementInfo& p_forStatementInfo)
    {
        auto statement = std::make_shared<ForStatement>();

        if (p_forStatementInfo.initializer)
        {
            statement->initializer = _composeStatement(*p_forStatementInfo.initializer);
        }

        if (p_forStatementInfo.condition)
        {
            statement->condition = _composeExpression(*p_forStatementInfo.condition);
        }

        if (p_forStatementInfo.increment)
        {
            statement->increment = _composeExpression(*p_forStatementInfo.increment);
        }

        statement->body = _composeSymbolBody(p_forStatementInfo.body);

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeRaiseExceptionStatement(const RaiseExceptionStatementInfo& p_raiseExceptionStatementInfo)
    {
        auto statement = std::make_shared<RaiseExceptionStatement>();

        // Additional processing can be added here if needed

        return statement;
    }

    std::shared_ptr<Parser::Statement> Parser::_composeCompoundStatement(const CompoundStatementInfo& p_compoundStatementInfo)
    {
        auto statement = std::make_shared<CompoundStatement>();

        statement->body = _composeSymbolBody(p_compoundStatementInfo.body);

        return statement;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeLiteralExpression(const LiteralExpressionInfo& p_literalExpressionInfo)
    {
        auto expression = std::make_shared<LiteralExpression>();

        expression->value = p_literalExpressionInfo.value.content;

        return expression;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeVariableExpression(const VariableExpressionInfo& p_variableExpressionInfo)
    {
        auto expression = std::make_shared<VariableExpression>();

        for (const auto& nspace : p_variableExpressionInfo.namespacePath)
        {
            expression->variableName += nspace.content + "_";
        }
        expression->variableName += p_variableExpressionInfo.variableName.content;

        return expression;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeBinaryExpression(const BinaryExpressionInfo& p_binaryExpressionInfo)
    {
        auto expression = std::make_shared<BinaryExpression>();

        expression->left = _composeExpression(*p_binaryExpressionInfo.left);
        expression->op = _stringToOperator(p_binaryExpressionInfo.operatorToken.content);
        expression->right = _composeExpression(*p_binaryExpressionInfo.right);

        return expression;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeUnaryExpression(const UnaryExpressionInfo& p_unaryExpressionInfo)
    {
        auto expression = std::make_shared<UnaryExpression>();

        expression->op = _stringToUnaryOperator(p_unaryExpressionInfo.operatorToken.content);
        expression->operand = _composeExpression(*p_unaryExpressionInfo.operand);

        return expression;
    }
    std::shared_ptr<Parser::Expression> Parser::_composeUnaryExpression(const PostfixExpressionInfo& p_postfixExpressionInfo)
    {
        auto expression = std::make_shared<UnaryExpression>();

        expression->op = _stringToUnaryOperator(p_postfixExpressionInfo.operatorToken.content);
        expression->operand = _composeExpression(*p_postfixExpressionInfo.operand);

        return expression;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeFunctionCallExpression(const FunctionCallExpressionInfo& p_functionCallExpressionInfo)
    {
        auto expression = std::make_shared<FunctionCallExpression>();

        for (const auto& nspace : p_functionCallExpressionInfo.namespacePath)
        {
            expression->functionName += nspace.content + "_";
        }
        expression->functionName += p_functionCallExpressionInfo.functionName.content;

        for (const auto& arg : p_functionCallExpressionInfo.arguments)
        {
            expression->arguments.push_back(_composeExpression(*arg));
        }

        return expression;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeMemberAccessExpression(const MemberAccessExpressionInfo& p_memberAccessExpressionInfo)
    {
        auto expression = std::make_shared<MemberAccessExpression>();

        expression->object = _composeExpression(*p_memberAccessExpressionInfo.object);
        expression->memberName = p_memberAccessExpressionInfo.memberName.content;

        return expression;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeArrayAccessExpression(const ArrayAccessExpressionInfo& p_arrayAccessExpressionInfo)
    {
        auto expression = std::make_shared<ArrayAccessExpression>();

        expression->array = _composeExpression(*p_arrayAccessExpressionInfo.array);
        expression->index = _composeExpression(*p_arrayAccessExpressionInfo.index);

        return expression;
    }

    std::shared_ptr<Parser::Expression> Parser::_composeCastExpression(const CastExpressionInfo& p_castExpressionInfo)
    {
        auto expression = std::make_shared<CastExpression>();

        expression->targetType = _composeExpressionType(p_castExpressionInfo.targetType);

        for (const auto& arg : p_castExpressionInfo.arguments)
        {
            expression->arguments.push_back(_composeExpression(*arg));
        }

        return expression;
    }

}