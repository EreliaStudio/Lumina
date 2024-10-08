#pragma once

#include <map>
#include <set>

#include "token.hpp"

namespace Lumina
{
	struct TypeInfo
	{
		std::vector<Lumina::Token> nspace;
		Lumina::Token value;
	};

	struct NameInfo
	{
		Lumina::Token value;
	};

	struct ArraySizeInfo
	{
		std::vector<Lumina::Token> dims;
	};

	struct VariableInfo
	{
		TypeInfo type;
		NameInfo name;
		ArraySizeInfo arraySizes;
	};

	struct SymbolBodyInfo
	{

	};

	struct ExpressionTypeInfo
	{
		TypeInfo type;
		ArraySizeInfo arraySizes;
	};

	struct FunctionInfo
	{
		ExpressionTypeInfo returnType;
		Lumina::Token name;
		std::vector<VariableInfo> parameters;
		SymbolBodyInfo body;
	};

	struct OperatorInfo
	{
		ExpressionTypeInfo returnType;
		Lumina::Token opeType;
		std::vector<VariableInfo> parameters;
		SymbolBodyInfo body;
	};

	struct BlockInfo
	{
		Lumina::Token name;
		std::vector<VariableInfo> attributes;
		std::map<std::string, std::vector<FunctionInfo>> methodInfos;
		std::map<std::string, std::vector<FunctionInfo>> operatorInfos;
	};

	struct PipelinePassInfo
	{
		Lumina::Token name;
		SymbolBodyInfo body;
	};

	struct NamespaceInfo
	{
		std::vector<BlockInfo> structureBlocks;
		std::vector<BlockInfo> attributeBlocks;
		std::vector<BlockInfo> constantBlocks;

		std::map<std::string, std::vector<FunctionInfo>> functionInfos;

		std::vector<NamespaceInfo> nestedNamespaces;
	};

	struct ShaderInfo
	{
		std::vector<VariableInfo> vertexPipelineFlows;
		std::vector<VariableInfo> fragmentPipelineFlows;
		std::vector<VariableInfo> outputPipelineFlows;

		PipelinePassInfo vertexPass;
		PipelinePassInfo fragmentPass;

		NamespaceInfo anonymNamespace;
	};
}