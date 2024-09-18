#include "lumina_semantic_checker.hpp"

#include <regex>

namespace Lumina
{
	void SemanticChecker::checkAttributeInstruction(const std::filesystem::path& p_file, const std::shared_ptr<AttributeBlockInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();

		if (type(namespacePrefix + p_instruction->name.content) != nullptr ||
			_vertexPassVariables.contains(namespacePrefix + p_instruction->name.content) == true ||
			_fragmentPassVariables.contains(namespacePrefix + p_instruction->name.content) == true)
		{
			throwException(p_file, "Attribute [" + p_instruction->name.content + "] already define", p_instruction->name);
		}

		std::vector<Symbol>* symbolVerification = symbolArray(p_instruction->name.content);

		if (symbolVerification != nullptr)
		{
			throwException(p_file, "Code block name [" + p_instruction->name.content + "] is invalid : Name conflict with an existing symbol", p_instruction->name);
		}

		Type newAttribute;

		newAttribute.name = namespacePrefix + p_instruction->name.content;

		size_t currentCpuOffset = 0;
		size_t currentGpuOffset = 0;

		for (const auto& element : p_instruction->elements)
		{
			try
			{
				auto it = std::find_if(newAttribute.attributes.begin(), newAttribute.attributes.end(), [&, element](const Type::Attribute& attribute)
					{
						return attribute.name == element->name.content;
					});

				if (it != newAttribute.attributes.end())
				{
					throw TokenBasedError(p_file, "[" + element->name.content + "] already defined in [" + p_instruction->name.content + "]", p_instruction->name);
				}

				Token typeToken = Token::merge(element->type->tokens, Token::Type::Identifier);

				Type* attributeType = type(typeToken.content);

				if (attributeType == nullptr)
				{
					throw TokenBasedError(p_file, "Type [" + typeToken.content + "] not found", typeToken);
				}

				size_t attributeCpuSize = attributeType->cpuSize;
				size_t attributeGpuSize = attributeType->gpuSize;

				size_t alignment = std::min(attributeGpuSize, static_cast<size_t>(16));

				currentGpuOffset = alignOffset(currentGpuOffset, attributeGpuSize, alignment);

				newAttribute.attributes.push_back({
					attributeType,
					element->name.content,
					{ currentCpuOffset, attributeCpuSize },
					{ currentGpuOffset, attributeGpuSize }
					});

				currentCpuOffset += attributeCpuSize;
				currentGpuOffset += attributeGpuSize;
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
		}

		newAttribute.cpuSize = currentCpuOffset;
		newAttribute.gpuSize = currentGpuOffset;

		addAttribute(newAttribute);
	}

	void SemanticChecker::compileAttributeInstruction(const std::shared_ptr<AttributeBlockInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();

		std::string attributeName = namespacePrefix + p_instruction->name.content;
		std::string typeName = std::regex_replace(attributeName, std::regex("::"), "_");

		Type* attributeType = attribute(namespacePrefix + p_instruction->name.content);

		std::string codeContent = "layout(attribute) uniform " + typeName + "Type" + " {\n";
		std::string attributeContent = typeName + "Type " + attributeName + " " + std::to_string(attributeType->cpuSize) + " " + std::to_string(attributeType->gpuSize) + " {\n";

		insertUniformDefinition(attributeContent, 4, attributeType);

		for (const auto& element : p_instruction->elements)
		{
			Type* elementType = type(element->type->tokens);

			std::string elementName = std::regex_replace(elementType->name, std::regex("::"), "_");

			codeContent += "    " + elementName + " " + element->name.content;
			codeContent += ";\n";
		}
		codeContent += "} " + typeName + ";\n\n";
		attributeContent += "};\n";

		_result.sections.vertexShader += codeContent + "\n";
		_result.sections.fragmentShader += codeContent + "\n";
		_result.sections.attribute += attributeContent;
	}
}