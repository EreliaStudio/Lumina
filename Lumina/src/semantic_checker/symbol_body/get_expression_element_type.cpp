#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	SemanticChecker::Type* SemanticChecker::getNumberExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<NumberExpressionValueInstruction>& p_instruction)
	{
		return (type("float"));
	}

	SemanticChecker::Type* SemanticChecker::getVariableExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<VariableExpressionValueInstruction>& p_instruction, std::unordered_map<std::string, Variable> p_variables)
	{
		size_t index = 1; 
		std::string variableName = p_instruction->tokens[0].content;

		while (p_instruction->tokens.size() >= index + 1 && p_instruction->tokens[index].type == Token::Type::NamespaceSeparator)
		{
			if (index != 0)
				variableName += "::";
			variableName += p_instruction->tokens[index + 1].content;
			index += 2;
		}

		if (p_variables.contains(variableName) == false)
		{
			throw TokenBasedError(p_file, "Variable [" + variableName + "] Doesn't exist." + DEBUG_INFORMATION, p_instruction->tokens[0]);
		}

		Variable variable = p_variables.at(variableName);
		Type* result = variable.type;

		for (size_t i = index; i < p_instruction->tokens.size(); i++)
		{
			std::string expectedAttributeName = p_instruction->tokens[i].content;
			const auto& it = std::find_if(result->attributes.begin(), result->attributes.end(), [expectedAttributeName](const Type::Attribute& p_attribute) {
				return (p_attribute.name == expectedAttributeName);
				});

			if (it == result->attributes.end())
			{
				throw TokenBasedError(p_file, "Attribute [" + expectedAttributeName + "] not found in [" + result->name + "] type" + DEBUG_INFORMATION, p_instruction->tokens[i]);
			}

			result = it->type;
		}

		if (p_instruction->arrayAccessorExpression != nullptr)
		{
			std::cout << "Accessor value : " << p_instruction->arrayAccessorExpression->mergedToken().content << std::endl;
		}

		return (result);
	}

	SemanticChecker::Type* SemanticChecker::getSymbolCallExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<ExpressionElementInstruction>& p_instruction)
	{
		std::shared_ptr<SymbolCallInstruction> value = static_pointer_cast<SymbolCallInstruction>(p_instruction);

		Token nameToken = Token::merge(value->name->tokens, Token::Type::Identifier);
		std::vector<Symbol>* targetSymbolArray = symbolArray(nameToken.content);

		if (targetSymbolArray == nullptr || targetSymbolArray->size() == 0)
		{
			throw TokenBasedError(p_file, "No symbol [" + nameToken.content + "] found" + DEBUG_INFORMATION, nameToken);
		}

		Type* result = targetSymbolArray->back().returnType;

		if (value->resultAccessor != nullptr)
		{
			for (size_t i = 0; i < value->resultAccessor->tokens.size(); i++)
			{
				std::string expectedAttributeName = value->resultAccessor->tokens[i].content;
				const auto& it = std::find_if(result->attributes.begin(), result->attributes.end(), [expectedAttributeName](const Type::Attribute& p_attribute) {
					return (p_attribute.name == expectedAttributeName);
					});

				if (it == result->attributes.end())
				{
					throw TokenBasedError(p_file, "Attribute [" + expectedAttributeName + "] not found in [" + result->name + "] type" + DEBUG_INFORMATION, value->resultAccessor->tokens[i]);
				}

				result = it->type;
			}
		}

		return (result);
	}

	SemanticChecker::Type* SemanticChecker::getExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<ExpressionElementInstruction>& p_instruction, std::unordered_map<std::string, Variable> p_variables)
	{
		switch (p_instruction->type)
		{
		case Instruction::Type::BoolExpressionValue:
		{
			return (type("bool"));
		}
		case Instruction::Type::NumberExpressionValue:
		{
			return (getNumberExpressionElementType(p_file, static_pointer_cast<NumberExpressionValueInstruction>(p_instruction)));
		}
		case Instruction::Type::VariableExpressionValue:
		{
			return (getVariableExpressionElementType(p_file, static_pointer_cast<VariableExpressionValueInstruction>(p_instruction), p_variables));
		}
		case Instruction::Type::SymbolCall:
		{
			return (getSymbolCallExpressionElementType(p_file, static_pointer_cast<SymbolCallInstruction>(p_instruction)));
		}
		default:
			return (type("void"));
		}
	}
}