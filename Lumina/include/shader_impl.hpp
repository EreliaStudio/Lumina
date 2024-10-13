#pragma once

namespace Lumina
{
	struct VariableImpl
	{
		std::string type;
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

	struct BlockImpl
	{
		std::string name;
		std::vector<VariableImpl> variables;
	};

	struct ExpressionType
	{
		std::string type;
		std::vector<size_t> arraySize;
	};

	struct ParameterImpl
	{
		std::string type;
		bool isReference;
		std::string name;
		std::vector<size_t> arraySize;
	};

	struct FunctionBodyImpl
	{
		std::vector<std::string> instructions;
	}; 

	struct FunctionImpl
	{
		ExpressionType returnType;
		std::string name;
		std::vector<ParameterImpl> parameters;

		FunctionBodyImpl body;
	};

	struct ShaderImpl
	{
		std::vector<PipelineFlowImpl> vertexPipelineFlows;
		std::vector<PipelineFlowImpl> fragmentPipelineFlows;

		std::vector<BlockImpl> structures;
		std::vector<BlockImpl> attributes;
		std::vector<BlockImpl> constants;

		std::vector<FunctionImpl> functions;

		std::vector<VariableImpl> textures;

		FunctionImpl vertexMain;
		FunctionImpl fragmentMain;
	};
}