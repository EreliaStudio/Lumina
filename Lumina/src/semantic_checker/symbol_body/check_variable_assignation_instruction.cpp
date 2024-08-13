#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkVariableAssignationInstruction(const std::filesystem::path& p_file, const std::shared_ptr<VariableAssignationInstruction>& p_instruction, std::unordered_map<std::string, SemanticChecker::Type*> p_variables)
	{
		if (p_variables.contains(p_instruction->name->tokens[0].content) == false)
		{
			throw TokenBasedError(p_file, "Variable [" + p_instruction->name->tokens[0].content + "] Doesn't exist." + DEBUG_INFORMATION, p_instruction->name->tokens[0]);
		}

		Type* variableType = p_variables.at(p_instruction->name->tokens[0].content);

		for (size_t i = 1; i < p_instruction->name->tokens.size(); i++)
		{
			std::string expectedAttributeName = p_instruction->name->tokens[i].content;
			const auto& it = std::find_if(variableType->attributes.begin(), variableType->attributes.end(), [expectedAttributeName](const Type::Attribute& p_attribute) {
				return (p_attribute.name == expectedAttributeName);
				});

			if (it == variableType->attributes.end())
			{
				throw TokenBasedError(p_file, "Attribute [" + expectedAttributeName + "] not found in [" + variableType->name + "] type" + DEBUG_INFORMATION, p_instruction->name->tokens[i]);
			}

			variableType = it->type;
		}

		checkExpressionInstruction(p_file, p_instruction->initializer, p_variables, variableType);
	}
}