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
        std::string target = _composeExpression(p_variables, *stmt.target);
        std::string value = _composeExpression(p_variables, *stmt.value);
        std::string op = stmt.operatorToken.content;

        std::string operatorFunctionName = _findOperatorFunctionName(p_variables, target, op, value, true);
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
            return (_composeMemberAccessExpression(p_variables, std::get<6>(expr)));
        case 7:
            return (_composeArrayAccessExpression(p_variables, std::get<7>(expr)));
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
        std::string lhs = _composeExpression(p_variables, *e.left);
        std::string rhs = _composeExpression(p_variables, *e.right);
        std::string op = e.operatorToken.content;

        std::string operatorFunctionName = _findOperatorFunctionName(p_variables, lhs, op, rhs);

        if (!operatorFunctionName.empty())
        {
            return operatorFunctionName + "(" + lhs + ", " + rhs + ")";
        }
        else
        {
            return "(" + lhs + " " + op + " " + rhs + ")";
        }
    }

    std::string Parser::_composeUnaryExpression(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& e)
    {
        std::string operand = _composeExpression(p_variables, *e.operand);
        std::string op = e.operatorToken.content;

        std::string operatorFunctionName = _findUnaryOperatorFunctionName(p_variables, op, operand);

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
        std::string operand = _composeExpression(p_variables, *e.operand);
        std::string op = e.operatorToken.content;

        std::string operatorFunctionName = _findPostfixOperatorFunctionName(p_variables, op, operand);

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

        std::string args;
        for (size_t i = 0; i < e.arguments.size(); ++i)
        {
            args += _composeExpression(p_variables, *e.arguments[i]);
            if (i < e.arguments.size() - 1)
            {
                args += ", ";
            }
        }

        return name + "(" + args + ")";
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

    // Helper functions to find operator function names
    std::string Parser::_findOperatorFunctionName(std::set<VariableImpl>& p_variables, const std::string& lhs, const std::string& op, const std::string& rhs, bool isAssignment)
    {
        std::string lhsType = "";
        std::string rhsType = "";

        // For the purpose of this example, let's assume lhsType is "int" and rhsType is "int"
        lhsType = "int"; // Simplify for illustration
        rhsType = "int";

        std::string operatorName = _operatorNames.find(op)->second;

        // Construct the function name
        std::string functionName = lhsType + "_Operator" + operatorName;

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

    std::string Parser::_findUnaryOperatorFunctionName(std::set<VariableImpl>& p_variables, const std::string& op, const std::string& operand)
    {
        // Determine the type of operand (simplified for this example)
        std::string operandType = "int"; // Simplify for illustration

        // Get the operator name from the operator token
        auto it = _operatorNames.find(op);
        if (it == _operatorNames.end())
            return "";

        std::string operatorName = it->second;

        // Construct the function name
        std::string functionName = operandType + "_Operator" + operatorName;

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

    std::string Parser::_findPostfixOperatorFunctionName(std::set<VariableImpl>& p_variables, const std::string& op, const std::string& operand)
    {
        return _findUnaryOperatorFunctionName(p_variables, op, operand);
    }
}
