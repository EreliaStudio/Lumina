// source_symbol_body.cpp

#include "parser.hpp"

namespace Lumina
{
    SymbolBodyImpl Parser::_composeSymbolBody(std::set<VariableImpl>& p_variables, const SymbolBodyInfo& p_symbolBodyInfo)
    {
        SymbolBodyImpl result;

        std::string code;

        for (const auto& statement : p_symbolBodyInfo.statements)
        {
            try
            {
                std::string statementCode = _composeStatement(p_variables, statement);
                code += statementCode + "\n";
            }
            catch (const Lumina::TokenBasedError& e)
            {
                _product.errors.push_back(e);
            }
        }

        result.code = code;

        return result;
    }

    std::string Parser::_composeStatement(std::set<VariableImpl>& p_variables, const StatementInfo& p_statementInfo)
    {
        switch (p_statementInfo.index())
        {
        case 0:
            return (_composeVariableDeclaration(p_variables, std::get<0>(p_statementInfo)) + ";");
        case 1:
            return (_composeExpressionStatement(p_variables, std::get<1>(p_statementInfo)) + ";");
        case 2:
            return (_composeAssignmentStatement(p_variables, std::get<2>(p_statementInfo)) + ";");
        case 3:
            return (_composeReturnStatement(p_variables, std::get<3>(p_statementInfo)) + ";");
        case 4:
            return ("discard;");
        case 5:
            return (_composeIfStatement(p_variables, std::get<5>(p_statementInfo)) + ";");
        case 6:
            return (_composeWhileStatement(p_variables, std::get<6>(p_statementInfo)) + ";");
        case 7:
            return (_composeForStatement(p_variables, std::get<7>(p_statementInfo)) + ";");
        case 8:
            return (_composeRaiseExceptionStatement(p_variables, std::get<8>(p_statementInfo)) + ";");
        case 9:
            return ("{\n" + _composeSymbolBody(p_variables, std::get<9>(p_statementInfo).body).code + "}\n");
        default:
            throw Lumina::TokenBasedError("Unknown statement type.", Token());
        }

        return ("");
    }

    std::string Parser::_composeVariableDeclaration(std::set<VariableImpl>& p_variables, const VariableDeclarationStatementInfo& stmt)
    {
        // Compose the variable (extract type and name)
        VariableImpl var = _composeVariable(stmt.variable);

        // Start composing the declaration code
        std::string code = var.type.name + " " + var.name;

        if (stmt.initializer)
        {
            // Deduce the type of the variable being declared
            ExpressionTypeImpl variableType = { var.type, var.arraySizes };

            // Deduce the type of the initializer expression
            ExpressionTypeImpl initializerType = _deduceExpressionType(p_variables, *stmt.initializer);

            // Check for type conversion
            bool conversionAvailable = false;

            if (variableType.type == initializerType.type && variableType.arraySizes == initializerType.arraySizes)
            {
                // Types are the same, no conversion needed
                conversionAvailable = true;
            }
            else
            {
                // Check if a conversion is available from initializerType to variableType
                auto convIt = _convertionTable.find(initializerType.type);
                if (convIt != _convertionTable.end() && convIt->second.count(variableType.type) > 0)
                {
                    conversionAvailable = true;
                }
            }

            // Compose the initializer expression
            std::string initializerCode = _composeExpression(p_variables, *stmt.initializer);

            if (conversionAvailable)
            {
                // Apply conversion if types are different
                if (initializerType.type != variableType.type)
                {
                    initializerCode = "(" + variableType.type.name + ")(" + initializerCode + ")";
                }
                code += " = " + initializerCode;
            }
            else
            {
                // No conversion available, attempt to find operator overload
                std::string op = "=";

                FunctionImpl operatorFunction = _findOperatorFunction(p_variables, variableType, op, initializerType, true);

                if (operatorFunction.name.size() != 0)
                {
                    // Use the operator function to perform the assignment
                    code += " = " + operatorFunction.name + "(" + var.name + ", " + initializerCode + ")";
                }
                else
                {
                    // No operator function or conversion available, throw an error
                    Token errorToken = getExpressionToken(*stmt.initializer);
                    throw TokenBasedError(
                        "Cannot assign type [" + initializerType.type.name + "] to variable of type [" + variableType.type.name + "]",
                        errorToken
                    );
                }
            }
        }

        // Insert the variable into the set of variables
        p_variables.insert(var);

        return code;
    }

    std::string Parser::_composeExpressionStatement(std::set<VariableImpl>& p_variables, const ExpressionStatementInfo& stmt)
    {
        return _composeExpression(p_variables, *stmt.expression);
    }

    std::string Parser::_composeAssignmentStatement(std::set<VariableImpl>& p_variables, const AssignmentStatementInfo& stmt)
    {
        ExpressionTypeImpl targetExpressionType = _deduceExpressionType(p_variables, *stmt.target);
        ExpressionTypeImpl valueExpressionType = _deduceExpressionType(p_variables, *stmt.value);
        std::string target = _composeExpression(p_variables, *stmt.target);
        std::string value = _composeExpression(p_variables, *stmt.value);
        std::string op = stmt.operatorToken.content;

        FunctionImpl operatorFunction = _findOperatorFunction(p_variables, targetExpressionType, op, valueExpressionType, true);
        if (!operatorFunction.name.empty())
        {
            return operatorFunction.name + "(" + target + ", " + value + ")";
        }
        else
        {
            return target + " " + op + " " + value;
        }
    }

