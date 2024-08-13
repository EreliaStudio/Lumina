#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkWhileLoopInstruction(const std::filesystem::path& p_file, const std::shared_ptr<WhileLoopInstruction>& p_instruction, std::unordered_map<std::string, SemanticChecker::Type*> p_variables, Type* returnType)
	{
		checkConditionInstruction(p_file, p_instruction->condition, p_variables);

		checkSymbolBodyInstruction(p_file, p_instruction->body, p_variables, returnType);
	}
}