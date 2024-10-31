// source_symbol_body.cpp

#include "parser.hpp"
#include "utils.hpp"

namespace Lumina
{
    Token Parser::getExpressionToken(const ExpressionInfo& expr) {
        return std::visit([&](auto&& arg) -> Token {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, LiteralExpressionInfo>) {
                return arg.value;
            }
            else if constexpr (std::is_same_v<T, VariableExpressionInfo>) {
                Token result;
                for (const auto& nsToken : arg.namespacePath) {
                    result = result + nsToken;
                }
                result = result + arg.variableName;
                return result;
            }
            else if constexpr (std::is_same_v<T, BinaryExpressionInfo>) {
                Token leftToken = getExpressionToken(*arg.left);
                Token operatorToken = arg.operatorToken;
                Token rightToken = getExpressionToken(*arg.right);
                return leftToken + operatorToken + rightToken;
            }
            else if constexpr (std::is_same_v<T, UnaryExpressionInfo>) {
                Token operatorToken = arg.operatorToken;
                Token operandToken = getExpressionToken(*arg.operand);
                return operatorToken + operandToken;
            }
            else if constexpr (std::is_same_v<T, PostfixExpressionInfo>) {
                Token operandToken = getExpressionToken(*arg.operand);
                Token operatorToken = arg.operatorToken;
                return operandToken + operatorToken;
            }
            else if constexpr (std::is_same_v<T, FunctionCallExpressionInfo>) {
                Token result;
                for (const auto& nsToken : arg.namespacePath) {
                    result = result + nsToken;
                }
                result = result + arg.functionName;

                for (size_t i = 0; i < arg.arguments.size(); ++i) {
                    Token argToken = getExpressionToken(*arg.arguments[i]);
                    result = result + argToken;
                }
                return result;
            }
            else if constexpr (std::is_same_v<T, MethodCallExpressionInfo>) {
                Token objectToken = getExpressionToken(*arg.object);
                Token result = objectToken + arg.name;
                for (size_t i = 0; i < arg.arguments.size(); ++i)
                {
                    Token argToken = getExpressionToken(*arg.arguments[i]);
                    result = result + argToken;
                }
                return result;
            }
            else if constexpr (std::is_same_v<T, MemberAccessExpressionInfo>) {
                Token objectToken = getExpressionToken(*arg.object);
                return objectToken + arg.memberName;
            }
            else if constexpr (std::is_same_v<T, ArrayAccessExpressionInfo>) {
                Token arrayToken = getExpressionToken(*arg.array);
                Token indexToken = getExpressionToken(*arg.index);
                indexToken.content += "]";
                return arrayToken + indexToken;
            }
            }, expr);
    }

    ExpressionTypeImpl Parser::_deduceLiteralExpressionType(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& expr)
    {
        const Token& token = expr.value;

        if (token.type == Token::Type::Number)
        {
            const std::string& content = token.content;
            std::string numberContent = content;
            bool isUnsigned = false;
            bool isFloat = false;
            bool isNegative = false;

            // Check for 'u' or 'U' suffix for unsigned integers
            if (!numberContent.empty() && (numberContent.back() == 'u' || numberContent.back() == 'U'))
            {
                isUnsigned = true;
                numberContent.pop_back(); // Remove the 'u' suffix
            }

            // Check for 'f' or 'F' suffix for floats
            if (!numberContent.empty() && (numberContent.back() == 'f' || numberContent.back() == 'F'))
            {
                isFloat = true;
                numberContent.pop_back(); // Remove the 'f' suffix
            }

            // Check for negative sign at the beginning
            if (!numberContent.empty() && numberContent.front() == '-')
            {
                isNegative = true;
                numberContent.erase(0, 1); // Remove the '-' sign
            }

            // Check for decimal point or exponent for floats
            if (numberContent.find('.') != std::string::npos || numberContent.find('e') != std::string::npos || numberContent.find('E') != std::string::npos)
            {
                isFloat = true;
            }

            // Now attempt to parse the number
            try
            {
                if (isFloat)
                {
                    // Attempt to parse as float
                    std::stof(numberContent); // Throws exception if invalid
                    return { _getType("float"), {} };
                }
                else
                {
                    if (isUnsigned)
                    {
                        if (isNegative)
                        {
                            throw Lumina::TokenBasedError("Unsigned integer cannot be negative.", token);
                        }
                        std::stoul(numberContent); // Attempt to parse as unsigned long
                        return { _getType("uint"), {} };
                    }
                    else
                    {
                        std::stol(numberContent); // Attempt to parse as long (signed integer)
                        return { _getType("int"), {} };
                    }
                }
            }
            catch (const std::exception&)
            {
                throw Lumina::TokenBasedError("Invalid numeric literal.", token);
            }
        }
        else
        {
            throw Lumina::TokenBasedError("Unknown literal type.", token);
        }
    }

    ExpressionTypeImpl Parser::_deduceVariableExpressionType(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& expr)
    {
        std::string name;
        for (const auto& ns : expr.namespacePath)
        {
            name += ns.content + "_";
        }
        name += expr.variableName.content;

        auto it = p_variables.find({ {}, name, {} });
        if (it != p_variables.end())
        {
            return { it->type, it->arraySizes };
        }
        else
        {
            auto thisIt = p_variables.find({ {}, "this", {} });
            if (thisIt != p_variables.end())
            {
                auto attrIt = thisIt->type.attributes.find({ {}, name, {} });
                if (attrIt != thisIt->type.attributes.end())
                {
                    return { attrIt->type, attrIt->arraySizes };
                }
            }
            throw Lumina::TokenBasedError("No variable named [" + name + "] declared in this scope" + DEBUG_INFORMATION, expr.variableName);
        }
    }

    ExpressionTypeImpl Parser::_deduceBinaryExpressionType(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& e)
    {
        ExpressionTypeImpl lhsType = _deduceExpressionType(p_variables, *e.left);
        ExpressionTypeImpl rhsType = _deduceExpressionType(p_variables, *e.right);
        std::string op = e.operatorToken.content;

        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        {
            return { _getType("bool"), {} };
        }
        else if (op == "&&" || op == "||")
        {
            if (lhsType.type.name != "bool" || rhsType.type.name != "bool")
            {
                throw Lumina::TokenBasedError("Logical operators require boolean operands", e.operatorToken);
            }
            return { _getType("bool"), {} };
        }
        else
        {
            FunctionImpl operatorFunction = _findOperatorFunction(p_variables, lhsType, op, rhsType);

            if (operatorFunction.name.size() != 0)
            {
                // Operator function exists, return its return type
                return operatorFunction.returnType;
            }
            else
            {
                // No operator function found, throw an error
                // Use getExpressionToken to get a combined token for the error message
                Token leftToken = getExpressionToken(*e.left);
                Token operatorToken = e.operatorToken;
                Token rightToken = getExpressionToken(*e.right);

                Token errorToken = leftToken + operatorToken + rightToken;

                throw TokenBasedError(
                    "No operator [" + op + "] for type [" + lhsType.type.name + "] with parameters [" + rhsType.type.name + "]" + DEBUG_INFORMATION,
                    errorToken
                );
            }
        }
    }

    ExpressionTypeImpl Parser::_deduceUnaryExpressionType(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& e)
    {
        ExpressionTypeImpl operandType = _deduceExpressionType(p_variables, *e.operand);
        return operandType;
    }

    ExpressionTypeImpl Parser::_deducePostfixExpressionType(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& e)
    {
        ExpressionTypeImpl operandType = _deduceExpressionType(p_variables, *e.operand);
        return operandType;
    }

    ExpressionTypeImpl Parser::_deduceFunctionCallExpressionType(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& e)
    {
        std::string name;
        for (const auto& ns : e.namespacePath)
        {
            name += ns.content + "_";
        }
        name += e.functionName.content;

        std::vector<ExpressionTypeImpl> argTypes;
        for (const auto& argExpr : e.arguments)
        {
            argTypes.push_back(_deduceExpressionType(p_variables, *argExpr));
        }

        for (const auto& func : _availibleFunctions)
        {
            if (func.name == name && func.parameters.size() == argTypes.size())
            {
                bool match = true;
                for (size_t i = 0; i < argTypes.size(); ++i)
                {
                    if (_convertionTable[func.parameters[i].type].contains(argTypes[i].type) == false)
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                {
                    return func.returnType;
                }
            }
        }

        throw Lumina::TokenBasedError("Function not found or argument types do not match" + DEBUG_INFORMATION, e.functionName);
    }

    ExpressionTypeImpl Parser::_deduceMethodCallExpressionType(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& e)
    {
        ExpressionTypeImpl objectType = _deduceExpressionType(p_variables, *e.object);
        std::string methodName = e.name.content;

        std::vector<ExpressionTypeImpl> argTypes;
        for (const auto& argExpr : e.arguments)
        {
            argTypes.push_back(_deduceExpressionType(p_variables, *argExpr));
        }

        std::string fullMethodName = objectType.type.name + "_" + methodName;

        for (const auto& func : _availibleFunctions)
        {
            if (func.name == fullMethodName && func.parameters.size() == argTypes.size() + 1)
            {
                if (func.parameters[0].type.name != objectType.type.name)
                    continue;

                bool match = true;
                for (size_t i = 0; i < argTypes.size(); ++i)
                {
                    if (func.parameters[i + 1].type.name != argTypes[i].type.name)
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                {
                    return func.returnType;
                }
            }
        }

        throw Lumina::TokenBasedError("Method not found or argument types do not match", e.name);
    }

    ExpressionTypeImpl Parser::_deduceMemberAccessExpressionType(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& e)
    {
        ExpressionTypeImpl objectType = _deduceExpressionType(p_variables, *e.object);
        std::string memberName = e.memberName.content;

        auto it = objectType.type.attributes.find({ {}, memberName, {} });
        if (it != objectType.type.attributes.end())
        {
            return { it->type, it->arraySizes };
        }
        else
        {
            throw Lumina::TokenBasedError("Member [" + memberName + "] not found in type [" + objectType.type.name + "]", e.memberName);
        }
    }

    ExpressionTypeImpl Parser::_deduceArrayAccessExpressionType(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& e)
    {
        ExpressionTypeImpl arrayType = _deduceExpressionType(p_variables, *e.array);
        ExpressionTypeImpl indexType = _deduceExpressionType(p_variables, *e.index);

        const MemberAccessExpressionInfo* arrayExpr = std::get_if<MemberAccessExpressionInfo>(&*e.array);
        const LiteralExpressionInfo* indexExpr = std::get_if<LiteralExpressionInfo>(&*e.index);


        if (indexType.type.name != "int" && indexType.type.name != "uint")
        {
            throw Lumina::TokenBasedError("Array index must be of type int or uint", indexExpr->value);
        }

        if (!arrayType.arraySizes.empty())
        {
            std::vector<size_t> newArraySizes = arrayType.arraySizes;
            newArraySizes.erase(newArraySizes.begin());
            return { arrayType.type, newArraySizes };
        }
        else
        {
            throw Lumina::TokenBasedError("Cannot index a non-array type", arrayExpr->memberName);
        }
    }

    ExpressionTypeImpl Parser::_deduceExpressionType(std::set<VariableImpl>& p_variables, const ExpressionInfo& expr)
    {
        switch (expr.index())
        {
        case 0:
            return _deduceLiteralExpressionType(p_variables, std::get<0>(expr));
        case 1:
            return _deduceVariableExpressionType(p_variables, std::get<1>(expr));
        case 2:
            return _deduceBinaryExpressionType(p_variables, std::get<2>(expr));
        case 3:
            return _deduceUnaryExpressionType(p_variables, std::get<3>(expr));
        case 4:
            return _deducePostfixExpressionType(p_variables, std::get<4>(expr));
        case 5:
            return _deduceFunctionCallExpressionType(p_variables, std::get<5>(expr));
        case 6:
            return _deduceMethodCallExpressionType(p_variables, std::get<6>(expr));
        case 7:
            return _deduceMemberAccessExpressionType(p_variables, std::get<7>(expr));
        case 8:
            return _deduceArrayAccessExpressionType(p_variables, std::get<8>(expr));
        default:
            throw Lumina::TokenBasedError("Unknown expression type.", Token());
        }
    }
}
