#include "lumina_compiler.hpp"

namespace Lumina
{
	void Compiler::compilePipelineFlow(std::shared_ptr<PipelineFlowMetaToken> p_metaToken)
	{
		Variable newVariable = composeVariable(p_metaToken->variableDescriptor);

		if (p_metaToken->inputFlow == "Input")
		{
			if (p_metaToken->outputFlow == "VertexPass")
			{
				_result.value.inputLayouts += std::to_string(nbVertexLayout) + " " + newVariable.type->name + "\n";

				_result.value.vertexShaderCode += "layout(location = " + std::to_string(nbVertexLayout) + ") in " + newVariable.type->name + " " + newVariable.name + ";\n\n";

				_vertexVariables.push_back(newVariable);

				nbVertexLayout++;
			}
			else
			{
				throw TokenBasedError("Invalid pipeline flow ouput token. Only \"VertexPass\" is valid with \"Input\" input token.", p_metaToken->outputFlow);
			}
		}
		else if (p_metaToken->inputFlow == "VertexPass")
		{
			if (p_metaToken->outputFlow == "FragmentPass")
			{
				_result.value.vertexShaderCode += "layout(location = " + std::to_string(nbFragmentLayout) + ") out " + newVariable.type->name + " " + newVariable.name + ";\n\n";
				_result.value.fragmentShaderCode += "layout(location = " + std::to_string(nbFragmentLayout) + ") in " + newVariable.type->name + " " + newVariable.name + ";\n\n";

				_vertexVariables.push_back(newVariable);
				_fragmentVariables.push_back(newVariable);

				nbFragmentLayout++;
			}
			else
			{
				throw TokenBasedError("Invalid pipeline flow ouput token. Only \"FragmentPass\" is valid with \"VertexPass\" input token.", p_metaToken->outputFlow);
			}
		}
		else if (p_metaToken->inputFlow == "FragmentPass")
		{
			if (p_metaToken->outputFlow == "Output")
			{
				_result.value.outputLayouts += std::to_string(nbOutputLayout) + " " + newVariable.type->name + "\n\n";
				_result.value.fragmentShaderCode += "layout(location = " + std::to_string(nbOutputLayout) + ") out " + newVariable.type->name + " " + newVariable.name + ";\n\n";

				_fragmentVariables.push_back(newVariable);

				nbOutputLayout++;
			}
			else
			{
				throw TokenBasedError("Invalid pipeline flow ouput token. Only \"Output\" is valid with \"FragmentPass\" input token.", p_metaToken->outputFlow);
			}
		}
		else
		{
			throw TokenBasedError("Invalid pipeline flow input token. Only \"Input\", \"VertexPass\" and \"FragmentPass\" are valid pipeline flow input.", p_metaToken->inputFlow);
		}
	}
}