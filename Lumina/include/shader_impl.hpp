#pragma once

#include <string>
#include <set>
#include <vector>

namespace Lumina
{
	struct VariableImpl;

	struct TypeImpl
	{
		std::string name = "NoName";
		std::vector<VariableImpl> attributes;
	};

	struct VariableImpl
	{
		TypeImpl type;
		std::string name;
		std::vector<size_t> arraySize;
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

	struct FunctionBodyImpl
	{
		std::string code;
	}; 

	struct FunctionImpl
	{
		ExpressionTypeImpl returnType;
		std::string name;
		std::vector<ParameterImpl> parameters;

		FunctionBodyImpl body;
	};

	struct ShaderImpl
	{
		std::vector<PipelineFlowImpl> vertexPipelineFlows;
		std::vector<PipelineFlowImpl> fragmentPipelineFlows;

		std::vector<TypeImpl> structures;
		std::vector<TypeImpl> attributes;
		std::vector<TypeImpl> constants;

		std::vector<FunctionImpl> functions;

		std::vector<VariableImpl> textures;

		FunctionImpl vertexMain;
		FunctionImpl fragmentMain;
	};

	std::ostream& operator<<(std::ostream& os, const ShaderImpl& shader);
}