#include "parser.hpp"
#include "utils.hpp"

namespace Lumina
{
	std::string Parser::_composeExpression(std::set<VariableImpl>& p_variables, const ExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		switch (p_expr.index())
		{
		case 0:
			return (_composeLiteralExpression(p_variables, std::get<0>(p_expr), usedTypes));
		case 1:
			return (_composeVariableExpression(p_variables, std::get<1>(p_expr), usedTypes));
		case 2:
			return (_composeBinaryExpression(p_variables, std::get<2>(p_expr), calledFunctions, usedTypes));
		case 3:
			return (_composeUnaryExpression(p_variables, std::get<3>(p_expr), calledFunctions, usedTypes));
		case 4:
			return (_composePostfixExpression(p_variables, std::get<4>(p_expr), calledFunctions, usedTypes));
		case 5:
			return (_composeFunctionCallExpression(p_variables, std::get<5>(p_expr), calledFunctions, usedTypes));
		case 6:
			return (_composeMethodCallExpression(p_variables, std::get<6>(p_expr), calledFunctions, usedTypes));
		case 7:
			return (_composeMemberAccessExpression(p_variables, std::get<7>(p_expr), calledFunctions, usedTypes));
		case 8:
			return (_composeArrayAccessExpression(p_variables, std::get<8>(p_expr), calledFunctions, usedTypes));
		case 9:
			return (_composeArrayDefinitionExpression(p_variables, std::get<9>(p_expr), calledFunctions, usedTypes));
		default:
			throw Lumina::TokenBasedError("Unknown expression type.", Token());
		}
	}

	std::string Parser::_composeLiteralExpression(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& p_expr, std::vector<TypeImpl>& usedTypes)
	{
		ExpressionTypeImpl exprType = _deduceLiteralExpressionType(p_variables, p_expr);

		if (std::find(usedTypes.begin(), usedTypes.end(), exprType.type) == usedTypes.end())
		{
			usedTypes.push_back(exprType.type);
		}

		return p_expr.value.content;
	}

	std::string Parser::_composeVariableExpression(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& p_expr, std::vector<TypeImpl>& usedTypes)
	{
		std::string name;
		for (const auto& ns : p_expr.namespacePath)
		{
			name += ns.content + "::";
		}
		name += p_expr.variableName.content;

		ExpressionTypeImpl exprType = _deduceVariableExpressionType(p_variables, p_expr);

		if (std::find(usedTypes.begin(), usedTypes.end(), exprType.type) == usedTypes.end())
		{
			usedTypes.push_back(exprType.type);
		}

		if (p_variables.contains({ {}, name, {} }) == false)
		{
			auto thisVarIt = std::find_if(
				p_variables.begin(),
				p_variables.end(),
				[](const VariableImpl& var) {
					return var.name == "this";
				}
			);

			if (thisVarIt == p_variables.end())
			{
				throw TokenBasedError("No variable named [this] declared in this scope", p_expr.variableName);
			}

			const auto& attributes = thisVarIt->type.attributes;
			auto attrIt = std::find_if(
				attributes.begin(),
				attributes.end(),
				[&name](const VariableImpl& attr) {
					return attr.name == name;
				}
			);

			if (attrIt == attributes.end())
			{
				throw TokenBasedError("No variable named [" + name + "] declared in this scope", p_expr.variableName);
			}

			return ("this." + name);
		}
		return name;
	}

