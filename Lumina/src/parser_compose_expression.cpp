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
            throw TokenBasedError("No operator [" + op + "] for type [" + leftExpressionType.type.name + "] with parameter [" + rightExpressionType.type.name + "]" + DEBUG_INFORMATION, getExpressionToken(e));
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

        if (operandExpressionType.arraySizes.size() != 0)
        {
            throw TokenBasedError("Can't execute operation on array [" + operandExpressionType.type.name + "] object" + DEBUG_INFORMATION, getExpressionToken(*e.operand));
        }

        FunctionImpl operatorFunction = _findUnaryOperatorFunction(p_variables, op, operandExpressionType);

        if (operatorFunction.name.size() == 0)
        {
            throw TokenBasedError("No operator [" + op + "] for type [" + operandExpressionType.type.name + "]" + DEBUG_INFORMATION, getExpressionToken(e));
        }

        if (operatorFunction.body.code == "")
        {
            return (op + operand);
        }

        return operatorFunction.name + "(" + operand + ")";
    }

    std::string Parser::_composePostfixExpression(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& e)
    {
        ExpressionTypeImpl operandExpressionType = _deduceExpressionType(p_variables, *e.operand);

        std::string operand = _composeExpression(p_variables, *e.operand);
        std::string op = e.operatorToken.content;

        if (operandExpressionType.arraySizes.size() != 0)
        {
            throw TokenBasedError("Can't execute operation on array [" + operandExpressionType.type.name + "] object" + DEBUG_INFORMATION, getExpressionToken(*e.operand));
        }

        FunctionImpl operatorFunction = _findPostfixOperatorFunction(p_variables, op, operandExpressionType);

        if (operatorFunction.name.size() == 0)
        {
            throw TokenBasedError("No operator [" + op + "] for type [" + operandExpressionType.type.name + "]" + DEBUG_INFORMATION, getExpressionToken(e));
        }

        if (operatorFunction.body.code == "")
        {
            return (operand + op);
        }

        return operatorFunction.name + "(" + operand + ")";
    }

    std::string Parser::_composeFunctionCallExpression(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& e)
    {
        std::string name;
        for (const auto& ns : e.namespacePath)
        {
            name += ns.content + "_";
        }
        name += e.functionName.content;

        std::vector<ExpressionTypeImpl> argumentTypes;
        std::vector<std::string> argumentCodes;
        for (const auto& argExpr : e.arguments)
        {
            ExpressionTypeImpl exprType = _deduceExpressionType(p_variables, *argExpr);
            argumentTypes.push_back(exprType);

            std::string argCode = _composeExpression(p_variables, *argExpr);
            argumentCodes.push_back(argCode);
        }

        FunctionImpl* matchingFunction = _findFunctionWithConversions(name, argumentTypes);

        if (!matchingFunction)
        {
            std::string parameterString;
            for (size_t i = 0; i < argumentTypes.size(); ++i)
            {
                if (i > 0) parameterString += ", ";
                parameterString += argumentTypes[i].type.name;
                for (const auto& size : argumentTypes[i].arraySizes)
                {
                    parameterString += "[" + std::to_string(size) + "]";
                }
            }

            throw TokenBasedError(
                "No function [" + e.functionName.content + "] detected with parameters [" + parameterString + "]",
                e.functionName
            );
        }

        std::string args;
        for (size_t i = 0; i < argumentCodes.size(); ++i)
        {
            const ExpressionTypeImpl& argType = argumentTypes[i];
            const ParameterImpl& param = matchingFunction->parameters[i];

            std::string argCode = argumentCodes[i];

            if (argType.type.name != param.type.name)
            {
                argCode = "(" + param.type.name + ")(" + argCode + ")";
            }

            if (i > 0) args += ", ";
            args += argCode;
        }

        return name + "(" + args + ")";
    }

    FunctionImpl* Parser::_findFunctionWithConversions(const std::string& name, const std::vector<ExpressionTypeImpl>& argumentTypes)
    {
        // Create a FunctionImpl object representing the function we are searching for
        FunctionImpl searchFunction;
        searchFunction.name = name;
        searchFunction.parameters.resize(argumentTypes.size());

        for (size_t i = 0; i < argumentTypes.size(); ++i)
        {
            searchFunction.parameters[i].type = argumentTypes[i].type;
            searchFunction.parameters[i].arraySizes = argumentTypes[i].arraySizes;
            searchFunction.parameters[i].isReference = false; // Assuming isReference is not relevant here
            searchFunction.parameters[i].name = ""; // Name is not relevant for matching
        }

        // Assuming _availibleFunctions is a std::set<FunctionImpl>
        auto it = _availibleFunctions.find(searchFunction);
        if (it != _availibleFunctions.end())
        {
            // Exact match found
            return const_cast<FunctionImpl*>(&(*it));
        }

        // If no exact match, attempt to find a function with type conversions
        FunctionImpl* bestMatch = nullptr;
        int lowestConversionCost = INT_MAX;
        bool ambiguous = false;

        for (const auto& func : _availibleFunctions)
        {
            if (func.name != name)
                continue; // Function name does not match

            if (func.parameters.size() != argumentTypes.size())
                continue; // Parameter count does not match

            int totalConversionCost = 0;
            bool match = true;

            for (size_t i = 0; i < argumentTypes.size(); ++i)
            {
                const ExpressionTypeImpl& argType = argumentTypes[i];
                const ParameterImpl& param = func.parameters[i];

                if (argType.type == param.type && argType.arraySizes == param.arraySizes)
                {
                    // Exact type match, no conversion needed
                    continue;
                }
                else
                {
                    // Check if a conversion is available from argType to param.type
                    auto convIt = _convertionTable.find(argType.type);
                    if (convIt != _convertionTable.end() && convIt->second.count(param.type) > 0)
                    {
                        // Conversion is allowed; increment conversion cost
                        totalConversionCost += 1;
                    }
                    else
                    {
                        // No conversion available; this function is not a match
                        match = false;
                        break;
                    }
                }
            }

            if (match)
            {
                if (totalConversionCost < lowestConversionCost)
                {
                    bestMatch = const_cast<FunctionImpl*>(&func);
                    lowestConversionCost = totalConversionCost;
                    ambiguous = false;
                }
                else if (totalConversionCost == lowestConversionCost)
                {
                    // Ambiguous match found
                    ambiguous = true;
                }
            }
        }

        if (ambiguous)
        {
            // Handle ambiguity (could throw an error or return nullptr)
            return nullptr;
        }

        return bestMatch; // Returns the best matching function or nullptr if none found
    }
    std::string Parser::_composeMethodCallExpression(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& e)
    {
        std::string objectExpression = _composeExpression(p_variables, *e.object);
        ExpressionTypeImpl objectType = _deduceExpressionType(p_variables, *e.object);

        std::string methodName = objectType.type.name + "_" + e.name.content;

        std::vector<ExpressionTypeImpl> argumentTypes;
        std::vector<std::string> argumentExpressions;

        argumentTypes.push_back(objectType);
        argumentExpressions.push_back(objectExpression);

        for (const auto& argExpr : e.arguments)
        {
            ExpressionTypeImpl argType = _deduceExpressionType(p_variables, *argExpr);
            argumentTypes.push_back(argType);

            std::string argExpression = _composeExpression(p_variables, *argExpr);
            argumentExpressions.push_back(argExpression);
        }

        FunctionImpl* matchingMethod = _findFunctionWithConversions(methodName, argumentTypes);

        if (!matchingMethod)
        {
            std::string parameterString;
            for (size_t i = 1; i < argumentTypes.size(); ++i)
            {
                if (!parameterString.empty())
                    parameterString += ", ";

                parameterString += argumentTypes[i].type.name;
                for (const auto& size : argumentTypes[i].arraySizes)
                {
                    parameterString += "[" + std::to_string(size) + "]";
                }
            }

            Token errorToken = getExpressionToken(*e.object) + e.name + getExpressionToken(*e.arguments.front());

            throw TokenBasedError(
                "No method [" + e.name.content + "] for type [" + objectType.type.name + "] with parameters [" + parameterString + "]",
                errorToken
            );
        }

        std::string parameters;
        for (size_t i = 0; i < argumentExpressions.size(); ++i)
        {
            const ExpressionTypeImpl& argType = argumentTypes[i];
            const ParameterImpl& param = matchingMethod->parameters[i];
            std::string argExpression = argumentExpressions[i];

            if (argType.type != param.type)
            {
                if (_convertionTable[argType.type].contains(param.type) == true)
                {
                    argExpression = "(" + param.type.name + ")(" + argExpression + ")";
                }
                else
                {
                    Token errorToken = getExpressionToken(*e.object);
                    throw TokenBasedError(
                        "Cannot convert type [" + argType.type.name + "] to [" + param.type.name + "] for parameter " + std::to_string(i),
                        errorToken
                    );
                }
            }

            if (i > 0)
                parameters += ", ";

            parameters += argExpression;
        }

        return methodName + "(" + parameters + ")";
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
