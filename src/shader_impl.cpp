#include "shader_impl.hpp"
#include <iostream>
#include <sstream>

namespace Lumina
{
	void printArraySizes(std::ostream& os, const std::vector<size_t>& arraySize)
	{
		if (!arraySize.empty())
		{
			os << "[";
			for (size_t i = 0; i < arraySize.size(); ++i)
			{
				if (i > 0) os << ", ";
				os << arraySize[i];
			}
			os << "]";
		}
	}

	std::ostream& operator<<(std::ostream& os, const VariableImpl& variable)
	{
		os << variable.type.name << " " << variable.name;
		printArraySizes(os, variable.arraySizes);
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const TypeImpl& type)
	{
		os << "Type: " << type.name << "\n";

		if (!type.attributes.empty())
		{
			os << "	Attributes:\n";
			for (const auto& attr : type.attributes)
			{
				os << "		" << attr << "\n";
			}
		}

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const PipelinePassImpl& pipelineFlow)
	{
		os << "{\n";
		os << pipelineFlow.body;
		os << "}";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderImpl& shader)
	{
		os << "Available Types:\n";
		for (const auto& type : shader.structures)
		{
			os << "	" << type << "\n";
		}

		os << "\nGlobal Variables:\n";
		for (const auto& var : shader.constants)
		{
			os << "	" << var << "\n";
		}

		os << "\nVertex Variables:\n";
		for (const auto& var : shader.vertexPipelineFlows)
		{
			os << "	" << var << "\n";
		}

		os << "\nFragment Variables:\n";
		for (const auto& var : shader.fragmentPipelineFlows)
		{
			os << "	" << var << "\n";
		}

		os << "\nOutput Variables:\n";
		for (const auto& var : shader.outputPipelineFlows)
		{
			os << "	" << var << "\n";
		}

		os << "\nAttributes:\n";
		for (const auto& attr : shader.attributes)
		{
			os << "	" << attr << "\n";
		}

		os << "\nTextures:\n";
		for (const auto& texture : shader.textures)
		{
			os << "	" << texture << "\n";
		}

		os << "\nFunctions:\n";
		for (const auto& func : shader.functions)
		{
			os << func << "\n";
		}

		os << "\nVertexMain:\n";
		os << shader.vertexPipelinePass;

		os << "\nFragmentMain:\n";
		os << shader.fragmentPipelinePass;

		return os;
	}
}
