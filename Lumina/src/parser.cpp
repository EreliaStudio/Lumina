#include "parser.hpp"

#include "tokenizer.hpp"
#include "lexer.hpp"

namespace Lumina
{
	Parser::Product Parser::parse(const Lexer::Output& p_input)
	{
		Parser parser;

		parser._parse(p_input);

		return parser._product;
	}
	
	TypeImpl Parser::_getType(const std::string& p_relativeName)
	{
		auto it = _availibleTypes.find({ p_relativeName, {} });
		if (it != _availibleTypes.end())
		{
			return *it;
		}

		std::string namespaceName = "";
		for (const auto& ns : _nspaces)
		{
			namespaceName += ns + "::";

			it = _availibleTypes.find({ namespaceName + p_relativeName, {} });
			if (it != _availibleTypes.end())
			{
				return *it;
			}
		}

		return (TypeImpl());
	}
	
	TypeImpl Parser::_getType(const TypeInfo& p_typeInfo)
	{
		std::string fullTypeName = "";

		for (const auto& ns : p_typeInfo.nspace)
		{
			fullTypeName += ns.content + "::";
		}
		fullTypeName += p_typeInfo.value.content;

		return (_getType(fullTypeName));
	}

	std::string Parser::_namespacePrefix()
	{
		std::string result;

		for (const auto& nspace : _nspaces)
		{
			result += nspace + "::";
		}

		return (result);
	}

	std::string Parser::_composeName(const NameInfo& p_nameInfo)
	{
		return (p_nameInfo.value.content);
	}
	
	std::vector<size_t> Parser::_composeArraySizes(const ArraySizeInfo& p_arraySize)
	{
		std::vector<size_t> result;

		for (const auto& sizeToken : p_arraySize.dims)
		{
			result.push_back(std::stoll(sizeToken.content));
		}

		return (result);
	}
	
	VariableImpl Parser::_composeVariable(const VariableInfo& p_variableInfo)
	{
		VariableImpl result;

		result.type = _getType(p_variableInfo.type);

		if (result.type.name == TypeImpl::DefaultName)
		{
			throw TokenBasedError(
				"Type [" + p_variableInfo.type.value.content + "] not found in this scope",
				p_variableInfo.type.value
			);
		}
		result.name = _composeName(p_variableInfo.name);
		result.arraySizes = _composeArraySizes(p_variableInfo.arraySizes);

		return (result);
	}

	VariableImpl Parser::_composePipelineFlowVariable(const PipelineFlowInfo& p_pipelineFlowInfo)
	{
		return (_composeVariable(p_pipelineFlowInfo.variable));
	}

	ExpressionTypeImpl Parser::_composeExpressionTypeImpl(const ExpressionTypeInfo& p_expressionTypeInfo)
	{
		ExpressionTypeImpl result;

		result.type = _getType(p_expressionTypeInfo.type);
		result.arraySizes = _composeArraySizes(p_expressionTypeInfo.arraySizes);

		return (result);
	}

	ParameterImpl Parser::_composeParameter(const ParameterInfo& p_parameterInfo)
	{
		ParameterImpl result;

		result.type = _getType(p_parameterInfo.type);
		result.isReference = p_parameterInfo.isReference;
		result.name = _composeName(p_parameterInfo.name);
		result.arraySizes = _composeArraySizes(p_parameterInfo.arraySizes);

		return (result);
	}

	VariableImpl Parser::_composeTexture(const TextureInfo& p_textureInfo)
	{
		VariableImpl result;

		result.type = _getType("Texture");
		result.name = _namespacePrefix() + _composeName(p_textureInfo.name);
		result.arraySizes = _composeArraySizes(p_textureInfo.arraySizes);

		_globalVariables.insert(result);

		return (result);
	}

