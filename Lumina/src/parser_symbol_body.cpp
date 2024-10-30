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

	ExpressionTypeImpl Parser::_deduceLiteralExpressionType(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& expr)
	{
		const Token& token = expr.value;
		// Assuming Token has a 'type' member to identify the token type
		if (token.type == Token::Type::Number)
		{
			return { _getType("float"), {} };
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
			throw Lumina::TokenBasedError("No variable named [" + name + "] declared in this scope", expr.variableName);
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
			std::string resultTypeName;
			if (lhsType.type.name == rhsType.type.name)
			{
				resultTypeName = lhsType.type.name;
			}
			else if ((lhsType.type.name == "float" && (rhsType.type.name == "int" || rhsType.type.name == "uint")) ||
				(rhsType.type.name == "float" && (lhsType.type.name == "int" || lhsType.type.name == "uint")))
			{
				resultTypeName = "float";
			}
			else
			{
				throw Lumina::TokenBasedError("Type mismatch in binary expression", e.operatorToken);
			}
			return { _getType(resultTypeName), {} };
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
			name += ns.content + "::";
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
					if (func.parameters[i].type.name != argTypes[i].type.name)
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

		throw Lumina::TokenBasedError("Function not found or argument types do not match", e.functionName);
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
