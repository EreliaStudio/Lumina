#include "lumina_metatokens.hpp"
#include "lumina_descriptors.hpp"

namespace Lumina
{
	MetaToken::MetaToken(Type p_type) :
		type(p_type)
	{

	}

	PipelineFlowMetaToken::PipelineFlowMetaToken() :
		MetaToken(Type::PipelineFlow)
	{

	}

	BlockMetaToken::BlockMetaToken(Type p_type) :
		MetaToken(p_type)
	{

	}

	ConstantMetaToken::ConstantMetaToken() :
		BlockMetaToken(MetaToken::Type::Constant)
	{

	}

	AttributeMetaToken::AttributeMetaToken() :
		BlockMetaToken(MetaToken::Type::Attribute)
	{

	}

	StructureMetaToken::StructureMetaToken() :
		BlockMetaToken(MetaToken::Type::Structure)
	{

	}

	TextureMetaToken::TextureMetaToken() :
MetaToken(MetaToken::Type::Texture)
	{

	}

	FunctionMetaToken::FunctionMetaToken() :
		MetaToken(MetaToken::Type::Function)
	{

	}

	PipelineBodyMetaToken::PipelineBodyMetaToken() :
		MetaToken(MetaToken::Type::PipelineBody)
	{

	}

	NamespaceMetaToken::NamespaceMetaToken() :
		MetaToken(MetaToken::Type::Namespace)
	{

	}
}