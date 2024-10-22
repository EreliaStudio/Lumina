#include "parser.hpp"

namespace Lumina
{
	std::string Parser::_composeTypeName(const TypeInfo& p_typeInfo)
	{
		std::string result = "";

		for (const auto& nspace : p_typeInfo.nspace)
		{
			result += nspace.content + "_";
		}

		result += p_typeInfo.value.content;

		return (result);
	}

	std::vector<size_t> Parser::_composeSizeArray(const ArraySizeInfo& p_arraySizeInfo)
	{
		std::vector<size_t> result;

		for (const auto& size : p_arraySizeInfo.dims)
		{
			result.push_back(std::stoull(size.content));
		}

		return (result);
	}

	std::string Parser::_composeIdentifierName(const std::string& p_identifierName)
	{
		std::string result = "";
		
		for (const auto& nspace : _nspaces)
		{
			result += nspace + "_";
		}

		result += p_identifierName;

		return (result);
	}

	Parser::Type* Parser::_insertType(const Type& p_inputType)
	{
		_availibleTypes[p_inputType] = p_inputType;
		_reservedIdentifiers.insert(p_inputType.name);
		return (&_availibleTypes[p_inputType]);
	}

	const Parser::Type* Parser::_findType(const std::string& p_objectName)
	{
		Parser::Type expectedType = { p_objectName, {} };

		if (_availibleTypes.contains(expectedType) == true)
		{
			return &(_availibleTypes.at(expectedType));
		}

		for (size_t i = 0; i < _nspaces.size(); i++)
		{
			std::string prefix = "";
			for (size_t j = i; j < _nspaces.size(); j++)
			{
				prefix += _nspaces[j] + "_";
			}

			expectedType = {prefix + p_objectName, {}};

			if (_availibleTypes.contains(expectedType) == true)
			{
				return &(_availibleTypes.at(expectedType));
			}
		}

		return (nullptr);
	}

	const Parser::Type* Parser::_findType(const TypeInfo& p_typeInfo)
	{
		return (_findType(_composeTypeName(p_typeInfo)));
	}

	void Parser::_insertVariable(const Parser::Variable& p_variable)
	{
		_globalVariables.insert(p_variable);
		_reservedIdentifiers.insert(p_variable.name);
	}

	Parser::Variable Parser::_composeVariable(const VariableInfo& p_variableInfo)
	{
		Parser::Variable result;

		result.type = _findType(_composeTypeName(p_variableInfo.type));
		result.name = _composeIdentifierName(p_variableInfo.name.value.content);
		result.arraySize = _composeSizeArray(p_variableInfo.arraySizes);

		return (result);
	}

	Parser::Type Parser::_composeType(const BlockInfo& p_block, const std::string& p_suffix)
	{
		std::string typeName = _composeIdentifierName(p_block.name.value.content + p_suffix);

		Parser::Type result = { typeName, {} };

		for (const auto& attributeInfo : p_block.attributes)
		{
			Parser::Variable newAttribute = _composeVariable(attributeInfo);

			result.attributes.insert(newAttribute);
		}

		return (result);
	}
	
	Parser::ExpressionType Parser::_composeExpressionType(const ExpressionTypeInfo& p_expressionType)
	{
		Parser::ExpressionType result;

		result.type = _findType(p_expressionType.type);
		result.arraySize = _composeSizeArray(p_expressionType.arraySizes);

		return (result);
	}
	
	Parser::SymbolBody Parser::_composeSymbolBody(const SymbolBodyInfo& p_symbolInfo)
	{
		Parser::SymbolBody result;

		return (result);
	}

	Parser::Function Parser::_composeMethodFunction(const FunctionInfo& p_functionInfo)
	{
		Parser::Function result;

		result.returnType = _composeExpressionType(p_functionInfo.returnType);
		result.name = p_functionInfo.name.value.content;

		for (const auto& parameter : p_functionInfo.parameters)
		{
			result.parameters.push_back({
					.type = _findType(parameter.type),
					.isReference = parameter.isReference,
					.name = parameter.name.value.content,
					.arraySize = _composeSizeArray(parameter.arraySizes)
				});			
		}

		result.body = _composeSymbolBody(p_functionInfo.body);

		return (result);
	}

	Parser::Type::Constructor Parser::_composeConstructorFunction(const ConstructorInfo& p_constructorInfo)
	{
		Parser::Type::Constructor result;

		for (const auto& parameter : p_constructorInfo.parameters)
		{
			result.parameters.push_back({
					.type = _findType(parameter.type),
					.isReference = parameter.isReference,
					.name = parameter.name.value.content,
					.arraySize = _composeSizeArray(parameter.arraySizes)
				});
		}

		result.body = _composeSymbolBody(p_constructorInfo.body);

		return (result);
	}

	Parser::Function Parser::_composeOperatorFunction(const OperatorInfo& p_operatorInfo)
	{
		Parser::Function result;

		result.returnType = _composeExpressionType(p_operatorInfo.returnType);
		result.name = p_operatorInfo.opeType.content;

		for (const auto& parameter : p_operatorInfo.parameters)
		{
			result.parameters.push_back({
					.type = _findType(parameter.type),
					.isReference = parameter.isReference,
					.name = parameter.name.value.content,
					.arraySize = _composeSizeArray(parameter.arraySizes)
				});
		}

		result.body = _composeSymbolBody(p_operatorInfo.body);

		return (result);
	}