    std::string Parser::_composeReturnStatement(std::set<VariableImpl>& p_variables, const ReturnStatementInfo& stmt)
    {
        if (stmt.expression)
        {
            return "return " + _composeExpression(p_variables, *stmt.expression);
        }
        else
        {
            return "return";
        }
    }

    std::string Parser::_composeRaiseExceptionStatement(std::set<VariableImpl>& p_variables, const RaiseExceptionStatementInfo& stmt)
    {
        return _composeFunctionCallExpression(p_variables, *stmt.functionCall);
    }

    std::string Parser::_composeIfStatement(std::set<VariableImpl>& p_variables, const IfStatementInfo& stmt)
    {
        std::string code;
        bool first = true;
        for (const auto& branch : stmt.branches)
        {
            if (first)
            {
                code += "if (" + _composeExpression(p_variables, *branch.condition) + ")\n";
            }
            else
            {
                code += "else if (" + _composeExpression(p_variables, *branch.condition) + ")\n";
            }
            code += "{\n" + _composeSymbolBody(p_variables, branch.body).code + "}\n";
            first = false;
        }

        if (!stmt.elseBody.statements.empty())
        {
            code += "else\n{\n" + _composeSymbolBody(p_variables, stmt.elseBody).code + "}\n";
        }

        return code;
    }

    std::string Parser::_composeWhileStatement(std::set<VariableImpl>& p_variables, const WhileStatementInfo& stmt)
    {
        std::string condition = _composeExpression(p_variables, *stmt.loop.condition);
        std::string body = _composeSymbolBody(p_variables, stmt.loop.body).code;

        std::string code = "while (" + condition + ")\n{\n" + body + "}\n";

        return code;
    }

    std::string Parser::_composeForStatement(std::set<VariableImpl>& p_variables, const ForStatementInfo& stmt)
    {
        std::string init = stmt.initializer ? _composeStatement(p_variables, *stmt.initializer) : "";
        std::string condition = stmt.condition ? _composeExpression(p_variables, *stmt.condition) : "";
        std::string increment = stmt.increment ? _composeExpression(p_variables, *stmt.increment) : "";
        std::string body = _composeSymbolBody(p_variables, stmt.body).code;

        if (!init.empty() && init.back() == ';')
            init.pop_back();

        std::string code = "for (" + init + "; " + condition + "; " + increment + ")\n{\n" + body + "}\n";

        return code;
    }
    
    FunctionImpl Parser::_findOperatorFunction(std::set<VariableImpl>& p_variables, const ExpressionTypeImpl& lhs, const std::string& op, const ExpressionTypeImpl& rhs, bool isAssignment)
    {
        std::string operatorName = _operatorNames.find(op)->second;

        std::string functionName = lhs.type.name + "_Operator" + operatorName;

        FunctionImpl searchFunction;
        searchFunction.name = functionName;
        searchFunction.parameters.push_back({
                .type = lhs.type,
                .arraySizes = lhs.arraySizes
            });

        std::cout << "Looking for function " << functionName << " accepting type " << rhs.type.name << std::endl;

        for (const auto& convertedType : _convertionTable[rhs.type])
        {
            FunctionImpl toTest = searchFunction;

            toTest.parameters.push_back({
                    .type = convertedType,
                    .arraySizes = rhs.arraySizes
                });

            std::cout << "Searching for function " << toTest.name << "(";
            for (const auto& parameter : toTest.parameters)
            {
                std::cout << "[" << parameter.type.name << "]";
            }
            std::cout << ")" << std::endl;

            auto funcIt = _availibleFunctions.find(toTest);
            if (funcIt != _availibleFunctions.end())
            {
                return (*funcIt);
            }
        }

        return FunctionImpl();
    }

    FunctionImpl Parser::_findUnaryOperatorFunction(std::set<VariableImpl>& p_variables, const std::string& op, const ExpressionTypeImpl& operand)
    {
        std::string operatorName = _operatorNames.find(op)->second;

        std::string functionName = operand.type.name + "_Operator" + operatorName;

        FunctionImpl searchFunction;
        searchFunction.name = functionName;
        searchFunction.parameters.push_back({
                .type = operand.type,
                .arraySizes = operand.arraySizes
            });

        auto funcIt = _availibleFunctions.find(searchFunction);
        if (funcIt != _availibleFunctions.end())
        {
            return (*funcIt);
        }

        return FunctionImpl();
    }
    
    FunctionImpl Parser::_findPostfixOperatorFunction(std::set<VariableImpl>& p_variables, const std::string& op, const ExpressionTypeImpl& operand)
    {
        return _findUnaryOperatorFunction(p_variables, op, operand);
    }
}
