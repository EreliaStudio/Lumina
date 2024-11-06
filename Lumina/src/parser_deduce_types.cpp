#include "parser.hpp"
#include "utils.hpp"

namespace Lumina
{
	Token Parser::_getExpressionToken(const ExpressionInfo& p_expr) {
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
				Token leftToken = _getExpressionToken(*arg.left);
				Token operatorToken = arg.operatorToken;
				Token rightToken = _getExpressionToken(*arg.right);
				return leftToken + operatorToken + rightToken;
			}
			else if constexpr (std::is_same_v<T, UnaryExpressionInfo>) {
				Token operatorToken = arg.operatorToken;
				Token operandToken = _getExpressionToken(*arg.operand);
				return operatorToken + operandToken;
			}
			else if constexpr (std::is_same_v<T, PostfixExpressionInfo>) {
				Token operandToken = _getExpressionToken(*arg.operand);
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
					Token argToken = _getExpressionToken(*arg.arguments[i]);
					result = result + argToken;
				}
				return result;
			}
			else if constexpr (std::is_same_v<T, MethodCallExpressionInfo>) {
				Token objectToken = _getExpressionToken(*arg.object);
				Token result = objectToken + arg.name;
				for (size_t i = 0; i < arg.arguments.size(); ++i)
				{
					Token argToken = _getExpressionToken(*arg.arguments[i]);
					result = result + argToken;
				}
				return result;
			}
			else if constexpr (std::is_same_v<T, MemberAccessExpressionInfo>) {
				Token objectToken = _getExpressionToken(*arg.object);
				return objectToken + arg.memberName;
			}
			else if constexpr (std::is_same_v<T, ArrayAccessExpressionInfo>) {
				Token arrayToken = _getExpressionToken(*arg.array);
				Token indexToken = _getExpressionToken(*arg.index);
				indexToken.content += "]";
				return arrayToken + indexToken;
			}
			}, p_expr);
	}

	ExpressionTypeImpl Parser::_deduceLiteralExpressionType(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& p_expr)
	{
		const Token& token = p_expr.value;

		if (token.type == Token::Type::Number)
		{
			const std::string& content = token.content;
			std::string numberContent = content;
			bool isUnsigned = false;
			bool isFloat = false;
			bool isNegative = false;

			if (!numberContent.empty() && (numberContent.back() == 'u' || numberContent.back() == 'U'))
			{
				isUnsigned = true;
				numberContent.pop_back();
			}

			if (!numberContent.empty() && (numberContent.back() == 'f' || numberContent.back() == 'F'))
			{
				isFloat = true;
				numberContent.pop_back();
			}

			if (!numberContent.empty() && numberContent.front() == '-')
			{
				isNegative = true;
				numberContent.erase(0, 1);
			}

			if (numberContent.find('.') != std::string::npos)
			{
				isFloat = true;
			}

			if (isFloat)
			{
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
					return { _getType("uint"), {} };
				}
				else
				{
					return { _getType("int"), {} };
				}
			}
		}
		else
		{
			throw Lumina::TokenBasedError("Unknown literal type.", token);
		}
	}

	ExpressionTypeImpl Parser::_deduceVariableExpressionType(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& p_expr)
	{
		std::string name;
		for (const auto& ns : p_expr.namespacePath)
		{
			name += ns.content + "::";
		}
		name += p_expr.variableName.content;

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
				auto attrIt = std::find_if(
					thisIt->type.attributes.begin(),
					thisIt->type.attributes.end(),
					[&name](const VariableImpl& attr) {
						return attr.name == name;
					}
				);

				if (attrIt != thisIt->type.attributes.end())
				{
					return { attrIt->type, attrIt->arraySizes };
				}
			}
			throw Lumina::TokenBasedError("No variable named [" + name + "] declared in this scope" + DEBUG_INFORMATION, p_expr.variableName);
		}
	}

	ExpressionTypeImpl Parser::_deduceBinaryExpressionType(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& p_expr)
	{
		ExpressionTypeImpl lhsType = _deduceExpressionType(p_variables, *(p_expr.left));
		ExpressionTypeImpl rhsType = _deduceExpressionType(p_variables, *(p_expr.right));
		std::string op = p_expr.operatorToken.content;

		if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
		{
			return { _getType("bool"), {} };
		}
		else if (op == "&&" || op == "||")
		{
			if (lhsType.type.name != "bool" || rhsType.type.name != "bool")
			{
				throw Lumina::TokenBasedError("Logical operators require boolean operands", p_expr.operatorToken);
			}
			return { _getType("bool"), {} };
		}
		else
		{
			FunctionImpl operatorFunction = _findOperatorFunction(p_variables, lhsType, op, rhsType);

			if (operatorFunction.name.size() != 0)
			{
				return operatorFunction.returnType;
			}
			else
			{
				Token leftToken = _getExpressionToken(*(p_expr.left));
				Token operatorToken = p_expr.operatorToken;
				Token rightToken = _getExpressionToken(*(p_expr.right));

				Token errorToken = leftToken + operatorToken + rightToken;

				throw TokenBasedError(
					"No operator [" + op + "] for type [" + lhsType.type.name + "] with parameters [" + rhsType.type.name + "]" + DEBUG_INFORMATION,
					errorToken
				);
			}
		}
	}

	ExpressionTypeImpl Parser::_deduceUnaryExpressionType(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& p_expr)
	{
		ExpressionTypeImpl operandType = _deduceExpressionType(p_variables, *(p_expr.operand));
		return operandType;
	}

	ExpressionTypeImpl Parser::_deducePostfixExpressionType(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& p_expr)
	{
		ExpressionTypeImpl operandType = _deduceExpressionType(p_variables, *(p_expr.operand));
		return operandType;
	}

	ExpressionTypeImpl Parser::_deduceFunctionCallExpressionType(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& p_expr)
	{
		std::string name;
		for (const auto& ns : p_expr.namespacePath)
		{
			name += ns.content + "::";
		}
		name += p_expr.functionName.content;

		std::vector<ExpressionTypeImpl> argTypes;
		for (const auto& argExpr : p_expr.arguments)
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

		throw Lumina::TokenBasedError("Function [" + name + "] not found or argument types do not match" + DEBUG_INFORMATION, p_expr.functionName);
	}

	ExpressionTypeImpl Parser::_deduceMethodCallExpressionType(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& p_expr)
	{
		ExpressionTypeImpl objectType = _deduceExpressionType(p_variables, *(p_expr.object));
		std::string methodName = p_expr.name.content;

		std::vector<ExpressionTypeImpl> argTypes;
		for (const auto& argExpr : p_expr.arguments)
		{
			argTypes.push_back(_deduceExpressionType(p_variables, *argExpr));
		}

		std::string fullMethodName = objectType.type.name + "_" + methodName;

		for (const auto& func : _availibleFunctions)
		{
			if (func.name == fullMethodName && func.parameters.size() == argTypes.size() + 1)
			{
				if (func.parameters[0].type != objectType.type)
				{
					continue;
				}

				bool match = true;
				for (size_t i = 0; i < argTypes.size(); ++i)
				{
					if (func.parameters[i + 1].type != argTypes[i].type)
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

		throw Lumina::TokenBasedError("Method [" + objectType.type.name + "::" + methodName + "] not found or argument types do not match", p_expr.name);
	}

	ExpressionTypeImpl Parser::_deduceMemberAccessExpressionType(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& p_expr)
	{
		ExpressionTypeImpl objectType = _deduceExpressionType(p_variables, *(p_expr.object));
		std::string memberName = p_expr.memberName.content;


		auto it = std::find_if(
			objectType.type.attributes.begin(),
			objectType.type.attributes.end(),
			[&memberName](const VariableImpl& attr) {
				return attr.name == memberName;
			}
		);

		if (it != objectType.type.attributes.end())
		{
			return { it->type, it->arraySizes };
		}
		else
		{
			std::set<std::string> acceptedSwizzlingStructures = { "Vector3", "Vector3Int", "Vector3UInt","Vector4", "Vector4Int", "Vector4UInt", };

			if (objectType.arraySizes.size() == 0 && acceptedSwizzlingStructures.contains(objectType.type.name) && memberName.size() >= 2 && memberName.size() <= 4)
			{
				bool error = false;

				for (const auto& c : memberName)
				{
					auto it = std::find_if(
						objectType.type.attributes.begin(),
						objectType.type.attributes.end(),
						[&c](const VariableImpl& attr) {
							return attr.name == std::string(1, c);
						}
					);

					if (it != objectType.type.attributes.end())
					{
						return { it->type, it->arraySizes };
					}
				}

				if (error == false)
				{
					std::string expectedType = objectType.type.name;
					expectedType[6] = '0' + static_cast<char>(memberName.size());
					return { _getType(expectedType), {} };
				}
			}

			throw Lumina::TokenBasedError("Member [" + memberName + "] not found in type [" + objectType.type.name + "]", p_expr.memberName);
		}
	}

	ExpressionTypeImpl Parser::_deduceArrayAccessExpressionType(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& p_expr)
	{
		ExpressionTypeImpl arrayType = _deduceExpressionType(p_variables, *(p_expr.array));
		ExpressionTypeImpl indexType = _deduceExpressionType(p_variables, *(p_expr.index));

		const MemberAccessExpressionInfo* arrayExpr = std::get_if<MemberAccessExpressionInfo>(&*(p_expr.array));
		const LiteralExpressionInfo* indexExpr = std::get_if<LiteralExpressionInfo>(&*(p_expr.index));


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

	ExpressionTypeImpl Parser::_deduceExpressionType(std::set<VariableImpl>& p_variables, const ExpressionInfo& p_expr)
	{
		switch (p_expr.index())
		{
		case 0:
			return _deduceLiteralExpressionType(p_variables, std::get<0>(p_expr));
		case 1:
			return _deduceVariableExpressionType(p_variables, std::get<1>(p_expr));
		case 2:
			return _deduceBinaryExpressionType(p_variables, std::get<2>(p_expr));
		case 3:
			return _deduceUnaryExpressionType(p_variables, std::get<3>(p_expr));
		case 4:
			return _deducePostfixExpressionType(p_variables, std::get<4>(p_expr));
		case 5:
			return _deduceFunctionCallExpressionType(p_variables, std::get<5>(p_expr));
		case 6:
			return _deduceMethodCallExpressionType(p_variables, std::get<6>(p_expr));
		case 7:
			return _deduceMemberAccessExpressionType(p_variables, std::get<7>(p_expr));
		case 8:
			return _deduceArrayAccessExpressionType(p_variables, std::get<8>(p_expr));
		default:
			throw Lumina::TokenBasedError("Unknown expression type.", Token());
		}
	}
}
