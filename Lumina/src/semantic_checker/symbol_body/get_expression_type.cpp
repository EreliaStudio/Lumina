#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	SemanticChecker::Type* SemanticChecker::getExpressionType(const std::filesystem::path& p_file, const std::shared_ptr<ExpressionInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables)
	{
		Type* result = nullptr;

		for (const auto& element : p_instruction->elements)
		{
			switch (element->type)
			{
			case Instruction::Type::BoolExpressionValue:
			case Instruction::Type::NumberExpressionValue:
			case Instruction::Type::VariableExpressionValue:
			case Instruction::Type::SymbolCall:
			{
				Type* elementType = getExpressionElementType(p_file, element, p_variables);

				if (elementType == nullptr)
					throw TokenBasedError(p_file, "No type found for token [" + element->mergedToken().content + "]", element->mergedToken());

				if (result != nullptr && result != elementType && result->acceptedConversions.contains(elementType) == false)
				{
					throw TokenBasedError(p_file, "No convertion from type [" + elementType->name + "] to expected type [" + result->name + "]" + DEBUG_INFORMATION, element->mergedToken());
				}
				result = elementType;
				break;
			}
			case Instruction::Type::OperatorExpression:
			{
				if (result->operators.contains(element->mergedToken().content) == false)
				{
					throw TokenBasedError(p_file, "Type [" + result->name + "] doesn't support operator [" + element->mergedToken().content + "]" + DEBUG_INFORMATION, element->mergedToken());
				}
				break;
			}
			case Instruction::Type::ComparatorOperatorExpression:
			{
				if (result->comparaisonOperators.contains(element->mergedToken().content) == false)
				{
					throw TokenBasedError(p_file, "Type [" + result->name + "] doesn't support comparator operator [" + element->mergedToken().content + "]" + DEBUG_INFORMATION, element->mergedToken());
				}
				break;
			}
			default:
			{
				throw TokenBasedError(p_file, "Unexpected expression instruction type : " + ::to_string(element->type) + DEBUG_INFORMATION, element->mergedToken());
			}
			}
			
		}


		if (result == nullptr)
			return (type("void"));
		return (result);
	}
}