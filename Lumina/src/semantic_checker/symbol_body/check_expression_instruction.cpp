#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkExpression(const std::filesystem::path& p_file, const std::shared_ptr<Expression>& p_instruction, const std::unordered_map<std::string, Variable> p_variables, SemanticChecker::Type* p_expectedType)
	{
		for (const auto& element : p_instruction->elements)
		{
			try
			{
				switch (element->type)
				{
				case Instruction::Type::BoolExpressionValue:
				{
					break;
				}
				case Instruction::Type::VariableExpressionValue:
				{
					checkVariableExpressionValueInstruction(p_file, static_pointer_cast<VariableExpressionValueInstruction>(element), p_variables, p_expectedType);
					break;
				}
				case Instruction::Type::NumberExpressionValue:
				{
					checkNumberExpressionValueInstruction(p_file, static_pointer_cast<NumberExpressionValueInstruction>(element), p_expectedType);
					break;
				}
				case Instruction::Type::SymbolCall:
				{
					checkSymbolCallInstruction(p_file, static_pointer_cast<SymbolCallInstruction>(element), p_variables, p_expectedType);
					break;
				}
				case Instruction::Type::OperatorExpression:
				{
					break;
				}
				case Instruction::Type::ComparatorOperatorExpression:
				{
					break;
				}
				default:
				{
					throw TokenBasedError(p_file, "Unexpected expression instruction type : " + ::to_string(element->type) + DEBUG_INFORMATION, element->mergedToken());
				}
				}
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
		}
	}
}