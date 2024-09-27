#include "lumina_compiler.hpp"

namespace Lumina
{
	bool Compiler::checkVariableCollision(const std::vector<Variable>& p_availableVariables, const Variable& variableToCheck)
	{
		const std::string& target = (variableToCheck.name);
		auto it = std::find_if(p_availableVariables.begin(), p_availableVariables.end(), [&target](const Variable& var) {
			return var.name == target;
			});

		return it != p_availableVariables.end();
	}

	std::string Compiler::parseVariableDeclaration(const std::shared_ptr<VariableDeclaration>& instruction, std::vector<Variable>& p_availableVariables)
	{
		std::string result = "";

		Variable newVariable = composeVariable(instruction->descriptor);

		if (checkVariableCollision(p_availableVariables, newVariable) == true)
		{
			throw TokenBasedError("Variable [" + newVariable.name + "] already exist in this scope.", instruction->descriptor.name);
		}

		if (instruction->initialValue != nullptr)
		{
			Variable expressionVariable = evaluateExpressionResult(instruction->initialValue);

			if (expressionVariable.type == nullptr)
			{
				throw TokenBasedError("Impossible to evaluate expression type.", instruction->initialValue->token());
			}

			if (expressionVariable.isSame(newVariable) == false)
			{
				throw TokenBasedError("No conversion found from [" + expressionVariable.typeString() + "] to [" + newVariable.typeString() + "].", instruction->initialValue->token());
			}
		}

		return result;
	}

	std::string Compiler::parseVariableAssignation(const std::shared_ptr<VariableAssignation>& instruction)
	{
		std::string result = "";

		return result;
	}

	std::string Compiler::parseSymbolCall(const std::shared_ptr<SymbolCall>& instruction)
	{
		std::string result = "";


		return result;
	}

	std::string Compiler::parseIfStatement(const std::shared_ptr<IfStatement>& instruction)
	{
		std::string result = "";

		return result;
	}

	std::string Compiler::parseWhileStatement(const std::shared_ptr<WhileStatement>& instruction)
	{
		std::string result = "";

		return result;
	}

	std::string Compiler::parseForStatement(const std::shared_ptr<ForStatement>& instruction)
	{
		std::string result = "";

		return result;
	}

	std::string Compiler::parseReturnStatement(const std::shared_ptr<ReturnStatement>& instruction)
	{
		std::string result = "";

		result += "return";
		if (instruction->returnValue.has_value())
		{
			result += " " + parseExpression(instruction->returnValue.value());
		}
		result += ";";

		return result;
	}

	std::string Compiler::parseDiscardStatement(const std::shared_ptr<DiscardStatement>& instruction)
	{
		std::string result = "";

		result += "discard;";
		return result;
	}
}