	FunctionImpl Parser::_composeFunction(const FunctionInfo& p_functionInfo)
	{
		std::set<VariableImpl> functionVariables = _globalVariables;
		FunctionImpl result;

		result.returnType = _composeExpressionTypeImpl(p_functionInfo.returnType);
		result.name = _namespacePrefix() + _composeName(p_functionInfo.name);
		result.isPrototype = p_functionInfo.isPrototype;
		for (const auto& param : p_functionInfo.parameters)
		{
			ParameterImpl paramImpl = _composeParameter(param);

			result.parameters.push_back(paramImpl);
			functionVariables.insert({.type = paramImpl.type, .name = paramImpl.name, .arraySizes = paramImpl.arraySizes});
		}

		result.body = _composeSymbolBody(functionVariables, p_functionInfo.body, 1);

		return (result);
	}

	TypeImpl Parser::_composeTypeImpl(const BlockInfo& p_blockInfo)
	{
		TypeImpl result;

		result.name = _namespacePrefix() + _composeName(p_blockInfo.name);
		for (const auto& element : p_blockInfo.attributes)
		{
			result.attributes.insert(_composeVariable(element));
		}

		return (result);
	}

	std::vector<FunctionImpl> Parser::_composeConstructors(const BlockInfo& p_blockInfo)
	{
		std::vector<FunctionImpl> result;

		TypeImpl originator = _getType(_composeName(p_blockInfo.name));

		for (const auto& constructorInfo : p_blockInfo.constructorInfos)
		{
			std::set<VariableImpl> constructionVariables = _globalVariables;
			FunctionImpl newConstructor = {
				.isPrototype = constructorInfo.isPrototype,
				.returnType = {originator, {}},
				.name = originator.name,
				.parameters = {},
				.body = {}
			};

			newConstructor.parameters.push_back({ originator, true, "this", {} });
			constructionVariables.insert({ originator, "this", {} });

			for (const auto& param : constructorInfo.parameters)
			{
				ParameterImpl paramImpl = _composeParameter(param);

				newConstructor.parameters.push_back(paramImpl);
				constructionVariables.insert({
					.type = paramImpl.type,
					.name = paramImpl.name,
					.arraySizes = paramImpl.arraySizes
					});
			}

			newConstructor.body = _composeSymbolBody(constructionVariables, constructorInfo.body, 1);

			result.push_back(newConstructor);
		}

		return (result);
	}

	std::vector<FunctionImpl> Parser::_composeMethods(const BlockInfo& p_blockInfo)
	{
		std::vector<FunctionImpl> result;

		TypeImpl originator = _getType(_composeName(p_blockInfo.name));

		for (const auto& [key, methodInfoArray] : p_blockInfo.methodInfos)
		{
			for (const auto& methodInfo : methodInfoArray)
			{
				std::string methodName = _composeName(methodInfo.name);

				if (originator.attributes.size() != 0 &&
					originator.attributes.find({ methodName, {} }) != originator.attributes.end())
				{
					throw TokenBasedError("The method [" + methodName + "] conflict with one attribute name.", methodInfo.name.value);
				}

				std::set<VariableImpl> methodVariables = _globalVariables;
				FunctionImpl newMethod = {
					.isPrototype = methodInfo.isPrototype,
					.returnType = {originator, {}},
					.name = originator.name + "_" + methodName,
					.parameters = {},
					.body = {}
				};

				newMethod.parameters.push_back({ originator, true, "this", {} });
				methodVariables.insert({ originator, "this", {} });

				for (const auto& param : methodInfo.parameters)
				{
					ParameterImpl paramImpl = _composeParameter(param);

					newMethod.parameters.push_back(paramImpl);
					methodVariables.insert({
						.type = paramImpl.type,
						.name = paramImpl.name,
						.arraySizes = paramImpl.arraySizes
						});
				}

				newMethod.body = _composeSymbolBody(methodVariables, methodInfo.body, 1);

				result.push_back(newMethod);
			}
		}

		return (result);
	}
	
