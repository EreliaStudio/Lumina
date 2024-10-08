#pragma once

#include <map>
#include <vector>

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

	struct TextureInfo
	{
		NameInfo name;
		ArraySizeInfo arraySizes;
	};

	struct ParameterInfo
	{
		TypeInfo type;
		bool isReference;
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
		std::vector<ParameterInfo> parameters;
		SymbolBodyInfo body;
	};

	struct OperatorInfo
	{
		ExpressionTypeInfo returnType;
		Lumina::Token opeType;
		std::vector<ParameterInfo> parameters;
		SymbolBodyInfo body;
	};

	struct BlockInfo
	{
		Lumina::Token name;
		std::vector<VariableInfo> attributes;
		std::map<std::string, std::vector<FunctionInfo>> methodInfos;
		std::map<std::string, std::vector<OperatorInfo>> operatorInfos;
	};

	struct NamespaceInfo
	{
		Lumina::Token name;
		std::vector<BlockInfo> structureBlocks;
		std::vector<BlockInfo> attributeBlocks;
		std::vector<BlockInfo> constantBlocks;

		std::vector<TextureInfo> textureInfos;

		std::map<std::string, std::vector<FunctionInfo>> functionInfos;

		std::vector<NamespaceInfo> nestedNamespaces;
	};

	struct PipelineFlowInfo
	{
		Lumina::Token input;
		Lumina::Token output;
		VariableInfo variable;
	};

	struct PipelinePassInfo
	{
		Lumina::Token name;
		SymbolBodyInfo body;
	};

	struct ShaderInfo
	{
		std::vector<PipelineFlowInfo> pipelineFlows;

		std::vector<PipelinePassInfo> pipelinePasses;

		NamespaceInfo anonymNamespace;
	};
}