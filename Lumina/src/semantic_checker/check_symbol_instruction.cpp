#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkSymbolInstruction(const std::filesystem::path& p_file, const std::shared_ptr<SymbolInstruction>& p_instruction)
	{
		Symbol newSymbol;

		auto returnTypeToken = Token::merge(p_instruction->returnType->tokens, Token::Type::Identifier);

		newSymbol.returnType = type(returnTypeToken.content);
		if (newSymbol.returnType == nullptr)
		{
			throwException(p_file, "Return type [" + returnTypeToken.content + "] not found", returnTypeToken);
		}

		Type* typeVerification = type(p_instruction->name.content);

		if (typeVerification != nullptr)
		{
			throwException(p_file, "Symbol name [" + p_instruction->name.content + "] is invalid : Name conflict with an existing code block.", p_instruction->name);
		}

		newSymbol.name = createNamespacePrefix() + p_instruction->name.content;
		for (const auto& parameter : p_instruction->parameters)
		{
			auto parameterTypeToken = Token::merge(parameter->type->tokens, Token::Type::Identifier);
			Type* parameterType = type(parameterTypeToken.content);

			if (parameterType == nullptr)
			{
				throwException(p_file, "Parameter type [" + parameterTypeToken.content + "] not found", returnTypeToken);
			}

			auto it = std::find_if(newSymbol.parameters.begin(), newSymbol.parameters.end(), [parameter](const Symbol::Parameter& param) {
				return (param.name == parameter->name.content);
				});
			if (it != newSymbol.parameters.end())
			{
				throwException(p_file, "Parameter [" + parameter->name.content + "] already declared in function [" + newSymbol.name + "]", returnTypeToken);
			}

			newSymbol.parameters.push_back({ parameter->name.content, parameterType });
		}

		const std::vector<Symbol>& tmpSymbolArray = _symbols[createNamespacePrefix() + newSymbol.name];

		if (tmpSymbolArray.size() != 0 &&
			tmpSymbolArray.back().returnType != newSymbol.returnType)
		{
			throw TokenBasedError(p_file, "Symbol [" + returnTypeToken.content + "] already define with another return type.", returnTypeToken);
		}

		for (const auto& symbol : tmpSymbolArray)
		{
			if (symbol.parameters.size() == newSymbol.parameters.size())
			{
				bool isSame = true;

				for (size_t i = 0; i < symbol.parameters.size(); i++)
				{
					if (symbol.parameters[i].type != newSymbol.parameters[i].type)
					{
						isSame = false;
					}
				}

				if (isSame == true)
				{
					std::vector<Token> tokens = { returnTypeToken, p_instruction->parameters.back()->name };
					tokens.back().content += ")";
					Token mergedToken = Token::merge(tokens, Token::Type::Identifier);

					throw TokenBasedError(p_file, "Symbol [" + mergedToken.content + "] already define with the same parameter type.", mergedToken);
				}
			}
		}

		addSymbol(newSymbol);

		std::unordered_map<std::string, Type*> functionVariables;

		for (const auto attribute : _attributes)
		{
			functionVariables[attribute->name] = attribute;
		}

		for (const auto constant : _constants)
		{
			functionVariables[constant->name] = constant;
		}

		for (const auto& [name, type] : newSymbol.parameters)
		{
			functionVariables[name] = type;
		}

		checkSymbolBodyInstruction(p_file, p_instruction->body, functionVariables, newSymbol.returnType);
	}
}