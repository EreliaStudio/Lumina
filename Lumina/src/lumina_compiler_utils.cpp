#include "lumina_compiler.hpp"

namespace Lumina
{
	std::string Compiler::namespacePrefix()
	{
		std::string result;

		for (const std::string& name : _namespaceNames)
		{
			result += name + "::";
		}

		return (result);
	}

	Compiler::Variable Compiler::composeVariable(const VariableDescriptor& p_descriptor)
	{
		Variable result;

		result.type = type(p_descriptor.type.value);
		result.name = p_descriptor.name.content;
		result.arraySizes = p_descriptor.arraySizes;

		if (result.type == nullptr)
		{
			throw TokenBasedError("Type : " + p_descriptor.type.value.content + " not found", p_descriptor.type.value);
		}

		return (result);
	}

	Compiler::Type Compiler::composeType(std::shared_ptr<BlockMetaToken> p_metaToken)
	{
		Type result;

		result.name = namespacePrefix() + p_metaToken->name.content;

		if (p_metaToken->type == MetaToken::Type::Attribute ||
			p_metaToken->type == MetaToken::Type::Constant)
		{
			result.name += "Type";
		}

		if (_type(result.name) != nullptr)
		{
			throw TokenBasedError("Type [" + result.name + "] already defined.", p_metaToken->name);
		}

		size_t cpuOffset = 0;
		size_t gpuOffset = 0;
		for (const auto& element : p_metaToken->elements)
		{
			if (result.contains(element.name.content) == true)
			{
				throw TokenBasedError("Attribute [" + element.name.content + "] already defined in [" + result.name + "] structure.", element.name);
			}

			Type::Element newElement;

			newElement.variable.name = element.name.content;
			newElement.variable.type = type(element.type.value);
			if (newElement.variable.type == _type("Texture"))
			{
				throw TokenBasedError("Texture can't be placed inside block.", element.name);
			}
			newElement.variable.arraySizes = element.arraySizes;

			size_t totalSize = 1;
			for (const auto& size : newElement.variable.arraySizes)
				totalSize *= size;

			size_t padding = 0;
			if (newElement.variable.type->gpuSize == 12)
				padding = 4;
			else if (newElement.variable.type->gpuSize >= 16)
				padding = (16 - (newElement.variable.type->gpuSize % 16)) % 16;

			if ((gpuOffset % 16) != 0)
			{
				size_t bytesLeft = 16 - (gpuOffset % 16);
				if (bytesLeft < newElement.variable.type->gpuSize)
				{
					gpuOffset += bytesLeft;
				}
			}

			newElement.gpuOffset = gpuOffset;
			gpuOffset += (newElement.variable.type->gpuSize + padding) * totalSize;

			newElement.cpuOffset = cpuOffset;
			cpuOffset += newElement.variable.type->cpuSize * totalSize;


			if (newElement.variable.type == nullptr)
			{
				throw TokenBasedError("Type [" + element.type.value.content + "] not found.", p_metaToken->name);
			}

			result.innerElements.push_back(newElement);
		}

		result.gpuSize = gpuOffset;
		result.cpuSize = cpuOffset;

		return (result);
	}

	Compiler::Variable Compiler::composeVariable(std::shared_ptr<BlockMetaToken> p_metaToken)
	{
		Type tmpType = composeType(p_metaToken);
		std::string name = namespacePrefix() + p_metaToken->name.content;

		addType(tmpType);

		Variable result;

		result.type = _type(tmpType.name);
		result.name = name;

		return (result);
	}
}