#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkStructureInstruction(const std::filesystem::path& p_file, const std::shared_ptr<StructureBlockInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();

		if (type(namespacePrefix + p_instruction->name.content) != nullptr)
		{
			throwException(p_file, "Structure [" + p_instruction->name.content + "] already define", p_instruction->name);
		}

		std::vector<Symbol>* symbolVerification = symbolArray(p_instruction->name.content);

		if (symbolVerification != nullptr)
		{
			throwException(p_file, "Code block name [" + p_instruction->name.content + "] is invalid : Name conflict with an existing symbol", p_instruction->name);
		}

		Type newStructure;

		newStructure.name = namespacePrefix + p_instruction->name.content;

		for (const auto& element : p_instruction->elements)
		{
			try
			{
				auto it = std::find_if(newStructure.attributes.begin(), newStructure.attributes.end(), [&, element](const Type::Attribute& attribute)
					{
						return attribute.name == element->name.content;
					});

				if (it != newStructure.attributes.end())
				{
					throw TokenBasedError(p_file, "[" + element->name.content + "] already defined in [" + p_instruction->name.content + "]", p_instruction->name);
				}

				Token typeToken = Token::merge(element->type->tokens, Token::Type::Identifier);

				Type* attributeType = type(typeToken.content);

				if (attributeType == nullptr)
				{
					throw TokenBasedError(p_file, "Type [" + typeToken.content + "] not found", typeToken);
				}

				newStructure.attributes.push_back({ attributeType, element->name.content, element->nbElement });
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
		}

		addStructure(newStructure);
	}
}