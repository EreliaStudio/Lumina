#include "parser.hpp"

namespace Lumina
{
	// Helper method to compose the type name
	std::string Parser::_composeTypeName(const TypeInfo& p_typeInfo)
	{
		std::string result = "";

		for (const auto& nspace : p_typeInfo.nspace)
		{
			result += nspace.content + "_";
		}

		result += p_typeInfo.value.content;

		return result;
	}

	// Helper method to compose array sizes
	std::vector<size_t> Parser::_composeSizeArray(const ArraySizeInfo& p_arraySizeInfo)
	{
		std::vector<size_t> result;

		for (const auto& size : p_arraySizeInfo.dims)
		{
			result.push_back(std::stoull(size.content));
		}

		return result;
	}

	// Get the current namespace as a string
	std::string Parser::currentNamespace()
	{
		std::string result = "";

		for (const auto& nspace : _nspaces)
		{
			result += nspace + "_";
		}

		return result;
	}

	// Extract the name from NameInfo
	std::string Parser::_extractNameInfo(const NameInfo& p_nameInfo)
	{
		return p_nameInfo.value.content;
	}

	// Find a type by name
	const ShaderRepresentation::Type* Parser::_findType(const std::string& p_objectName)
	{
		return _shaderRepresentation.findType(p_objectName);
	}

	// Find a type using TypeInfo
	const ShaderRepresentation::Type* Parser::_findType(const TypeInfo& p_typeInfo)
	{
		return _findType(_composeTypeName(p_typeInfo));
	}

	// Insert a variable into the shader representation
	void Parser::_insertVariable(const ShaderRepresentation::Variable& p_variable)
	{
		_shaderRepresentation.insertVariable(p_variable);
	}

	// Compose an ExpressionType from ExpressionTypeInfo
	ShaderRepresentation::ExpressionType Parser::_composeExpressionType(const ExpressionTypeInfo& p_expressionType)
	{
		ShaderRepresentation::ExpressionType result;

		result.type = _findType(p_expressionType.type);
		result.arraySize = _composeSizeArray(p_expressionType.arraySizes);

		return result;
	}

	// Compose a Variable from VariableInfo
	ShaderRepresentation::Variable Parser::_composeVariable(const VariableInfo& p_variableInfo)
	{
		ShaderRepresentation::Variable result;

		result.type = _findType(_composeTypeName(p_variableInfo.type));
		result.name = _extractNameInfo(p_variableInfo.name);
		result.arraySize = _composeSizeArray(p_variableInfo.arraySizes);

		return result;
	}

	// Compose a Type from BlockInfo
	ShaderRepresentation::Type Parser::_composeType(const BlockInfo& p_block, const std::string& p_suffix)
	{
		std::string typeName = currentNamespace() + p_block.name.value.content + p_suffix;

		ShaderRepresentation::Type result;
		result.name = typeName;

		for (const auto& attributeInfo : p_block.attributes)
		{
			ShaderRepresentation::Variable newAttribute = _composeVariable(attributeInfo);
			result.attributes.insert(newAttribute);
		}

		return result;
	}

	// Compose a Parameter from ParameterInfo
	ShaderRepresentation::Parameter Parser::_composeParameter(const ParameterInfo& p_parameterInfo)
	{
		ShaderRepresentation::Parameter result;

		result.type = _findType(p_parameterInfo.type);
		result.isReference = p_parameterInfo.isReference;
		result.name = _extractNameInfo(p_parameterInfo.name);
		result.arraySize = _composeSizeArray(p_parameterInfo.arraySizes);

		return result;
	}