	std::vector<FunctionImpl> Parser::_composeOperators(const BlockInfo& p_blockInfo)
	{
		
		std::vector<FunctionImpl> result;

		TypeImpl originator = _getType(_composeName(p_blockInfo.name));

		for (const auto& [key, operatorInfoArray] : p_blockInfo.operatorInfos)
		{
			for (const auto& operatorInfo : operatorInfoArray)
			{
				std::set<VariableImpl> operatorVariables = _globalVariables;
				FunctionImpl newOperator = {
					.isPrototype = operatorInfo.isPrototype,
					.returnType = {originator, {}},
					.name = originator.name + "_Operator" + _operatorNames.at(operatorInfo.opeType.content),
					.parameters = {},
					.body = {}
				};

				newOperator.parameters.push_back({ originator, true, "this", {} });
				operatorVariables.insert({ originator, "this", {} });

				for (const auto& param : operatorInfo.parameters)
				{
					ParameterImpl paramImpl = _composeParameter(param);

					newOperator.parameters.push_back(paramImpl);
					operatorVariables.insert({
						.type = paramImpl.type,
						.name = paramImpl.name,
						.arraySizes = paramImpl.arraySizes
						});
				}

				newOperator.body = _composeSymbolBody(operatorVariables, operatorInfo.body, 1);

				result.push_back(newOperator);
			}
		}

		return (result);
	}

	PipelinePassImpl Parser::_composePipelinePass(const PipelinePassInfo& p_pipelinePassInfo)
	{
		PipelinePassImpl result;

		result.isDefined = true;

		std::set<VariableImpl> variables = _globalVariables;

		if (p_pipelinePassInfo.name == "VertexPass")
		{
			variables.insert(_vertexVariables.begin(), _vertexVariables.end());
		}
		else
		{
			variables.insert(_fragmentVariables.begin(), _fragmentVariables.end());
		}

		result.body = _composeSymbolBody(variables, p_pipelinePassInfo.body, 1);

		return (result);
	}

