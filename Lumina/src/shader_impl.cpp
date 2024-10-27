#include "shader_impl.hpp"
#include <iostream>

namespace Lumina
{
	// Overload for VariableImpl
	std::ostream& operator<<(std::ostream& os, const VariableImpl& variable)
	{
		os << "Variable(name: " << variable.name << ", type: ";
		
		if (variable.type == nullptr)
		{
			os << "No type";
		}
		else
		{
			os << variable.type->name;
		}
		if (!variable.arraySize.empty())
		{
			os << ", arraySize: [";
			for (size_t i = 0; i < variable.arraySize.size(); ++i)
			{
				os << variable.arraySize[i];
				if (i < variable.arraySize.size() - 1)
					os << ", ";
			}
			os << "]";
		}
		os << ")";
		return os;
	}

	// Overload for TypeImpl
	std::ostream& operator<<(std::ostream& os, const TypeImpl& type)
	{
		os << "Type(name: " << type.name << ", attributes: [";
		for (size_t i = 0; i < type.attributes.size(); ++i)
		{
			os << type.attributes[i];
			if (i < type.attributes.size() - 1)
				os << ", ";
		}
		os << "])";
		return os;
	}

	// Overload for ExpressionType
	std::ostream& operator<<(std::ostream& os, const ExpressionType& exprType)
	{
		os << "ExpressionType(type: " << exprType.type;
		if (!exprType.arraySize.empty())
		{
			os << ", arraySize: [";
			for (size_t i = 0; i < exprType.arraySize.size(); ++i)
			{
				os << exprType.arraySize[i];
				if (i < exprType.arraySize.size() - 1)
					os << ", ";
			}
			os << "]";
		}
		os << ")";
		return os;
	}

	// Overload for ParameterImpl
	std::ostream& operator<<(std::ostream& os, const ParameterImpl& parameter)
	{
		os << "Parameter(name: " << parameter.name << ", type: " << parameter.type;
		if (parameter.isReference)
			os << "&";
		if (!parameter.arraySize.empty())
		{
			os << ", arraySize: [";
			for (size_t i = 0; i < parameter.arraySize.size(); ++i)
			{
				os << parameter.arraySize[i];
				if (i < parameter.arraySize.size() - 1)
					os << ", ";
			}
			os << "]";
		}
		os << ")";
		return os;
	}

	// Overload for FunctionBodyImpl
	std::ostream& operator<<(std::ostream& os, const FunctionBodyImpl& functionBody)
	{
		os << "FunctionBody(code: \"" << functionBody.code << "\")";
		return os;
	}

	// Overload for FunctionImpl
	std::ostream& operator<<(std::ostream& os, const FunctionImpl& function)
	{
		os << "Function(name: " << function.name << ", returnType: " << function.returnType << ", parameters: [";
		for (size_t i = 0; i < function.parameters.size(); ++i)
		{
			os << function.parameters[i];
			if (i < function.parameters.size() - 1)
				os << ", ";
		}
		os << "], body: " << function.body << ")";
		return os;
	}

	// Overload for PipelineFlowImpl
	std::ostream& operator<<(std::ostream& os, const PipelineFlowImpl& pipelineFlow)
	{
		os << "PipelineFlow(direction: ";
		if (pipelineFlow.direction == PipelineFlowImpl::Direction::In)
			os << "In";
		else
			os << "Out";
		os << ", variable: " << pipelineFlow.variable << ")";
		return os;
	}

	// Overload for ShaderImpl
	std::ostream& operator<<(std::ostream& os, const ShaderImpl& shader)
	{
		os << "ShaderImpl {\n";

		// Vertex Pipeline Flows
		os << "  VertexPipelineFlows: [\n";
		for (const auto& flow : shader.vertexPipelineFlows)
		{
			os << "    " << flow << "\n";
		}
		os << "  ]\n";

		// Fragment Pipeline Flows
		os << "  FragmentPipelineFlows: [\n";
		for (const auto& flow : shader.fragmentPipelineFlows)
		{
			os << "    " << flow << "\n";
		}
		os << "  ]\n";

		// Structures
		os << "  Structures: [\n";
		for (const auto& structure : shader.structures)
		{
			os << "    " << structure << "\n";
		}
		os << "  ]\n";

		// Attributes
		os << "  Attributes: [\n";
		for (const auto& attribute : shader.attributes)
		{
			os << "    " << attribute << "\n";
		}
		os << "  ]\n";

		// Constants
		os << "  Constants: [\n";
		for (const auto& constant : shader.constants)
		{
			os << "    " << constant << "\n";
		}
		os << "  ]\n";

		// Functions
		os << "  Functions: [\n";
		for (const auto& function : shader.functions)
		{
			os << "    " << function << "\n";
		}
		os << "  ]\n";

		// Textures
		os << "  Textures: [\n";
		for (const auto& texture : shader.textures)
		{
			os << "    " << texture << "\n";
		}
		os << "  ]\n";

		// Main Functions
		os << "  VertexMain: " << shader.vertexMain << "\n";
		os << "  FragmentMain: " << shader.fragmentMain << "\n";

		os << "}";
		return os;
	}
}