	void Parser::_computeMethodAndOperator(Parser::Type* p_originator, const BlockInfo& p_block)
	{
		for (const auto& constructor : p_block.constructorInfos)
		{
			Parser::Type::Constructor newConstructor = _composeConstructorFunction(constructor);

			p_originator->constructors.push_back(newConstructor);
		}

		for (const auto& methodArray : p_block.methodInfos)
		{
			for (const auto& method : methodArray.second)
			{
				Parser::Function newMethods = _composeMethodFunction(method);

				p_originator->methods[newMethods.name].push_back(newMethods);
			}
		}

		for (const auto& operatorArray : p_block.operatorInfos)
		{
			for (const auto& ope : operatorArray.second)
			{
				Parser::Function newOperator = _composeOperatorFunction(ope);

				p_originator->operators[newOperator.name].push_back(newOperator);
			}
		}
	}

	void Parser::_parseStructure(const BlockInfo& p_block)
	{
		Type* insertedType = _insertType(_composeType(p_block));

		_computeMethodAndOperator(insertedType, p_block);
	}

	void Parser::_parseAttribute(const BlockInfo& p_block)
	{
		Type* insertedType = _insertType(_composeType(p_block, "_Attributes"));

		_attributesTypes.push_back(insertedType);

		_computeMethodAndOperator(insertedType, p_block);

		_insertVariable({
				.type = insertedType,
				.name = _composeIdentifierName(p_block.name.value.content),
				.arraySize = {}
			});

		_computeMethodAndOperator(insertedType, p_block);
	}

	void Parser::_parseConstant(const BlockInfo& p_block)
	{
		Type* insertedType = _insertType(_composeType(p_block, "_Constants"));

		_constantsTypes.push_back(insertedType);

		_computeMethodAndOperator(insertedType, p_block);

		_insertVariable({
				.type = insertedType,
				.name = _composeIdentifierName(p_block.name.value.content),
				.arraySize = {}
			});

		_computeMethodAndOperator(insertedType, p_block);
	}

	void Parser::_parseTexture(const TextureInfo& p_texture)
	{
		Parser::Variable newTexture = {
				.type = _findType("Texture"),
				.name = _composeIdentifierName(p_texture.name.value.content),
				.arraySize = _composeSizeArray(p_texture.arraySizes)
		};

		_globalVariables.insert(newTexture);
		_reservedIdentifiers.insert(newTexture.name);
	}

	void Parser::_parseFunction(const FunctionInfo& p_functionInfo)
	{
		Function newFunction;

		newFunction.returnType = _composeExpressionType(p_functionInfo.returnType);
		newFunction.name = _composeIdentifierName(p_functionInfo.name.value.content);

		for (const auto& parameter : p_functionInfo.parameters)
		{
			newFunction.parameters.push_back({
					.type = _findType(parameter.type),
					.isReference = parameter.isReference,
					.name = parameter.name.value.content,
					.arraySize = _composeSizeArray(parameter.arraySizes)
				});
		}

		newFunction.body = _composeSymbolBody(p_functionInfo.body);

		_reservedIdentifiers.insert(newFunction.name);
		_availibleFunctions[newFunction.name].push_back(newFunction);
	}

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

		for (auto it : p_namespaceInfo.functionInfos)
		{
			for (const auto& function : it.second)
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
		
	void Parser::_parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow)
	{
		if (p_pipelineFlow.input == "Input" && p_pipelineFlow.output == "VertexPass")
		{
			_vertexVariables.insert(_composeVariable(p_pipelineFlow.variable));
		}
		else if (p_pipelineFlow.input == "VertexPass" && p_pipelineFlow.output == "FragmentPass")
		{
			_vertexVariables.insert(_composeVariable(p_pipelineFlow.variable));
			_fragmentVariables.insert(_composeVariable(p_pipelineFlow.variable));
		}
		else if (p_pipelineFlow.input == "FragmentPass" && p_pipelineFlow.output == "Output")
		{
			_fragmentVariables.insert(_composeVariable(p_pipelineFlow.variable));
		}
		else
		{
			
		}
	}
	
	Parser::Function Parser::_composePipelinePass(const PipelinePassInfo& p_pipelinePass)
	{
		Function result;

		result.returnType = { _findType("void"), {} };
		result.name = "main";
		result.parameters = {};
		result.body = _composeSymbolBody(p_pipelinePass.body);

		return (result);
	}
	
	void Parser::_parsePipelinePass(const PipelinePassInfo& p_pipelinePass)
	{
		if (p_pipelinePass.name == "VertexPass")
		{
			_vertexPassMain = _composePipelinePass(p_pipelinePass);
		}
		else if(p_pipelinePass.name == "FragmentPass")
		{
			_fragmentPassMain = _composePipelinePass(p_pipelinePass);
		}
	}

	Parser::Product Parser::_parse(const Lexer::Output& p_input)
	{
		_product = Product();

		for (const auto& flow : p_input.pipelineFlows)
		{
			_parsePipelineFlow(flow);
		}

		for (const auto& pass : p_input.pipelinePasses)
		{
			_parsePipelinePass(pass);
		}

		_parseNamespace(p_input.anonymNamespace);

		printParsedData();

		return (_product);
	}

	Parser::Product Parser::parse(const Lexer::Output& p_input)
	{
		return (Parser()._parse(p_input));
	}
}