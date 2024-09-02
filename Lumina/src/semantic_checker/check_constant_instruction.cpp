#include "lumina_semantic_checker.hpp"

#include <regex>

namespace Lumina
{
	void SemanticChecker::checkConstantInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ConstantBlockInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();

		if (type(namespacePrefix + p_instruction->name.content) != nullptr ||
			_vertexPassVariables.contains(namespacePrefix + p_instruction->name.content) == true ||
			_fragmentPassVariables.contains(namespacePrefix + p_instruction->name.content) == true)
		{
			throwException(p_file, "Constant [" + p_instruction->name.content + "] already define", p_instruction->name);
		}


		std::vector<Symbol>* symbolVerification = symbolArray(p_instruction->name.content);

		if (symbolVerification != nullptr)
		{
			throwException(p_file, "Code block name [" + p_instruction->name.content + "] is invalid : Name conflict with an existing symbol", p_instruction->name);
		}

		Type newConstant;

		newConstant.name = namespacePrefix + p_instruction->name.content;

		size_t currentCpuOffset = 0;
		size_t currentGpuOffset = 0;

		for (const auto& element : p_instruction->elements)
		{
			try
			{
				auto it = std::find_if(newConstant.attributes.begin(), newConstant.attributes.end(), [&, element](const Type::Attribute& constant)
					{
						return constant.name == element->name.content;
					});

				if (it != newConstant.attributes.end())
				{
					throw TokenBasedError(p_file, "[" + element->name.content + "] already defined in [" + p_instruction->name.content + "]", p_instruction->name);
				}

				Token typeToken = Token::merge(element->type->tokens, Token::Type::Identifier);

				Type* constantType = type(typeToken.content);

				if (constantType == nullptr)
				{
					throw TokenBasedError(p_file, "Type [" + typeToken.content + "] not found", typeToken);
				}

				size_t attributeCpuSize = constantType->cpuSize * (element->nbElement > 1 ? element->nbElement : 1);
				size_t attributeGpuSize = constantType->gpuSize * (element->nbElement > 1 ? element->nbElement : 1);

				size_t alignment = std::min(attributeGpuSize, static_cast<size_t>(16));

				currentGpuOffset = alignOffset(currentGpuOffset, attributeGpuSize, alignment);

				newConstant.attributes.push_back({
					constantType,
					element->name.content,
					element->nbElement,
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

		newConstant.cpuSize = currentCpuOffset;
		newConstant.gpuSize = currentGpuOffset;

		addConstant(newConstant);
	}

	void SemanticChecker::compileConstantInstruction(const std::shared_ptr<ConstantBlockInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();

		std::string constantName = namespacePrefix + p_instruction->name.content;
		std::string typeName = std::regex_replace(constantName, std::regex("::"), "_");

		Type* constantType = constant(namespacePrefix + p_instruction->name.content);

		std::string codeContent = "layout(constant) uniform " + typeName + "Type" + "{\n";
		std::string constantContent = typeName + "Type " + constantName + " " + std::to_string(constantType->cpuSize) + " " + std::to_string(constantType->gpuSize) + " {\n";

		insertUniformDefinition(constantContent, 4, constantType);

		for (const auto& element : p_instruction->elements)
		{
			Type* elementType = type(element->type->tokens);

			std::string elementName = std::regex_replace(elementType->name, std::regex("::"), "_");

			codeContent += "    " + elementName + " " + element->name.content;
			if (element->nbElement != 0)
				codeContent += "[" + std::to_string(element->nbElement) + "]";
			codeContent += ";\n";
		}
		codeContent += "} " + typeName + ";\n";
		constantContent += "};\n";

		_result.sections.vertexShader += codeContent + "\n";
		_result.sections.fragmentShader += codeContent + "\n";
		_result.sections.constant += constantContent;
	}
}