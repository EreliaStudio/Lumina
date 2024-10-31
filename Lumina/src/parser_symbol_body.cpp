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
        VariableImpl var = _composeVariable(stmt.variable);
        std::string code = var.type.name + " " + var.name;

        if (stmt.initializer)
        {
            code += " = " + _composeExpression(p_variables, *stmt.initializer);
        }

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

        std::string operatorFunctionName = _findOperatorFunctionName(p_variables, targetExpressionType, op, valueExpressionType, true);
        if (!operatorFunctionName.empty())
        {
            return operatorFunctionName + "(" + target + ", " + value + ")";
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
        searchFunction.parameters.push_back({
                .type = rhs.type,
                .arraySizes = rhs.arraySizes
            });

        auto funcIt = _availibleFunctions.find(searchFunction);
        if (funcIt != _availibleFunctions.end())
        {
            return (*funcIt);
        }

        return FunctionImpl();
    }

    // Helper functions to find operator function names
    std::string Parser::_findOperatorFunctionName(std::set<VariableImpl>& p_variables, const ExpressionTypeImpl& lhs, const std::string& op, const ExpressionTypeImpl& rhs, bool isAssignment)
    {
        return _findOperatorFunction(p_variables, lhs, op, rhs, isAssignment).name;
    }

    std::string Parser::_findUnaryOperatorFunctionName(std::set<VariableImpl>& p_variables, const std::string& op, const ExpressionTypeImpl& operand)
    {
        // Get the operator name from the operator token
        auto it = _operatorNames.find(op);
        if (it == _operatorNames.end())
            return "";

        std::string operatorName = it->second;

        // Construct the function name
        std::string functionName = operand.type.name + "_Operator" + operatorName;

        // Search for the function in _availibleFunctions
        FunctionImpl searchFunction;
        searchFunction.name = functionName;

        auto funcIt = _availibleFunctions.find(searchFunction);
        if (funcIt != _availibleFunctions.end())
        {
            // Check if the function has a non-empty body
            if (!funcIt->body.code.empty())
            {
                return functionName;
            }
        }

        return "";
    }

    std::string Parser::_findPostfixOperatorFunctionName(std::set<VariableImpl>& p_variables, const std::string& op, const ExpressionTypeImpl& operand)
    {
        return _findUnaryOperatorFunctionName(p_variables, op, operand);
    }
}
