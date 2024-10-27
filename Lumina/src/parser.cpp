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

		newFunction.isPrototype = p_functionInfo.isPrototype;
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

		result.isPrototype = p_functionInfo.isPrototype;
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

		result.isPrototype = p_constructorInfo.isPrototype;
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

		result.isPrototype = p_operatorInfo.isPrototype;
		result.body = _composeSymbolBody(p_operatorInfo.body);

		return result;
	}

	void Parser::_composeShaderImpl()
	{
		// Convert pipeline flows
		for (const auto& variable : _shaderRepresentation.vertexVariables)
		{
			PipelineFlowImpl flowImpl;
			flowImpl.direction = PipelineFlowImpl::Direction::Out;
			flowImpl.variable = _convertVariable(variable);
			_product.value.vertexPipelineFlows.push_back(flowImpl);
		}

		for (const auto& variable : _shaderRepresentation.fragmentVariables)
		{
			PipelineFlowImpl flowImpl;
			flowImpl.direction = PipelineFlowImpl::Direction::In;
			flowImpl.variable = _convertVariable(variable);
			_product.value.fragmentPipelineFlows.push_back(flowImpl);
		}

		// Convert types (structures, attributes, constants)
		for (const auto& type : _shaderRepresentation.availableTypes)
		{
			TypeImpl typeImpl = _convertType(type);

			// Determine type category
			if (std::find(_shaderRepresentation.attributesTypes.begin(), _shaderRepresentation.attributesTypes.end(), &type) != _shaderRepresentation.attributesTypes.end())
			{
				_product.value.attributes.push_back(typeImpl);
			}
			else if (std::find(_shaderRepresentation.constantsTypes.begin(), _shaderRepresentation.constantsTypes.end(), &type) != _shaderRepresentation.constantsTypes.end())
			{
				_product.value.constants.push_back(typeImpl);
			}
			else
			{
				_product.value.structures.push_back(typeImpl);
			}
		}

		// Convert functions
		for (const auto& functionPair : _shaderRepresentation.availableFunctions)
		{
			for (const auto& function : functionPair.second)
			{
				FunctionImpl functionImpl = _convertFunction(function);
				_product.value.functions.push_back(functionImpl);
			}
		}

		// Convert methods, operators, and constructors
		for (const auto& type : _shaderRepresentation.availableTypes)
		{
			std::string typeName = type.name;
			// Methods
			for (const auto& methodPair : type.methods)
			{
				for (const auto& method : methodPair.second)
				{
					FunctionImpl functionImpl = _convertFunction(method, typeName);
					_product.value.functions.push_back(functionImpl);
				}
			}

			// Operators
			for (const auto& operatorPair : type.operators)
			{
				for (const auto& op : operatorPair.second)
				{
					FunctionImpl functionImpl = _convertFunction(op, typeName);
					_product.value.functions.push_back(functionImpl);
				}
			}

			// Constructors
			for (const auto& constructor : type.constructors)
			{
				FunctionImpl functionImpl;
				functionImpl.returnType.type = typeName;
				functionImpl.name = typeName + "_constructor";

				// Add parameters
				for (const auto& param : constructor.parameters)
				{
					functionImpl.parameters.push_back(_convertParameter(param));
				}

				functionImpl.body = _convertFunctionBody(constructor.body);
				_product.value.functions.push_back(functionImpl);
			}
		}

		// Convert global variables (textures)
		for (const auto& variable : _shaderRepresentation.globalVariables)
		{
			VariableImpl variableImpl = _convertVariable(variable);
			_product.value.textures.push_back(variableImpl);
		}

		// Convert main functions
		_product.value.vertexMain = _convertFunction(_shaderRepresentation.vertexPassMain);
		_product.value.fragmentMain = _convertFunction(_shaderRepresentation.fragmentPassMain);
	}

	// Convert a ShaderRepresentation::Type to TypeImpl
	TypeImpl Parser::_convertType(const ShaderRepresentation::Type& type)
	{
		TypeImpl typeImpl;
		typeImpl.name = type.name;

		// Convert attributes (variables)
		for (const auto& attribute : type.attributes)
		{
			VariableImpl varImpl = _convertVariable(attribute);
			typeImpl.attributes.push_back(varImpl);
		}

		return typeImpl;
	}

	// Convert a ShaderRepresentation::Variable to VariableImpl
	VariableImpl Parser::_convertVariable(const ShaderRepresentation::Variable& variable)
	{
		VariableImpl varImpl;
		varImpl.type = _findTypeImpl(variable.type->name);
		varImpl.name = variable.name;
		varImpl.arraySize = variable.arraySize;
		return varImpl;
	}

	// Helper method to find or create a TypeImpl based on type name
	const TypeImpl* Parser::_findTypeImpl(const std::string& typeName)
	{
		// Search in existing types
		for (const auto& typeImpl : _product.value.structures)
		{
			if (typeImpl.name == typeName)
				return &typeImpl;
		}
		for (const auto& typeImpl : _product.value.attributes)
		{
			if (typeImpl.name == typeName)
				return &typeImpl;
		}
		for (const auto& typeImpl : _product.value.constants)
		{
			if (typeImpl.name == typeName)
				return &typeImpl;
		}

		return nullptr;
	}

	// Convert a ShaderRepresentation::Function to FunctionImpl
	FunctionImpl Parser::_convertFunction(const ShaderRepresentation::Function& function, const std::string& typeName)
	{
		FunctionImpl funcImpl;
		funcImpl.returnType.type = function.returnType.type->name;
		funcImpl.returnType.arraySize = function.returnType.arraySize;

		// If converting a method, add 'self' parameter
		if (!typeName.empty())
		{
			funcImpl.name = typeName + "_" + function.name;

			ParameterImpl selfParam;
			selfParam.type = typeName + "*";
			selfParam.isReference = false;
			selfParam.name = "self";
			funcImpl.parameters.push_back(selfParam);
		}
		else
		{
			funcImpl.name = function.name;
		}

		// Convert parameters
		for (const auto& param : function.parameters)
		{
			funcImpl.parameters.push_back(_convertParameter(param));
		}

		// Convert function body (here, we just collect the code as a string)
		funcImpl.body = _convertFunctionBody(function.body);

		return funcImpl;
	}

	// Convert a ShaderRepresentation::Parameter to ParameterImpl
	ParameterImpl Parser::_convertParameter(const ShaderRepresentation::Parameter& parameter)
	{
		ParameterImpl paramImpl;
		paramImpl.type = parameter.type->name;
		if (parameter.isReference)
			paramImpl.type += "*"; // Using pointer to represent reference in C-style
		paramImpl.isReference = parameter.isReference;
		paramImpl.name = parameter.name;
		paramImpl.arraySize = parameter.arraySize;
		return paramImpl;
	}

	// Convert function body (SymbolBody) to FunctionBodyImpl
	FunctionBodyImpl Parser::_convertFunctionBody(const ShaderRepresentation::SymbolBody& body)
	{
		FunctionBodyImpl bodyImpl;
		// For the purpose of this conversion, we will simulate the code as a string
		// In a real implementation, we would need to generate the actual code
		bodyImpl.code = "// Function body code goes here\n";
		return bodyImpl;
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

		_composeShaderImpl();

		std::cout << _product.value << std::endl;

		return _product;
	}
}
