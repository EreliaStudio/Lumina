#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkVariableExpressionValueInstruction(const std::filesystem::path& p_file, const std::shared_ptr<VariableExpressionValueInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables, Type* p_expectedType)
	{
		Token variableToken = Token::merge(p_instruction->tokens, Token::Type::Identifier);
		Type* variableType = getExpressionElementType(p_file, static_pointer_cast<ExpressionElementInstruction>(p_instruction), p_variables);

		if (p_expectedType != variableType && p_expectedType->acceptedConversions.contains(variableType) == false)
		{
			throw TokenBasedError(p_file, "No convertion from type [" + variableType->name + "] to expected type [" + p_expectedType->name + "]" + DEBUG_INFORMATION, variableToken);
		}
	}
}