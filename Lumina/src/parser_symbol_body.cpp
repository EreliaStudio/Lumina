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

	std::string Parser::_composeVariableDeclaration(std::set<VariableImpl>& p_variables, const VariableDeclarationStatementInfo& p_stmt)
	{
		VariableImpl var = _composeVariable(p_stmt.variable);

		std::string code = var.type.name + " " + var.name;

		if (p_stmt.initializer)
		{
			ExpressionTypeImpl variableType = { var.type, var.arraySizes };

			ExpressionTypeImpl initializerType = _deduceExpressionType(p_variables, *p_stmt.initializer);

			bool conversionAvailable = false;

			if (variableType.type == initializerType.type && variableType.arraySizes == initializerType.arraySizes)
			{
				conversionAvailable = true;
			}
			else
			{
				auto convIt = _convertionTable.find(initializerType.type);
				if (convIt != _convertionTable.end() && convIt->second.count(variableType.type) > 0)
				{
					conversionAvailable = true;
				}
			}

			std::string initializerCode = _composeExpression(p_variables, *p_stmt.initializer);

			if (conversionAvailable)
			{
				if (initializerType.type != variableType.type)
				{
					initializerCode = "(" + variableType.type.name + ")(" + initializerCode + ")";
				}
				code += " = " + initializerCode;
			}
			else
			{
				std::string op = "=";

				FunctionImpl operatorFunction = _findOperatorFunction(p_variables, variableType, op, initializerType, true);

				if (operatorFunction.name.size() != 0)
				{
					code += " = " + operatorFunction.name + "(" + var.name + ", " + initializerCode + ")";
				}
				else
				{
					Token errorToken = _getExpressionToken(*p_stmt.initializer);
					throw TokenBasedError(
						"Cannot assign type [" + initializerType.type.name + "] to variable of type [" + variableType.type.name + "]",
						errorToken
					);
				}
			}
		}

		p_variables.insert(var);

		return code;
	}

	std::string Parser::_composeExpressionStatement(std::set<VariableImpl>& p_variables, const ExpressionStatementInfo& p_stmt)
	{
		return _composeExpression(p_variables, *p_stmt.expression);
	}

	std::string Parser::_composeAssignmentStatement(std::set<VariableImpl>& p_variables, const AssignmentStatementInfo& p_stmt)
	{
		ExpressionTypeImpl targetExpressionType = _deduceExpressionType(p_variables, *p_stmt.target);
		ExpressionTypeImpl valueExpressionType = _deduceExpressionType(p_variables, *p_stmt.value);
		std::string target = _composeExpression(p_variables, *p_stmt.target);
		std::string value = _composeExpression(p_variables, *p_stmt.value);
		std::string op = p_stmt.operatorToken.content;

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

	std::string Parser::_composeReturnStatement(std::set<VariableImpl>& p_variables, const ReturnStatementInfo& p_stmt)
	{
		if (p_stmt.expression)
		{
			return "return " + _composeExpression(p_variables, *p_stmt.expression);
		}
		else
		{
			return "return";
		}
	}

	std::string Parser::_composeRaiseExceptionStatement(std::set<VariableImpl>& p_variables, const RaiseExceptionStatementInfo& p_stmt)
	{
		return _composeFunctionCallExpression(p_variables, *p_stmt.functionCall);
	}

	std::string Parser::_composeIfStatement(std::set<VariableImpl>& p_variables, const IfStatementInfo& p_stmt)
	{
		std::string code;
		bool first = true;
		for (const auto& branch : p_stmt.branches)
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

		if (!p_stmt.elseBody.statements.empty())
		{
			code += "else\n{\n" + _composeSymbolBody(p_variables, p_stmt.elseBody).code + "}\n";
		}

		return code;
	}

	std::string Parser::_composeWhileStatement(std::set<VariableImpl>& p_variables, const WhileStatementInfo& p_stmt)
	{
		std::string condition = _composeExpression(p_variables, *p_stmt.loop.condition);
		std::string body = _composeSymbolBody(p_variables, p_stmt.loop.body).code;

		std::string code = "while (" + condition + ")\n{\n" + body + "}\n";

		return code;
	}

	std::string Parser::_composeForStatement(std::set<VariableImpl>& p_variables, const ForStatementInfo& p_stmt)
	{
		std::string init = p_stmt.initializer ? _composeStatement(p_variables, *p_stmt.initializer) : "";
		std::string condition = p_stmt.condition ? _composeExpression(p_variables, *p_stmt.condition) : "";
		std::string increment = p_stmt.increment ? _composeExpression(p_variables, *p_stmt.increment) : "";
		std::string body = _composeSymbolBody(p_variables, p_stmt.body).code;

		if (!init.empty() && init.back() == ';')
			init.pop_back();

		std::string code = "for (" + init + "; " + condition + "; " + increment + ")\n{\n" + body + "}\n";

		return code;
	}
	
	FunctionImpl Parser::_findOperatorFunction(std::set<VariableImpl>& p_variables, const ExpressionTypeImpl& p_lhs, const std::string& p_op, const ExpressionTypeImpl& p_rhs, bool isAssignment)
	{
		std::string operatorName = _operatorNames.find(p_op)->second;

		std::string functionName = p_lhs.type.name + "_Operator" + operatorName;

		FunctionImpl searchFunction;
		searchFunction.name = functionName;
		searchFunction.parameters.push_back({
				.type = p_lhs.type,
				.arraySizes = p_lhs.arraySizes
			});

		for (const auto& convertedType : _convertionTable[p_rhs.type])
		{
			FunctionImpl toTest = searchFunction;

			toTest.parameters.push_back({
					.type = convertedType,
					.arraySizes = p_rhs.arraySizes
				});

			auto funcIt = _availibleFunctions.find(toTest);
			if (funcIt != _availibleFunctions.end())
			{
				return (*funcIt);
			}
		}

		return FunctionImpl();
	}

	FunctionImpl Parser::_findUnaryOperatorFunction(std::set<VariableImpl>& p_variables, const std::string& p_op, const ExpressionTypeImpl& p_operand)
	{
		std::string operatorName = _operatorNames.find(p_op)->second;

		std::string functionName = p_operand.type.name + "_Operator" + operatorName;

		FunctionImpl searchFunction;
		searchFunction.name = functionName;
		searchFunction.parameters.push_back({
				.type = p_operand.type,
				.arraySizes = p_operand.arraySizes
			});

		auto funcIt = _availibleFunctions.find(searchFunction);
		if (funcIt != _availibleFunctions.end())
		{
			return (*funcIt);
		}

		return FunctionImpl();
	}
	
	FunctionImpl Parser::_findPostfixOperatorFunction(std::set<VariableImpl>& p_variables, const std::string& p_op, const ExpressionTypeImpl& p_operand)
	{
		return _findUnaryOperatorFunction(p_variables, p_op, p_operand);
	}
}
