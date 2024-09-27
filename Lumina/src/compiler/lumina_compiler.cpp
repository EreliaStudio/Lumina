#include "lumina_compiler.hpp"

namespace Lumina
{
	void Compiler::compileNamespace(std::shared_ptr<NamespaceMetaToken> p_metaToken)
	{
		_namespaceNames.push_back(p_metaToken->name.content);

		for (const auto& innerMetaToken : p_metaToken->innerMetaTokens)
		{
			try
			{
				switch (innerMetaToken->type)
				{
				case MetaToken::Type::Constant:
				{
					compileConstant(static_pointer_cast<ConstantMetaToken>(innerMetaToken));
					break;
				}
				case MetaToken::Type::Attribute:
				{
					compileAttribute(static_pointer_cast<AttributeMetaToken>(innerMetaToken));
					break;
				}
				case MetaToken::Type::Structure:
				{
					compileStructure(static_pointer_cast<StructureMetaToken>(innerMetaToken));
					break;
				}
				case MetaToken::Type::Texture:
				{
					compileTexture(static_pointer_cast<TextureMetaToken>(innerMetaToken));
					break;
				}
				case MetaToken::Type::Function:
				{
					compileFunction(static_pointer_cast<FunctionMetaToken>(innerMetaToken));
					break;
				}
				case MetaToken::Type::Namespace:
				{
					compileNamespace(static_pointer_cast<NamespaceMetaToken>(innerMetaToken));
					break;
				}
				default:
				{
					throw TokenBasedError("Unknow meta token type", Token());
					break;
				}
				}
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
		}

		_namespaceNames.pop_back();
	}

	Compiler::Product Compiler::_compile(const std::vector<std::shared_ptr<MetaToken>>& p_metaTokens)
	{
		_result = Product();

		_result.value.outputLayouts += "0 Vector4\n\n";
		_result.value.fragmentShaderCode += "layout(location = 0) out vec4 pixelColor;\n\n";
		nbOutputLayout++;

		for (const auto& metaToken : p_metaTokens)
		{
			try
			{
				switch (metaToken->type)
				{
				case MetaToken::Type::PipelineFlow:
				{
					compilePipelineFlow(static_pointer_cast<PipelineFlowMetaToken>(metaToken));
					break;
				}
				case MetaToken::Type::PipelineBody:
				{
					compilePipelineBody(static_pointer_cast<PipelineBodyMetaToken>(metaToken));
					break;
				}
				case MetaToken::Type::Constant:
				{
					compileConstant(static_pointer_cast<ConstantMetaToken>(metaToken));
					break;
				}
				case MetaToken::Type::Attribute:
				{
					compileAttribute(static_pointer_cast<AttributeMetaToken>(metaToken));
					break;
				}
				case MetaToken::Type::Structure:
				{
					compileStructure(static_pointer_cast<StructureMetaToken>(metaToken));
					break;
				}
				case MetaToken::Type::Texture:
				{
					compileTexture(static_pointer_cast<TextureMetaToken>(metaToken));
					break;
				}
				case MetaToken::Type::Function:
				{
					compileFunction(static_pointer_cast<FunctionMetaToken>(metaToken));
					break;
				}
				case MetaToken::Type::Namespace:
				{
					compileNamespace(static_pointer_cast<NamespaceMetaToken>(metaToken));
					break;
				}
				default:
				{
					throw TokenBasedError("Unknow meta token type", Token());
					break;
				}
				}
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
		}

		return (_result);
	}

	Compiler::Compiler()
	{
		createScalarTypes();

		createFloatVectorTypes();
		createIntVectorTypes();
		createUIntVectorTypes();

		createMatrixTypes();

		createLuminaTypes();
	}

}