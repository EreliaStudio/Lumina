#pragma once

#include "lumina_token.hpp"
#include "lumina_descriptors.hpp"
#include "lumina_instruction.hpp"

namespace Lumina
{
	struct MetaToken
	{
		enum class Type
		{
			Unknow,
			Include,
			PipelineFlow,
			PipelineBody,
			Constant,
			Attribute,
			Structure,
			Texture,
			Function,
			Namespace
		};

		Type type;

		MetaToken(Type p_type);
	};

	struct PipelineFlowMetaToken : public MetaToken
	{
		Lumina::Token inputFlow;
		Lumina::Token outputFlow;
		VariableDescriptor variableDescriptor;

		PipelineFlowMetaToken();
	};

	struct BlockMetaToken : public MetaToken
	{
		Lumina::Token name;
		std::vector<VariableDescriptor> elements;

		BlockMetaToken(Type p_type);
	};

	struct ConstantMetaToken : public BlockMetaToken
	{
		ConstantMetaToken();
	};

	struct AttributeMetaToken : public BlockMetaToken
	{
		AttributeMetaToken();
	};

	struct StructureMetaToken : public BlockMetaToken
	{
		StructureMetaToken();
	};

	struct TextureMetaToken : public MetaToken
	{
		Lumina::Token name;

		TextureMetaToken();
	};

	struct FunctionMetaToken : public MetaToken
	{
		ReturnTypeDescriptor returnType;
		Lumina::Token name;
		std::vector<VariableDescriptor> parameters;
		SymbolBody body;

		FunctionMetaToken();
	};

	struct PipelineBodyMetaToken : public MetaToken
	{
		Lumina::Token type;
		SymbolBody body;

		PipelineBodyMetaToken();
	};

	struct NamespaceMetaToken : public MetaToken
	{
		Lumina::Token name;
		std::vector<std::shared_ptr<MetaToken>> innerMetaTokens;

		NamespaceMetaToken();
	};
}