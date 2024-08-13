#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkReturnInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ReturnInstruction>& p_instruction, std::unordered_map<std::string, SemanticChecker::Type*> p_variables, Type* p_returnType)
	{
		if (p_returnType == type("void"))
		{
			if (p_instruction->argument->elements.size() != 0)
				throw TokenBasedError(p_file, "Expected return type [void].", p_instruction->argument->mergedToken());
		}
		else
		{
			Type* expressionType = getExpressionType(p_file, p_instruction->argument, p_variables);

			if (p_returnType != expressionType && p_returnType->acceptedConversions.contains(expressionType) == false)
			{
				throw TokenBasedError(p_file, "No convertion from type [" + expressionType->name + "] to expected type [" + p_returnType->name + "]" + DEBUG_INFORMATION, p_instruction->argument->mergedToken());
			}
		}
	}
}