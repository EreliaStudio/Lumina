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
			fullTypeName += ns.content;
		}
		fullTypeName += p_typeInfo.value.content;

		return (_getType(fullTypeName));
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
		result.arraySize = _composeArraySizes(p_expressionTypeInfo.arraySizes);

		return (result);
	}

	ParameterImpl Parser::_composeParameter(const ParameterInfo& p_parameterInfo)
	{
		ParameterImpl result;

		result.type = _getType(p_parameterInfo.type);
		result.isReference = p_parameterInfo.isReference;
		result.name = _composeName(p_parameterInfo.name);
		result.arraySize = _composeArraySizes(p_parameterInfo.arraySizes);

		return (result);
	}

	SymbolBodyImpl Parser::_composeSymbolBody(const SymbolBodyInfo& p_symbolBodyInfo)
	{
		SymbolBodyImpl result;

		return (result);
	}

	VariableImpl Parser::_composeTexture(const TextureInfo& p_textureInfo)
	{
		VariableImpl result;

		result.type = _getType("Texture");
		result.name = _composeName(p_textureInfo.name);
		result.arraySizes = _composeArraySizes(p_textureInfo.arraySizes);

		return (result);
	}

	FunctionImpl Parser::_composeFunction(const FunctionInfo& p_functionInfo)
	{
		FunctionImpl result;

		result.returnType = _composeExpressionTypeImpl(p_functionInfo.returnType);
		result.name = _composeName(p_functionInfo.name);
		result.isPrototype = p_functionInfo.isPrototype;
		for (const auto& param : p_functionInfo.parameters)
		{
			result.parameters.push_back(_composeParameter(param));
		}
		result.body = _composeSymbolBody(p_functionInfo.body);

		return (result);
	}

	TypeImpl Parser::_composeTypeImpl(const BlockInfo& p_blockInfo)
	{
		TypeImpl result;

		result.name = _composeName(p_blockInfo.name);
		for (const auto& element : p_blockInfo.attributes)
		{
			result.attributes.push_back(_composeVariable(element));
		}

		return (result);
	}

	std::vector<FunctionImpl> Parser::_composeConstructors(const BlockInfo& p_blockInfo)
	{
		std::vector<FunctionImpl> result;

		TypeImpl originator = _getType(_composeName(p_blockInfo.name));

		for (const auto& constructorInfo : p_blockInfo.constructorInfos)
		{
			FunctionImpl newConstructor = {
				.isPrototype = constructorInfo.isPrototype,
				.returnType = {originator, {}},
				.name = originator.name + "_Constructor",
				.parameters = {},
				.body = {}
			};

			newConstructor.parameters.push_back({ originator, true, "this", {} });
			for (const auto& param : constructorInfo.parameters)
			{
				newConstructor.parameters.push_back(_composeParameter(param));
			}

			newConstructor.body = _composeSymbolBody(constructorInfo.body);

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
				FunctionImpl newMethod = {
					.isPrototype = methodInfo.isPrototype,
					.returnType = {originator, {}},
					.name = originator.name + "_" + _composeName(methodInfo.name),
					.parameters = {},
					.body = {}
				};

				newMethod.parameters.push_back({ originator, true, "this", {} });
				for (const auto& param : methodInfo.parameters)
				{
					newMethod.parameters.push_back(_composeParameter(param));
				}

				newMethod.body = _composeSymbolBody(methodInfo.body);

				result.push_back(newMethod);
			}
		}

		return (result);
	}
	
	std::vector<FunctionImpl> Parser::_composeOperators(const BlockInfo& p_blockInfo)
	{
		const static std::map<std::string, std::string> _operatorNames = {
			{"+", "Plus"},
			{"-", "Minus"},
			{"*", "Multiply"},
			{"/", "Divide"},
			{"%", "Modulo"},

			{"=", "Assign"},
			{"+=", "AddAssign"},
			{"-=", "SubtractAssign"},
			{"*=", "MultiplyAssign"},
			{"/=", "DivideAssign"},
			{"%=", "ModuloAssign"},

			{"==", "Equal"},
			{"!=", "NEqual"},
			{"<", "Less"},
			{">", "Greater"},
			{"<=", "LEqual"},
			{">=", "GEqual"},

			{"&&", "And"},
			{"||", "Or"},

			{"++", "Increment"},
			{"--", "Decrement"},
		};
		std::vector<FunctionImpl> result;

		TypeImpl originator = _getType(_composeName(p_blockInfo.name));

		for (const auto& [key, operatorInfoArray] : p_blockInfo.operatorInfos)
		{
			for (const auto& operatorInfo : operatorInfoArray)
			{
				FunctionImpl newOperator = {
					.isPrototype = operatorInfo.isPrototype,
					.returnType = {originator, {}},
					.name = originator.name + "_Operator" + _operatorNames.at(operatorInfo.opeType.content),
					.parameters = {},
					.body = {}
				};

				newOperator.parameters.push_back({ originator, true, "this", {} });
				for (const auto& param : operatorInfo.parameters)
				{
					newOperator.parameters.push_back(_composeParameter(param));
				}

				newOperator.body = _composeSymbolBody(operatorInfo.body);

				result.push_back(newOperator);
			}
		}

		return (result);
	}

	PipelinePassImpl Parser::_composePipelinePass(const PipelinePassInfo& p_pipelinePassInfo)
	{
		PipelinePassImpl result;

		result.isDefined = true;
		result.body = _composeSymbolBody(p_pipelinePassInfo.body);

		return (result);
	}

	void Parser::_parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow)
	{
		VariableImpl pipelineVariable = _composePipelineFlowVariable(p_pipelineFlow);

		if (p_pipelineFlow.input == "Input" && p_pipelineFlow.output == "VertexPass")
		{
			_product.value.vertexPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::In,
					.variable = pipelineVariable
				});
		}
		else if (p_pipelineFlow.input == "VertexPass" && p_pipelineFlow.output == "FragmentPass")
		{
			_product.value.vertexPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::Out,
					.variable = pipelineVariable
				});

			_product.value.fragmentPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::In,
					.variable = pipelineVariable
				});
		}
		else if (p_pipelineFlow.input == "FragmentPass" && p_pipelineFlow.output == "Output")
		{
			_product.value.fragmentPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::Out,
					.variable = pipelineVariable
				});
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

	void Parser::_parseBlockInfo(const BlockInfo& p_blockInfo, std::vector<TypeImpl>& p_destination)
	{
		TypeImpl newType = _composeTypeImpl(p_blockInfo);

		_availibleTypes.insert(newType);
		p_destination.push_back(newType);

		std::vector<FunctionImpl> constructors = _composeConstructors(p_blockInfo);
		std::vector<FunctionImpl> methods = _composeMethods(p_blockInfo);
		std::vector<FunctionImpl> operators = _composeOperators(p_blockInfo);

		_availibleFunctions.insert(_product.value.functions.end(), constructors.begin(), constructors.end());
		_product.value.functions.insert(_product.value.functions.end(), constructors.begin(), constructors.end());

		_availibleFunctions.insert(_product.value.functions.end(), methods.begin(), methods.end());
		_product.value.functions.insert(_product.value.functions.end(), methods.begin(), methods.end());

		_availibleFunctions.insert(_product.value.functions.end(), operators.begin(), operators.end());
		_product.value.functions.insert(_product.value.functions.end(), operators.begin(), operators.end());
	}
	
	void Parser::_parseBlockArray(const std::vector<BlockInfo>& p_blockInfos, std::vector<TypeImpl>& p_destination)
	{
		for (const auto& structure : p_blockInfos)
		{
			try
			{
				_parseBlockInfo(structure, p_destination);
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

				_availibleFunctions.push_back(newFunction);
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
		_parseBlockArray(p_namespaceInfo.structureBlocks, _product.value.structures);
		_parseBlockArray(p_namespaceInfo.attributeBlocks, _product.value.attributes);
		_parseBlockArray(p_namespaceInfo.constantBlocks, _product.value.constants);

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

		_parseNamespace(p_input.anonymNamespace);
	}
}
