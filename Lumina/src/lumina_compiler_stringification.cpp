#include "lumina_compiler.hpp"

namespace Lumina
{
	std::string Compiler::composeBlockCode(const BlockType& p_blockType, const Variable& p_variable)
	{
		std::string result = "";

		switch (p_blockType)
		{
		case BlockType::Constant:
		{
			result += "layout (constant) uniform ";
			break;
		}
		case BlockType::Attribute:
		{
			result += "layout (attribute) uniform ";
			break;
		}
		case BlockType::Structure:
		{
			result += "struct ";
			break;
		}
		}

		result += namespacePrefix() + p_variable.type->name;

		result += " {\n";
		for (const auto& element : p_variable.type->innerElements)
		{
			result += "    " + element.variable.type->name + " " + element.variable.name;
			for (const auto& size : element.variable.arraySizes)
			{
				result += "[" + std::to_string(size) + "]";
			}
			result += ";\n";
		}
		result += "}";

		if (p_blockType != BlockType::Structure)
		{
			result += " " + namespacePrefix() + p_variable.name;
		}

		result += ";\n\n";

		return (result);
	}

	void Compiler::insertElement(std::string& p_stringToFill, const Compiler::Type::Element& p_elementToInsert, size_t p_nbSpace)
	{
		p_stringToFill += std::string(p_nbSpace, ' ') + p_elementToInsert.variable.name + " " + std::to_string(p_elementToInsert.cpuOffset) + " " + std::to_string(p_elementToInsert.variable.type->cpuSize) + " " + std::to_string(p_elementToInsert.gpuOffset) + " " + std::to_string(p_elementToInsert.variable.type->gpuSize);
		if (p_elementToInsert.variable.type->innerElements.size() == 0)
		{
			p_stringToFill += " {}";
		}
		else
		{
			p_stringToFill += " {\n";
			for (const auto& innerElement : p_elementToInsert.variable.type->innerElements)
			{
				insertElement(p_stringToFill, innerElement, p_nbSpace + 4);
			}
			p_stringToFill += std::string(p_nbSpace, ' ') + "}";
		}
		if (p_elementToInsert.variable.arraySizes.size() != 0)
		{
			size_t bufferSize = 1;
			for (const auto& size : p_elementToInsert.variable.arraySizes)
				bufferSize *= size;
			size_t padding = 0;
			if (p_elementToInsert.variable.type->gpuSize == 12)
				padding = 4;
			else if (p_elementToInsert.variable.type->gpuSize >= 16)
				padding = (16 - (p_elementToInsert.variable.type->gpuSize % 16)) % 16;
			p_stringToFill += " ";
			for (size_t i = 0; i < p_elementToInsert.variable.arraySizes.size(); i++)
			{
				if (i != 0)
					p_stringToFill += "x";
				p_stringToFill += std::to_string(p_elementToInsert.variable.arraySizes[i]);
			}
			p_stringToFill += " " + std::to_string(bufferSize) + " " + std::to_string(padding);
		}
		p_stringToFill += "\n";
	}

	std::string Compiler::composeDataDescriptor(const Compiler::Variable& p_variable)
	{
		std::string result = p_variable.type->name + " " + p_variable.name + " " + std::to_string(p_variable.type->cpuSize) + " " + std::to_string(p_variable.type->gpuSize) + " {\n";
		for (const auto& element : p_variable.type->innerElements)
		{
			insertElement(result, element, 4);
		}
		result += "}\n";

		return (result);
	}
}