	void Parser::_parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow)
	{
		VariableImpl pipelineVariable = _composePipelineFlowVariable(p_pipelineFlow);

		if (p_pipelineFlow.input == "Input" && p_pipelineFlow.output == "VertexPass")
		{
			_vertexVariables.insert(pipelineVariable);

			_product.value.vertexPipelineFlows.push_back(pipelineVariable);
		}
		else if (p_pipelineFlow.input == "VertexPass" && p_pipelineFlow.output == "FragmentPass")
		{
			_vertexVariables.insert(pipelineVariable);
			_fragmentVariables.insert(pipelineVariable);

			_product.value.fragmentPipelineFlows.push_back(pipelineVariable);
		}
		else if (p_pipelineFlow.input == "FragmentPass" && p_pipelineFlow.output == "Output")
		{
			_fragmentVariables.insert(pipelineVariable);

			_product.value.outputPipelineFlows.push_back(pipelineVariable);
		}
		else
		{
			throw Lumina::TokenBasedError("Invalid pipeline flow definition.", p_pipelineFlow.input + p_pipelineFlow.output);
		}
	}

	void Parser::_parsePipelinePass(const PipelinePassInfo& p_pipelinePass)
	{
		if (p_pipelinePass.name == "VertexPass")
		{
			if (_product.value.vertexPipelinePass.isDefined == true)
			{
				throw Lumina::TokenBasedError("VertexPass already defined.", p_pipelinePass.name);
			}

			_product.value.vertexPipelinePass = _composePipelinePass(p_pipelinePass);
		}
		else if (p_pipelinePass.name == "FragmentPass")
		{
			if (_product.value.fragmentPipelinePass.isDefined == true)
			{
				throw Lumina::TokenBasedError("VertexPass already defined.", p_pipelinePass.name);
			}

			_product.value.fragmentPipelinePass = _composePipelinePass(p_pipelinePass);
		}
		else
		{
			throw Lumina::TokenBasedError("Only VertexPass and FragmentPass can be defined.", p_pipelinePass.name);
		}
	}

	void Parser::_parseBlockInfo(const BlockInfo& p_blockInfo, std::vector<TypeImpl>& p_destination, bool p_needInstanciation)
	{
		TypeImpl newType = _composeTypeImpl(p_blockInfo);

		if (p_needInstanciation == true)
		{
			std::string name = newType.name;
			newType.name += "_Type";

			VariableImpl instance = {
				.type = newType,
				.name = name,
				.arraySizes = {}
			};

			_globalVariables.insert(instance);
		}

		_availibleTypes.insert(newType);
		_convertionTable[newType] = {_getType(newType.name)};
		p_destination.push_back(newType);

		std::vector<FunctionImpl> constructors = _composeConstructors(p_blockInfo);
		std::vector<FunctionImpl> methods = _composeMethods(p_blockInfo);
		std::vector<FunctionImpl> operators = _composeOperators(p_blockInfo);

		for (const auto& function : constructors)
		{
			_availibleFunctions.insert(function);
			_product.value.functions.push_back(function);
		}

		for (const auto& function : methods)
		{
			_availibleFunctions.insert(function);
			_product.value.functions.push_back(function);
		}

		for (const auto& function : operators)
		{
			_availibleFunctions.insert(function);
			_product.value.functions.push_back(function);
		}
	}
	
	void Parser::_parseBlockArray(const std::vector<BlockInfo>& p_blockInfos, std::vector<TypeImpl>& p_destination, bool p_needInstanciation)
	{
		for (const auto& structure : p_blockInfos)
		{
			try
			{
				_parseBlockInfo(structure, p_destination, p_needInstanciation);
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
		}
	}

	void Parser::_parseTextures(const std::vector<TextureInfo>& p_textureInfos)
	{
		for (const auto& textureInfo : p_textureInfos)
		{
			try
			{
				_product.value.textures.push_back(_composeTexture(textureInfo));
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
		}
	}
	void Parser::_parseFunctions(const std::vector<FunctionInfo>& p_functionInfos)
	{
		for (const auto& function : p_functionInfos)
		{
			try
			{
				FunctionImpl newFunction = _composeFunction(function);

				_availibleFunctions.insert(newFunction);
				_product.value.functions.push_back(newFunction);
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
		}
	}
	
	void Parser::_parseFunctionMap(const std::map<std::string, std::vector<FunctionInfo>>& p_functionInfosMap)
	{
		for (const auto& [key, functionArray] : p_functionInfosMap)
		{
			try
			{
				_parseFunctions(functionArray);
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
		}
	}

	void Parser::_parseNamespace(const NamespaceInfo& p_namespaceInfo)
	{
		_parseBlockArray(p_namespaceInfo.structureBlocks, _product.value.structures, false);
		_parseBlockArray(p_namespaceInfo.attributeBlocks, _product.value.attributes, true);
		_parseBlockArray(p_namespaceInfo.constantBlocks, _product.value.constants, true);

		_parseTextures(p_namespaceInfo.textureInfos);

		_parseFunctionMap(p_namespaceInfo.functionInfos);

		for (const auto& nspace : p_namespaceInfo.nestedNamespaces)
		{
			_nspaces.push_back(_composeName(nspace.name));

			try
			{
				_parseNamespace(nspace);
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}

			_nspaces.pop_back();
		}
	}
	
	void Parser::_parse(const Lexer::Output& p_input)
	{
		for (const auto& pipelineFlow : p_input.pipelineFlows)
		{
			try
			{
				_parsePipelineFlow(pipelineFlow);
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
		}

		_parseNamespace(p_input.anonymNamespace);

		for (const auto& pipelinePass : p_input.pipelinePasses)
		{
			try
			{
				_parsePipelinePass(pipelinePass);
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
		}
	}
}