	std::string Parser::_composeBinaryExpression(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		ExpressionTypeImpl leftExpressionType = _deduceExpressionType(p_variables, *(p_expr.left));
		ExpressionTypeImpl rightExpressionType = _deduceExpressionType(p_variables, *(p_expr.right));

		if (std::find(usedTypes.begin(), usedTypes.end(), leftExpressionType.type) == usedTypes.end())
		{
			usedTypes.push_back(leftExpressionType.type);
		}
		if (std::find(usedTypes.begin(), usedTypes.end(), rightExpressionType.type) == usedTypes.end())
		{
			usedTypes.push_back(rightExpressionType.type);
		}

		std::string lhs = _composeExpression(p_variables, *(p_expr.left), calledFunctions, usedTypes);
		std::string rhs = _composeExpression(p_variables, *(p_expr.right), calledFunctions, usedTypes);
		std::string op = p_expr.operatorToken.content;

		if (leftExpressionType == rightExpressionType && op == "=")
		{
			return (lhs + " = " + rhs);
		}

		if (leftExpressionType.arraySizes.size() != 0)
		{
			throw TokenBasedError("Can't execute operation on array [" + leftExpressionType.type.name + "] object" + DEBUG_INFORMATION, _getExpressionToken(*(p_expr.left)));
		}

		FunctionImpl operatorFunction = _findOperatorFunction(p_variables, leftExpressionType, op, rightExpressionType);

		if (operatorFunction.name.size() == 0)
		{
			throw TokenBasedError("No operator [" + op + "] for type [" + leftExpressionType.type.name + "] with parameter [" + rightExpressionType.type.name + "]" + DEBUG_INFORMATION, _getExpressionToken(p_expr));
		}

		if (operatorFunction.body.code == "")
		{
			return (lhs + " " + op + " " + rhs);
		}

		calledFunctions.push_back(operatorFunction);

		return operatorFunction.name + "(" + lhs + ", " + rhs + ")";
	}

	std::string Parser::_composeUnaryExpression(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		ExpressionTypeImpl operandExpressionType = _deduceExpressionType(p_variables, *(p_expr.operand));

		std::string operand = _composeExpression(p_variables, *(p_expr.operand), calledFunctions, usedTypes);
		std::string op = p_expr.operatorToken.content;

		if (operandExpressionType.arraySizes.size() != 0)
		{
			throw TokenBasedError("Can't execute operation on array [" + operandExpressionType.type.name + "] object" + DEBUG_INFORMATION, _getExpressionToken(*(p_expr.operand)));
		}

		FunctionImpl operatorFunction = _findUnaryOperatorFunction(p_variables, op, operandExpressionType);

		if (operatorFunction.name.size() == 0)
		{
			throw TokenBasedError("No operator [" + op + "] for type [" + operandExpressionType.type.name + "]" + DEBUG_INFORMATION, _getExpressionToken(p_expr));
		}

		if (operatorFunction.body.code == "")
		{
			return (op + operand);
		}
		calledFunctions.push_back(operatorFunction);

		return operatorFunction.name + "(" + operand + ")";
	}

	std::string Parser::_composePostfixExpression(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		ExpressionTypeImpl operandExpressionType = _deduceExpressionType(p_variables, *(p_expr.operand));

		std::string operand = _composeExpression(p_variables, *(p_expr.operand), calledFunctions, usedTypes);
		std::string op = p_expr.operatorToken.content;

		if (operandExpressionType.arraySizes.size() != 0)
		{
			throw TokenBasedError("Can't execute operation on array [" + operandExpressionType.type.name + "] object" + DEBUG_INFORMATION, _getExpressionToken(*(p_expr.operand)));
		}

		FunctionImpl operatorFunction = _findPostfixOperatorFunction(p_variables, op, operandExpressionType);

		if (operatorFunction.name.size() == 0)
		{
			throw TokenBasedError("No operator [" + op + "] for type [" + operandExpressionType.type.name + "]" + DEBUG_INFORMATION, _getExpressionToken(p_expr));
		}

		if (operatorFunction.body.code == "")
		{
			return (operand + op);
		}

		calledFunctions.push_back(operatorFunction);
		return operatorFunction.name + "(" + operand + ")";
	}