	// Parse the pipeline flows
	void Parser::_parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow)
	{
		if (p_pipelineFlow.input == "Input" && p_pipelineFlow.output == "VertexPass")
		{
			_shaderRepresentation.vertexVariables.insert(_composeVariable(p_pipelineFlow.variable));
		}
		else if (p_pipelineFlow.input == "VertexPass" && p_pipelineFlow.output == "FragmentPass")
		{
			_shaderRepresentation.vertexVariables.insert(_composeVariable(p_pipelineFlow.variable));
			_shaderRepresentation.fragmentVariables.insert(_composeVariable(p_pipelineFlow.variable));
		}
		else if (p_pipelineFlow.input == "FragmentPass" && p_pipelineFlow.output == "Output")
		{
			_shaderRepresentation.fragmentVariables.insert(_composeVariable(p_pipelineFlow.variable));
		}
		else
		{
			// Handle other cases if necessary
		}
	}

	// Parse the pipeline passes
	void Parser::_parsePipelinePass(const PipelinePassInfo& p_pipelinePass)
	{
		if (p_pipelinePass.name == "VertexPass")
		{
			_shaderRepresentation.vertexPassMain = _composePipelinePass(p_pipelinePass);
		}
		else if (p_pipelinePass.name == "FragmentPass")
		{
			_shaderRepresentation.fragmentPassMain = _composePipelinePass(p_pipelinePass);
		}
	}

	// Compose a pipeline pass function
	ShaderRepresentation::Function Parser::_composePipelinePass(const PipelinePassInfo& p_pipelinePass)
	{
		ShaderRepresentation::Function result;

		result.returnType = { _findType("void"), {} };
		result.name = "main";
		result.parameters = {};
		result.body = _composeSymbolBody(p_pipelinePass.body);

		return result;
	}

	// Parse namespaces recursively
	void Parser::_parseNamespace(const NamespaceInfo& p_namespaceInfo)
	{
		for (const auto& block : p_namespaceInfo.structureBlocks)
		{
			_parseStructure(block);
		}

		for (const auto& block : p_namespaceInfo.attributeBlocks)
		{
			_parseAttribute(block);
		}

		for (const auto& block : p_namespaceInfo.constantBlocks)
		{
			_parseConstant(block);
		}

		for (const auto& texture : p_namespaceInfo.textureInfos)
		{
			_parseTexture(texture);
		}

		for (const auto& funcPair : p_namespaceInfo.functionInfos)
		{
			for (const auto& function : funcPair.second)
			{
				_parseFunction(function);
			}
		}

		for (const auto& nspace : p_namespaceInfo.nestedNamespaces)
		{
			_nspaces.push_back(nspace.name.value.content);

			_parseNamespace(nspace);

			_nspaces.pop_back();
		}
	}

	// Parse structures
	void Parser::_parseStructure(const BlockInfo& p_block)
	{
		ShaderRepresentation::Type newType = _composeType(p_block);
		ShaderRepresentation::Type* insertedType = _shaderRepresentation.insertType(newType);

		insertedType->acceptedConvertions = { insertedType };

		_computeMethodAndOperator(insertedType, p_block);
	}

	// Parse attributes
	void Parser::_parseAttribute(const BlockInfo& p_block)
	{
		ShaderRepresentation::Type newType = _composeType(p_block, "_Attributes");
		ShaderRepresentation::Type* insertedType = _shaderRepresentation.insertType(newType);

		_shaderRepresentation.attributesTypes.push_back(insertedType);

		_computeMethodAndOperator(insertedType, p_block);

		ShaderRepresentation::Variable var;
		var.type = insertedType;
		var.name = currentNamespace() + _extractNameInfo(p_block.name);
		var.arraySize = {};

		_shaderRepresentation.insertVariable(var);
	}

	// Parse constants
	void Parser::_parseConstant(const BlockInfo& p_block)
	{
		ShaderRepresentation::Type newType = _composeType(p_block, "_Constants");
		ShaderRepresentation::Type* insertedType = _shaderRepresentation.insertType(newType);

		_shaderRepresentation.constantsTypes.push_back(insertedType);

		_computeMethodAndOperator(insertedType, p_block);

		ShaderRepresentation::Variable var;
		var.type = insertedType;
		var.name = currentNamespace() + _extractNameInfo(p_block.name);
		var.arraySize = {};

		_shaderRepresentation.insertVariable(var);
	}

	// Parse textures
	void Parser::_parseTexture(const TextureInfo& p_texture)
	{
		ShaderRepresentation::Variable newTexture = {
			.type = _shaderRepresentation.findType("Texture"),
			.name = currentNamespace() + _extractNameInfo(p_texture.name),
			.arraySize = _composeSizeArray(p_texture.arraySizes)
		};

		_shaderRepresentation.globalVariables.insert(newTexture);
		_shaderRepresentation.reservedIdentifiers.insert(newTexture.name);
	}

	// Parse functions
	void Parser::_parseFunction(const FunctionInfo& p_functionInfo)
	{
		ShaderRepresentation::Function newFunction;

		newFunction.returnType = _composeExpressionType(p_functionInfo.returnType);
		newFunction.name = currentNamespace() + _extractNameInfo(p_functionInfo.name);

		for (const auto& parameter : p_functionInfo.parameters)
		{
			newFunction.parameters.push_back(_composeParameter(parameter));
		}

		newFunction.body = _composeSymbolBody(p_functionInfo.body);

		_shaderRepresentation.reservedIdentifiers.insert(newFunction.name);
		_shaderRepresentation.availableFunctions[newFunction.name].push_back(newFunction);
	}

	// Compute methods and operators for a type
	void Parser::_computeMethodAndOperator(ShaderRepresentation::Type* p_originator, const BlockInfo& p_block)
	{
		for (const auto& constructor : p_block.constructorInfos)
		{
			ShaderRepresentation::Type::Constructor newConstructor = _composeConstructorFunction(p_originator, constructor);
			p_originator->constructors.push_back(newConstructor);
		}

		for (const auto& methodArray : p_block.methodInfos)
		{
			for (const auto& method : methodArray.second)
			{
				ShaderRepresentation::Function newMethod = _composeMethodFunction(p_originator, method);
				p_originator->methods[newMethod.name].push_back(newMethod);
			}
		}

		for (const auto& operatorArray : p_block.operatorInfos)
		{
			for (const auto& ope : operatorArray.second)
			{
				ShaderRepresentation::Function newOperator = _composeOperatorFunction(p_originator, ope);
				p_originator->operators[newOperator.name].push_back(newOperator);
			}
		}
	}

	// Parse method function
	ShaderRepresentation::Function Parser::_composeMethodFunction(const ShaderRepresentation::Type* p_originatorType, const FunctionInfo& p_functionInfo)
	{
		ShaderRepresentation::Function result;

		result.returnType = _composeExpressionType(p_functionInfo.returnType);
		result.name = p_functionInfo.name.value.content;

		for (const auto& parameter : p_functionInfo.parameters)
		{
			result.parameters.push_back(_composeParameter(parameter));
		}

		result.body = _composeSymbolBody(p_functionInfo.body);

		return result;
	}

	// Parse constructor function
	ShaderRepresentation::Type::Constructor Parser::_composeConstructorFunction(const ShaderRepresentation::Type* p_originatorType, const ConstructorInfo& p_constructorInfo)
	{
		ShaderRepresentation::Type::Constructor result;

		for (const auto& parameter : p_constructorInfo.parameters)
		{
			result.parameters.push_back(_composeParameter(parameter));
		}

		result.body = _composeSymbolBody(p_constructorInfo.body);

		return result;
	}

	// Parse operator function
	ShaderRepresentation::Function Parser::_composeOperatorFunction(const ShaderRepresentation::Type* p_originatorType, const OperatorInfo& p_operatorInfo)
	{
		ShaderRepresentation::Function result;

		result.returnType = _composeExpressionType(p_operatorInfo.returnType);
		result.name = p_operatorInfo.opeType.content;

		for (const auto& parameter : p_operatorInfo.parameters)
		{
			result.parameters.push_back(_composeParameter(parameter));
		}

		result.body = _composeSymbolBody(p_operatorInfo.body);

		return result;
	}

	// The main parse method
	Parser::Product Parser::parse(const Lexer::Output& p_input)
	{
		Parser parser;
		return parser._parse(p_input);
	}

	// Internal parse method
	Parser::Product Parser::_parse(const Lexer::Output& p_input)
	{
		for (const auto& flow : p_input.pipelineFlows)
		{
			_parsePipelineFlow(flow);
		}

		for (const auto& pass : p_input.pipelinePasses)
		{
			_parsePipelinePass(pass);
		}

		_parseNamespace(p_input.anonymNamespace);

		std::cout << _shaderRepresentation << std::endl;

		return Product();
	}
}
