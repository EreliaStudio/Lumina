#include "parser.hpp"

#include <algorithm>

namespace Lumina
{
	SymbolBodyImpl Parser::_composeSymbolBody(std::set<VariableImpl>& p_variables, const SymbolBodyInfo& p_symbolBodyInfo, size_t depth)
	{
		SymbolBodyImpl result;

		std::string code;
		std::vector<FunctionImpl> calledFunctions;
		std::vector<TypeImpl> usedTypes;

		for (const auto& statement : p_symbolBodyInfo.statements)
		{
			try
			{
				std::string statementCode = _composeStatement(p_variables, statement, calledFunctions, usedTypes, depth);
				code += statementCode + "\n";
			}
			catch (Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
		}

		result.code = code;
		result.calledFunctions = calledFunctions;
		result.usedTypes = usedTypes;

		return result;
	}

	std::string Parser::_composeStatement(std::set<VariableImpl>& p_variables, const StatementInfo& p_statementInfo, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		switch (p_statementInfo.index())
		{
		case 0:
			return (_composeVariableDeclaration(p_variables, std::get<0>(p_statementInfo), calledFunctions, usedTypes, depth) + ";");
		case 1:
			return (_composeExpressionStatement(p_variables, std::get<1>(p_statementInfo), calledFunctions, usedTypes, depth) + ";");
		case 2:
			return (_composeAssignmentStatement(p_variables, std::get<2>(p_statementInfo), calledFunctions, usedTypes, depth) + ";");
		case 3:
			return (_composeReturnStatement(p_variables, std::get<3>(p_statementInfo), calledFunctions, usedTypes, depth) + ";");
		case 4:
			return (std::string(depth * 4, ' ') + "discard;");
		case 5:
			return (_composeIfStatement(p_variables, std::get<5>(p_statementInfo), calledFunctions, usedTypes, depth));
		case 6:
			return (_composeWhileStatement(p_variables, std::get<6>(p_statementInfo), calledFunctions, usedTypes, depth));
		case 7:
			return (_composeForStatement(p_variables, std::get<7>(p_statementInfo), calledFunctions, usedTypes, depth));
		case 8:
			return (_composeRaiseExceptionStatement(p_variables, std::get<8>(p_statementInfo), calledFunctions, usedTypes, depth) + ";");
		case 9:
		{
			SymbolBodyImpl innerBody = _composeSymbolBody(p_variables, std::get<9>(p_statementInfo).body, depth + 1);

			for (const auto& function : innerBody.calledFunctions)
			{
				if (std::find(calledFunctions.begin(), calledFunctions.end(), function) == calledFunctions.end())
				{
					calledFunctions.insert(calledFunctions.end(), function);
				}
			}

			for (const auto& structure : innerBody.usedTypes)
			{
				if (std::find(usedTypes.begin(), usedTypes.end(), structure) == usedTypes.end())
				{
					usedTypes.insert(usedTypes.end(), structure);
				}
			}

			return ("{\n" + innerBody.code + "}\n");
		}
		default:
			throw Lumina::TokenBasedError("Unknown statement type.", Token());
		}

		return ("");
	}

	std::string Parser::_composeVariableDeclaration(std::set<VariableImpl>& p_variables, const VariableDeclarationStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		VariableImpl var = _composeVariable(p_stmt.variable);

		std::string code = var.type.name + " " + var.name;

		for (const auto& dim : var.arraySizes)
		{
			code += "[" + std::to_string(dim) + "]";
		}

		if (p_stmt.initializer)
		{
			ExpressionTypeImpl variableType = { var.type, var.arraySizes };

			ExpressionTypeImpl initializerType = _deduceExpressionType(p_variables, *p_stmt.initializer);

			if (variableType.arraySizes != initializerType.arraySizes)
			{
				std::string variableTypeString = variableType.type.name;
				for (const auto& dim : variableType.arraySizes)
				{
					variableTypeString += "[" + std::to_string(dim) + "]";
				}
				std::string initializerTypeString = initializerType.type.name;
				for (const auto& dim : initializerType.arraySizes)
				{
					initializerTypeString += "[" + std::to_string(dim) + "]";
				}

				Token errorToken = _getExpressionToken(*p_stmt.initializer);
				throw TokenBasedError(
					"Cannot assign type [" + initializerTypeString + "] to type [" + variableTypeString + "] : array sizes doesn't match",
					errorToken
				);
			}

			bool conversionAvailable = false;

			if (variableType.type == initializerType.type)
			{
				conversionAvailable = true;
			}
			else
			{
				auto convIt = _convertionTable.find(initializerType);
				if (convIt != _convertionTable.end() && convIt->second.count(variableType) > 0)
				{
					conversionAvailable = true;
				}
			}

			std::string initializerCode = _composeExpression(p_variables, *p_stmt.initializer, calledFunctions, usedTypes);



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

		return std::string(depth * 4, ' ') + code;
	}

	std::string Parser::_composeExpressionStatement(std::set<VariableImpl>& p_variables, const ExpressionStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		return std::string(depth * 4, ' ') + _composeExpression(p_variables, *p_stmt.expression, calledFunctions, usedTypes);
	}

	std::string Parser::_composeAssignmentStatement(std::set<VariableImpl>& p_variables, const AssignmentStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		ExpressionTypeImpl targetExpressionType = _deduceExpressionType(p_variables, *p_stmt.target);
		ExpressionTypeImpl valueExpressionType = _deduceExpressionType(p_variables, *p_stmt.value);
		std::string target = _composeExpression(p_variables, *p_stmt.target, calledFunctions, usedTypes);
		std::string value = _composeExpression(p_variables, *p_stmt.value, calledFunctions, usedTypes);
		std::string op = p_stmt.operatorToken.content;

		FunctionImpl operatorFunction = _findOperatorFunction(p_variables, targetExpressionType, op, valueExpressionType, true);
		if (!operatorFunction.name.empty())
		{
			return std::string(depth * 4, ' ') + operatorFunction.name + "(" + target + ", " + value + ")";
		}
		else
		{
			return std::string(depth * 4, ' ') + target + " " + op + " " + value;
		}
	}

	std::string Parser::_composeReturnStatement(std::set<VariableImpl>& p_variables, const ReturnStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		if (p_stmt.expression)
		{
			return std::string(depth * 4, ' ') + "return " + _composeExpression(p_variables, *p_stmt.expression, calledFunctions, usedTypes);
		}
		else
		{
			return std::string(depth * 4, ' ') + "return";
		}
	}

	std::string Parser::_composeRaiseExceptionStatement(std::set<VariableImpl>& p_variables, const RaiseExceptionStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		return std::string(depth * 4, ' ') + _composeFunctionCallExpression(p_variables, *p_stmt.functionCall, calledFunctions, usedTypes);
	}

	std::string Parser::_composeIfStatement(std::set<VariableImpl>& p_variables, const IfStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		std::string code;
		bool first = true;
		for (const auto& branch : p_stmt.branches)
		{
			if (first == true)
			{
				code += std::string(depth * 4, ' ') + "if (" + _composeExpression(p_variables, *branch.condition, calledFunctions, usedTypes) + ")\n";
			}
			else
			{
				code += std::string(depth * 4, ' ') + "else if (" + _composeExpression(p_variables, *branch.condition, calledFunctions, usedTypes) + ")\n";
			}
			SymbolBodyImpl branchBody = _composeSymbolBody(p_variables, branch.body, depth + 1);

			for (const auto& function : branchBody.calledFunctions)
			{
				if (std::find(calledFunctions.begin(), calledFunctions.end(), function) == calledFunctions.end())
				{
					calledFunctions.push_back(function);
				}
			}


			for (const auto& structure : branchBody.usedTypes)
			{
				if (std::find(usedTypes.begin(), usedTypes.end(), structure) == usedTypes.end())
				{
					usedTypes.push_back(structure);
				}
			}

			code += std::string(depth * 4, ' ') + "{\n" + branchBody.code + std::string(depth * 4, ' ') + "}";
			
			first = false;
		}

		if (!p_stmt.elseBody.statements.empty())
		{
			SymbolBodyImpl elseBody = _composeSymbolBody(p_variables, p_stmt.elseBody, depth + 1);

			for (const auto& function : elseBody.calledFunctions)
			{
				if (std::find(calledFunctions.begin(), calledFunctions.end(), function) == calledFunctions.end())
				{
					calledFunctions.push_back(function);
				}
			}


			for (const auto& structure : elseBody.usedTypes)
			{
				if (std::find(usedTypes.begin(), usedTypes.end(), structure) == usedTypes.end())
				{
					usedTypes.push_back(structure);
				}
			}

			code += "\n" + std::string(depth * 4, ' ') + "else\n" + std::string(depth * 4, ' ') + "{\n" + elseBody.code + std::string(depth * 4, ' ') + "}";
		}

		return code;
	}

	std::string Parser::_composeWhileStatement(std::set<VariableImpl>& p_variables, const WhileStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		std::string condition = _composeExpression(p_variables, *p_stmt.loop.condition, calledFunctions, usedTypes);
		SymbolBodyImpl innerBody = _composeSymbolBody(p_variables, p_stmt.loop.body, depth + 1);

		for (const auto& function : innerBody.calledFunctions)
		{
			if (std::find(calledFunctions.begin(), calledFunctions.end(), function) == calledFunctions.end())
			{
				calledFunctions.push_back(function);
			}
		}

		for (const auto& structure : innerBody.usedTypes)
		{
			if (std::find(usedTypes.begin(), usedTypes.end(), structure) == usedTypes.end())
			{
				usedTypes.push_back(structure);
			}
		}

		std::string code = std::string(depth * 4, ' ') + "while (" + condition + ")\n" + std::string(depth * 4, ' ') + "{\n" + innerBody.code + std::string(depth * 4, ' ') + "}";

		return code;
	}

	std::string Parser::_composeForStatement(std::set<VariableImpl>& p_variables, const ForStatementInfo& p_stmt, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes, size_t depth)
	{
		std::string init = p_stmt.initializer ? _composeStatement(p_variables, *p_stmt.initializer, calledFunctions, usedTypes, 0) : "";
		std::string condition = p_stmt.condition ? _composeExpression(p_variables, *p_stmt.condition, calledFunctions, usedTypes) : "";
		std::string increment = p_stmt.increment ? _composeExpression(p_variables, *p_stmt.increment, calledFunctions, usedTypes) : "";

		SymbolBodyImpl innerBody = _composeSymbolBody(p_variables, p_stmt.body, depth + 1);

		for (const auto& function : innerBody.calledFunctions)
		{
			if (std::find(calledFunctions.begin(), calledFunctions.end(), function) == calledFunctions.end())
			{
				calledFunctions.push_back(function);
			}
		}

		for (const auto& structure : innerBody.usedTypes)
		{
			if (std::find(usedTypes.begin(), usedTypes.end(), structure) == usedTypes.end())
			{
				usedTypes.push_back(structure);
			}
		}

		if (!init.empty() && init.back() == ';')
			init.pop_back();

		std::string code = std::string(depth * 4, ' ') + "for (" + init + "; " + condition + "; " + increment + ")\n" + std::string(depth * 4, ' ') + "{\n" + innerBody.code + std::string(depth * 4, ' ') + "}";

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

		for (const auto& convertedType : _convertionTable[p_rhs])
		{
			FunctionImpl toTest = searchFunction;

			toTest.parameters.push_back({
					.type = convertedType.type,
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

		std::string functionName = p_operand.type.name + "_UnaryOperator" + operatorName;

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
