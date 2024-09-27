#include "lumina_compiler.hpp"

namespace Lumina
{

	void Compiler::compileStructure(std::shared_ptr<StructureMetaToken> p_metaToken)
	{
		Type structType = composeType(p_metaToken);

		std::string structCode = composeBlockCode(BlockType::Structure, { &structType, "", {} });

		_result.value.vertexShaderCode += structCode;
		_result.value.fragmentShaderCode += structCode;

		addType(structType);
	}

	void Compiler::compileAttribute(std::shared_ptr<AttributeMetaToken> p_metaToken)
	{
		Variable attributeVariable = composeVariable(p_metaToken);

		_result.value.attributes += composeDataDescriptor(attributeVariable);

		std::string attributeCode = composeBlockCode(BlockType::Attribute, attributeVariable);

		_result.value.vertexShaderCode += attributeCode;
		_result.value.fragmentShaderCode += attributeCode;

		_vertexVariables.push_back(attributeVariable);
		_fragmentVariables.push_back(attributeVariable);
	}

	void Compiler::compileConstant(std::shared_ptr<ConstantMetaToken> p_metaToken)
	{
		Variable constantVariable = composeVariable(p_metaToken);

		_result.value.constants += composeDataDescriptor(constantVariable);

		std::string constantCode = composeBlockCode(BlockType::Attribute, constantVariable);

		_result.value.vertexShaderCode += constantCode;
		_result.value.fragmentShaderCode += constantCode;

		_vertexVariables.push_back(constantVariable);
		_fragmentVariables.push_back(constantVariable);
	}
}