#pragma once

#include <string>
#include <set>
#include <vector>

namespace Lumina
{
	struct VariableImpl;

	struct TypeImpl
	{
		std::string name = "UnknownTypeName";
		std::vector<VariableImpl> attributes;

		bool operator < (const TypeImpl& p_other) const
		{
			return (name < p_other.name);
		}
	};

	struct VariableImpl
	{
		TypeImpl type;
		std::string name;
		std::vector<size_t> arraySizes;
	};

	struct PipelineFlowImpl
	{
		enum class Direction
		{
			In,
			Out
		};

		Direction direction;
		VariableImpl variable;
	};

	struct ExpressionTypeImpl
	{
		TypeImpl type;
		std::vector<size_t> arraySize;
	};

	struct ParameterImpl
	{
		TypeImpl type;
		bool isReference;
		std::string name;
		std::vector<size_t> arraySize;
	};

	struct SymbolBodyImpl
	{
		std::string code;
	}; 

	struct FunctionImpl
	{
		bool isPrototype;
		ExpressionTypeImpl returnType;
		std::string name;
		std::vector<ParameterImpl> parameters;

		SymbolBodyImpl body;
	};

	struct PipelinePassImpl
	{
		bool isDefined = false;
		SymbolBodyImpl body;
	};

	struct ShaderImpl
	{
		std::vector<PipelineFlowImpl> vertexPipelineFlows;
		std::vector<PipelineFlowImpl> fragmentPipelineFlows;
		std::vector<PipelineFlowImpl> outputPipelineFlows;

		std::vector<TypeImpl> structures;
		std::vector<TypeImpl> attributes;
		std::vector<TypeImpl> constants;

		std::vector<FunctionImpl> functions;

		std::vector<VariableImpl> textures;

		PipelinePassImpl vertexPipelinePass;
		PipelinePassImpl fragmentPipelinePass;
	};

	std::ostream& operator<<(std::ostream& os, const ShaderImpl& shader);
}