	std::string Parser::_composeFunctionCallExpression(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		std::string name;
		for (const auto& ns : p_expr.namespacePath)
		{
			name += ns.content + "::";
		}
		name += p_expr.functionName.content;

		std::vector<ExpressionTypeImpl> argumentTypes;
		std::vector<std::string> argumentCodes;

		for (const auto& argExpr : p_expr.arguments)
		{
			ExpressionTypeImpl exprType = _deduceExpressionType(p_variables, *argExpr);
			argumentTypes.push_back(exprType);

			if (std::find(usedTypes.begin(), usedTypes.end(), exprType.type) == usedTypes.end())
			{
				usedTypes.push_back(exprType.type);
			}

			std::string argCode = _composeExpression(p_variables, *argExpr, calledFunctions, usedTypes);
			argumentCodes.push_back(argCode);
		}

		FunctionImpl* matchingFunction = _findFunctionWithConversions(name, argumentTypes);

		if (!matchingFunction)
		{
			std::string parameterString;
			for (size_t i = 0; i < argumentTypes.size(); ++i)
			{
				if (i > 0)
				{
					parameterString += ", ";
				}
				parameterString += argumentTypes[i].type.name;
				for (const auto& size : argumentTypes[i].arraySizes)
				{
					parameterString += "[" + std::to_string(size) + "]";
				}
			}

			throw TokenBasedError(
				"No function [" + p_expr.functionName.content + "] detected with parameters [" + parameterString + "]" + DEBUG_INFORMATION,
				p_expr.functionName
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

			if (i > 0)
			{
				args += ", ";
			}
			args += argCode;
		}

		if (matchingFunction->body.code != "")
		{
			calledFunctions.push_back(*matchingFunction);
		}
		if (std::find(usedTypes.begin(), usedTypes.end(), matchingFunction->returnType.type) == usedTypes.end())
		{
			usedTypes.push_back(matchingFunction->returnType.type);
		}
		return name + "(" + args + ")";
	}

	std::vector<const FunctionImpl *> Parser::_getFunctionsWithNamespaces(const std::string& p_relativeName)
	{
		std::vector<const FunctionImpl *> result;

		for (const auto& func : _availibleFunctions)
		{
			if (func.name == p_relativeName)
			{
				result.push_back(&func);
			}
		}

		std::string namespacePrefix = "";
		for (const auto& ns : _nspaces)
		{
			if (!namespacePrefix.empty())
			{
				namespacePrefix += "::";
			}
			namespacePrefix += ns;

			std::string qualifiedName = namespacePrefix + "::" + p_relativeName;

			for (const auto& func : _availibleFunctions)
			{ 
				if (func.name == qualifiedName)
				{
					result.push_back(&func);
				}
			}
		}

		return result;
	}

	FunctionImpl* Parser::_findFunctionWithConversions(const std::string& name, const std::vector<ExpressionTypeImpl>& argumentTypes)
	{
		auto candidateFunctions = _getFunctionsWithNamespaces(name);

		for (const auto& functPtr : candidateFunctions)
		{
			const FunctionImpl& funct = *functPtr;

			if (funct.parameters.size() != argumentTypes.size())
			{
				continue;
			}

			bool exactMatch = true;
			for (size_t i = 0; i < argumentTypes.size(); ++i)
			{
				const ExpressionTypeImpl& argType = argumentTypes[i];
				const ParameterImpl& param = funct.parameters[i];

				if (!(argType.type == param.type && argType.arraySizes == param.arraySizes))
				{
					exactMatch = false;
					break;
				}
			}

			if (exactMatch)
			{
				return const_cast<FunctionImpl*>(functPtr);
			}
		}

		const FunctionImpl* bestMatch = nullptr;
		int lowestConversionCost = INT_MAX;
		bool ambiguous = false;

		for (const auto* functPtr : candidateFunctions)
		{
			const FunctionImpl& funct = *functPtr;

			if (funct.parameters.size() != argumentTypes.size())
			{
				continue;
			}

			int totalConversionCost = 0;
			bool match = true;

			for (size_t i = 0; i < argumentTypes.size(); ++i)
			{
				const ExpressionTypeImpl& argType = argumentTypes[i];
				const ParameterImpl& param = funct.parameters[i];

				if (argType.type == param.type && argType.arraySizes == param.arraySizes)
				{
					continue;
				}
				else
				{
					auto convIt = _convertionTable.find(argType.type);
					if (convIt != _convertionTable.end() && convIt->second.count(param.type) > 0)
					{
						totalConversionCost += 1;
					}
					else
					{
						match = false;
						break;
					}
				}
			}

			if (match)
			{
				if (totalConversionCost < lowestConversionCost)
				{
					bestMatch = functPtr;
					lowestConversionCost = totalConversionCost;
					ambiguous = false;
				}
				else if (totalConversionCost == lowestConversionCost)
				{
					ambiguous = true;
				}
			}
		}

		if (ambiguous)
		{
			return nullptr;
		}

		return const_cast<FunctionImpl*>(bestMatch);
	}

	std::string Parser::_composeMethodCallExpression(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		std::string objectExpression = _composeExpression(p_variables, *(p_expr.object), calledFunctions, usedTypes);
		ExpressionTypeImpl objectType = _deduceExpressionType(p_variables, *(p_expr.object));

		std::string methodName = objectType.type.name + "_" + p_expr.name.content;

		std::vector<ExpressionTypeImpl> argumentTypes;
		std::vector<std::string> argumentExpressions;

		argumentTypes.push_back(objectType);
		argumentExpressions.push_back(objectExpression);

		for (const auto& argExpr : p_expr.arguments)
		{
			ExpressionTypeImpl argType = _deduceExpressionType(p_variables, *argExpr);
			argumentTypes.push_back(argType);

			std::string argExpression = _composeExpression(p_variables, *argExpr, calledFunctions, usedTypes);
			argumentExpressions.push_back(argExpression);
		}

		FunctionImpl* matchingMethod = _findFunctionWithConversions(methodName, argumentTypes);

		if (matchingMethod == nullptr)
		{
			std::string parameterString;
			for (size_t i = 1; i < argumentTypes.size(); ++i)
			{
				if (parameterString.size() != 0)
				{
					parameterString += ", ";
				}

				parameterString += argumentTypes[i].type.name;
				for (const auto& size : argumentTypes[i].arraySizes)
				{
					parameterString += "[" + std::to_string(size) + "]";
				}
			}

			Token errorToken = _getExpressionToken(*(p_expr.object)) + p_expr.name + _getExpressionToken(*(p_expr.arguments).front());

			throw TokenBasedError(
				"No method [" + p_expr.name.content + "] for type [" + objectType.type.name + "] with parameters [" + parameterString + "]",
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
					Token errorToken = _getExpressionToken(*(p_expr.object));
					throw TokenBasedError(
						"Cannot convert type [" + argType.type.name + "] to [" + param.type.name + "] for parameter " + std::to_string(i),
						errorToken
					);
				}
			}

			if (i > 0)
			{
				parameters += ", ";
			}

			parameters += argExpression;
		}

		if (matchingMethod->body.code == "")
		{
			return (p_expr.name.content + "(" + parameters + ")");
		}
		calledFunctions.push_back(*matchingMethod);
		return methodName + "(" + parameters + ")";
	}

	std::string Parser::_composeMemberAccessExpression(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		std::string object = _composeExpression(p_variables, *(p_expr.object), calledFunctions, usedTypes);
		std::string member = p_expr.memberName.content;
		return object + "." + member;
	}

	std::string Parser::_composeArrayAccessExpression(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& p_expr, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		std::string arrayResult = _composeExpression(p_variables, *(p_expr.array), calledFunctions, usedTypes);
		std::string index = _composeExpression(p_variables, *(p_expr.index), calledFunctions, usedTypes);
		return arrayResult + "[" + index + "]";
	}
	
	std::string Parser::_composeArrayDefinitionExpression(std::set<VariableImpl>& p_variables, const ArrayDefinitionExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes)
	{
		std::string result = "{";

		for (const auto& element : p_expression.elements)
		{
			if (result.size() != 1)
			{
				result += ", ";
			}

			result += _composeExpression(p_variables, *element, calledFunctions, usedTypes);
		}

		result += "}";

		return (result);
	}
}
