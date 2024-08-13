#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkNumberExpressionValueInstruction(const std::filesystem::path& p_file, const std::shared_ptr<NumberExpressionValueInstruction>& p_instruction, Type* p_expectedType)
	{
		Type* numberType = getNumberExpressionElementType(p_file, p_instruction);

		if (p_expectedType != numberType && p_expectedType->acceptedConversions.contains(numberType) == false)
		{
			throw TokenBasedError(p_file, "No convertion from type [" + numberType->name + "] to expected type [" + p_expectedType->name + "]" + DEBUG_INFORMATION, p_instruction->mergedToken());
		}
	}
}