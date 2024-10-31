#include "parser.hpp"
#include "utils.hpp"

namespace Lumina
{
    std::string Parser::_composeExpression(std::set<VariableImpl>& p_variables, const ExpressionInfo& expr)
    {
        switch (expr.index())
        {
        case 0:
            return (_composeLiteralExpression(p_variables, std::get<0>(expr)));
        case 1:
            return (_composeVariableExpression(p_variables, std::get<1>(expr)));
        case 2:
            return (_composeBinaryExpression(p_variables, std::get<2>(expr)));
        case 3:
            return (_composeUnaryExpression(p_variables, std::get<3>(expr)));
        case 4:
            return (_composePostfixExpression(p_variables, std::get<4>(expr)));
        case 5:
            return (_composeFunctionCallExpression(p_variables, std::get<5>(expr)));
        case 6:
            return (_composeMethodCallExpression(p_variables, std::get<6>(expr)));
        case 7:
            return (_composeMemberAccessExpression(p_variables, std::get<7>(expr)));
        case 8:
            return (_composeArrayAccessExpression(p_variables, std::get<8>(expr)));
        default:
            throw Lumina::TokenBasedError("Unknown expression type.", Token());
        }
        return ("");
    }

    std::string Parser::_composeLiteralExpression(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& expr)
    {
        return expr.value.content;
    }

    std::string Parser::_composeVariableExpression(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& expr)
    {
        std::string name;
        for (const auto& ns : expr.namespacePath)
        {
            name += ns.content + "_";
        }
        name += expr.variableName.content;

        if (p_variables.contains({ {}, name, {} }) == false)
        {
            if (p_variables.contains({ {}, "this", {} }) == false ||
                (p_variables.find({ {}, "this", {} }))->type.attributes.contains({ {}, name, {} }) == false)
            {
                throw (TokenBasedError("No variable named [" + name + "] declared in this scope", expr.variableName));
            }
            return ("this." + name);
        }
        return name;
    }

    std::string Parser::_composeBinaryExpression(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& e)
    {
        ExpressionTypeImpl leftExpressionType = _deduceExpressionType(p_variables, *e.left);
        ExpressionTypeImpl rightExpressionType = _deduceExpressionType(p_variables, *e.right);

        std::string lhs = _composeExpression(p_variables, *e.left);
        std::string rhs = _composeExpression(p_variables, *e.right);
        std::string op = e.operatorToken.content;

        if (leftExpressionType == rightExpressionType && op == "=")
        {
            return (lhs + " = " + rhs);
        }

        if (leftExpressionType.arraySizes.size() != 0)
        {
            throw TokenBasedError("Can't execute operation on array [" + leftExpressionType.type.name + "] object" + DEBUG_INFORMATION, getExpressionToken(*e.left));
        }

        FunctionImpl operatorFunction = _findOperatorFunction(p_variables, leftExpressionType, op, rightExpressionType);

        if (operatorFunction.name.size() == 0)
        {
            throw TokenBasedError("No operator [" + op + "] for type [" + leftExpressionType.type.name + "] with parameter [" + rightExpressionType.type.name + "]" + DEBUG_INFORMATION, e.operatorToken);
        }

        if (operatorFunction.body.code == "")
        {
            return (lhs + " " + op + " " + rhs);
        }

        return operatorFunction.name + "(" + lhs + ", " + rhs + ")";
    }

    std::string Parser::_composeUnaryExpression(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& e)
    {
        ExpressionTypeImpl operandExpressionType = _deduceExpressionType(p_variables, *e.operand);
        std::string operand = _composeExpression(p_variables, *e.operand);
        std::string op = e.operatorToken.content;

        std::string operatorFunctionName = _findUnaryOperatorFunctionName(p_variables, op, operandExpressionType);

        if (!operatorFunctionName.empty())
        {
            return operatorFunctionName + "(" + operand + ")";
        }
        else
        {
            return "(" + op + operand + ")";
        }
    }

    std::string Parser::_composePostfixExpression(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& e)
    {
        ExpressionTypeImpl operandExpressionType = _deduceExpressionType(p_variables, *e.operand);
        std::string operand = _composeExpression(p_variables, *e.operand);
        std::string op = e.operatorToken.content;

        std::string operatorFunctionName = _findPostfixOperatorFunctionName(p_variables, op, operandExpressionType);

        if (!operatorFunctionName.empty())
        {
            return operatorFunctionName + "(" + operand + ")";
        }
        else
        {
            return "(" + operand + op + ")";
        }
    }

    std::string Parser::_composeFunctionCallExpression(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& e)
    {
        std::string name;
        for (const auto& ns : e.namespacePath)
        {
            name += ns.content + "::";
        }
        name += e.functionName.content;

        std::vector<ParameterImpl> parameterTypes;

        std::string args;
        for (size_t i = 0; i < e.arguments.size(); ++i)
        {
            ExpressionTypeImpl expressionType = _deduceExpressionType(p_variables, *e.arguments[i]);

            parameterTypes.push_back({
                    .type = expressionType.type,
                    .isReference = false,
                    .name = "",
                    .arraySizes = expressionType.arraySizes
                });

            args += _composeExpression(p_variables, *e.arguments[i]);
            if (i < e.arguments.size() - 1)
            {
                args += ", ";
            }
        }

        FunctionImpl expectedFunction = {
            .isPrototype = false,
            .returnType = {},
            .name = name,
            .parameters = parameterTypes,
            .body = {}
        };

        if (_availibleFunctions.contains(expectedFunction) == false)
        {
            std::string parameterString = "";

            for (const auto& parameter : parameterTypes)
            {
                if (parameterString.size() != 0)
                    parameterString += ", ";

                parameterString += parameter.type.name;
                for (const auto& size : parameter.arraySizes)
                {
                    parameterString += "[" + std::to_string(size) + "]";
                }
            }

            throw TokenBasedError("No function [" + e.functionName.content + "] detected with following parameters [" + parameterString + "]" + DEBUG_INFORMATION, e.functionName);
        }

        return "(" + name + "(" + args + "))";
    }

    std::string Parser::_composeMethodCallExpression(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& e)
    {
        std::string objectExpression = _composeExpression(p_variables, *e.object);

        ExpressionTypeImpl objectType = _deduceExpressionType(p_variables, *e.object);
        std::string methodName = objectType.type.name + "_" + e.name.content;

        std::string parameters = objectExpression;

        for (size_t i = 0; i < e.arguments.size(); ++i)
        {
            std::string argExpression = _composeExpression(p_variables, *e.arguments[i]);
            parameters += ", " + argExpression;
        }

        return "(" + methodName + "(" + parameters + "))";
    }

    std::string Parser::_composeMemberAccessExpression(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& e)
    {
        std::string object = _composeExpression(p_variables, *e.object);
        std::string member = e.memberName.content;
        return object + "." + member;
    }

    std::string Parser::_composeArrayAccessExpression(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& e)
    {
        std::string arrayResult = _composeExpression(p_variables, *e.array);
        std::string index = _composeExpression(p_variables, *e.index);
        return arrayResult + "[" + index + "]";
    }
}
