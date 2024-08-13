#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkConditionElementInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ConditionElementInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables)
	{
		Type* lhType = getExpressionType(p_file, p_instruction->lhs, p_variables);
		checkExpressionInstruction(p_file, p_instruction->lhs, p_variables, lhType);
		if (p_instruction->rhs != nullptr)
			checkExpressionInstruction(p_file, p_instruction->rhs, p_variables, lhType);
	}
	
	void SemanticChecker::checkConditionInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ConditionInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables)
	{
		for (auto conditionElement : p_instruction->elements)
		{
			checkConditionElementInstruction(p_file, conditionElement, p_variables);
		}
	}

	void SemanticChecker::checkIfStatementInstruction(const std::filesystem::path& p_file, const std::shared_ptr<IfStatementInstruction>& p_instruction, std::unordered_map<std::string, SemanticChecker::Type*> p_variables, Type* returnType)
	{
		checkConditionInstruction(p_file, p_instruction->condition, p_variables);

		checkSymbolBodyInstruction(p_file, p_instruction->body, p_variables, returnType);

		for (const auto& elseBlockInstruction : p_instruction->elseBlocks)
		{
			if (elseBlockInstruction->condition != nullptr)
				checkConditionInstruction(p_file, elseBlockInstruction->condition, p_variables);

			checkSymbolBodyInstruction(p_file, elseBlockInstruction->body, p_variables, returnType);
		}
	}
}