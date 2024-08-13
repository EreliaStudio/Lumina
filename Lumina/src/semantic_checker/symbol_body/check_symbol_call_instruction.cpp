#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkSymbolCallInstruction(const std::filesystem::path& p_file, const std::shared_ptr<SymbolCallInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables, Type* p_expectedType)
	{
		Token nameToken = Token::merge(p_instruction->name->tokens, Token::Type::Identifier);
		std::vector<Symbol>* targetSymbolArray = symbolArray(nameToken.content);

		if (targetSymbolArray == nullptr || targetSymbolArray->size() == 0)
		{
			throw TokenBasedError(p_file, "No symbol [" + nameToken.content + "] found" + DEBUG_INFORMATION, nameToken);
		}

		Type* symbolReturnType = targetSymbolArray->front().returnType;

		if (p_instruction->resultAccessor != nullptr)
		{
			for (size_t i = 0; i < p_instruction->resultAccessor->tokens.size(); i++)
			{
				std::string expectedAttributeName = p_instruction->resultAccessor->tokens[i].content;
				const auto& it = std::find_if(symbolReturnType->attributes.begin(), symbolReturnType->attributes.end(), [expectedAttributeName](const Type::Attribute& p_attribute) {
					return (p_attribute.name == expectedAttributeName);
					});

				if (it == symbolReturnType->attributes.end())
				{
					throw TokenBasedError(p_file, "Attribute [" + expectedAttributeName + "] not found in [" + symbolReturnType->name + "] type" + DEBUG_INFORMATION, p_instruction->resultAccessor->tokens[i]);
				}

				symbolReturnType = it->type;
			}
		}

		if (p_expectedType != symbolReturnType && p_expectedType->acceptedConversions.contains(symbolReturnType) == false)
		{
			throw TokenBasedError(p_file, "No convertion from type [" + symbolReturnType->name + "] to expected type [" + p_expectedType->name + "]" + DEBUG_INFORMATION, nameToken);
		}

		bool validated = false;
		for (const auto& symbol : *targetSymbolArray)
		{
			if (symbol.parameters.size() == p_instruction->arguments.size())
			{
				bool error = false;
				for (size_t i = 0; i < p_instruction->arguments.size(); i++)
				{
					const Symbol::Parameter& param = symbol.parameters[i];

					Type* parameterType = getExpressionType(p_file, p_instruction->arguments[i], p_variables);

					if (parameterType != param.type && param.type->acceptedConversions.contains(parameterType) == false)
					{
						error = true;
					}
				}

				if (error == false)
				{
					validated = true;
					break;
				}
			}
			if (validated == true)
				break;
		}

		if (validated == false)
		{
			std::string error = "No symbol [" + nameToken.content + "] found with matching types [";
			for (size_t i = 0; i < p_instruction->arguments.size(); i++)
			{
				if (i != 0)
					error += ", ";
				Type* paramType = getExpressionType(p_file, p_instruction->arguments[i], p_variables);
				if (paramType == nullptr)
					error += "Unknow type";
				else
					error += paramType->name;
			}
			error += "]";
			throw TokenBasedError(p_file, error + DEBUG_INFORMATION, nameToken);
		}

		for (const auto& argument : p_instruction->arguments)
		{
			checkExpressionInstruction(p_file, argument, p_variables, getExpressionType(p_file, argument, p_variables));
		}
	}
}