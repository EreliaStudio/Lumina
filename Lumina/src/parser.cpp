#include "parser.hpp"

namespace Lumina
{
	std::string Parser::currentPrefix() const
	{
		std::string result;

		for (const auto& nspace : _currentNamespacePrefix)
		{
			std::string modifiedNspace;
			modifiedNspace += std::toupper(nspace[0]);

			for (size_t i = 1; i < nspace.size(); ++i)
			{
				modifiedNspace += std::tolower(nspace[i]);
			}

			result += modifiedNspace;
		}

		if (result.size() != 0)
		{
			result += "_";
		}

		return (result);
	}

	VariableImpl Parser::parseVariable(const VariableInfo& p_variableInfo)
	{
		VariableImpl result;

		result.type = p_variableInfo.type.value.content;
		result.name = p_variableInfo.name.value.content;

		for (const auto& dim : p_variableInfo.arraySizes.dims)
		{
			result.arraySize.push_back(std::stoul(dim.content));
		}

		return (result);
	}

	PipelineFlowImpl Parser::parsePipelineFlow(PipelineFlowImpl::Direction p_direction, const PipelineFlowInfo& p_pipelineInfo)
	{
		PipelineFlowImpl result;

		result.direction = PipelineFlowImpl::Direction::In;
		result.variable = parseVariable(p_pipelineInfo.variable);

		return (result);
	}

	void Parser::parsePipelineFlows(const std::vector<PipelineFlowInfo>& p_pipelineFlows)
	{
		for (const auto& flow : p_pipelineFlows)
		{
			if (flow.input == "Input" && flow.output == "VertexPass")
			{
				_product.value.vertexPipelineFlows.push_back(parsePipelineFlow(PipelineFlowImpl::Direction::In, flow));
			}
			else if (flow.input == "VertexPass" && flow.output == "FragmentPass")
			{
				_product.value.vertexPipelineFlows.push_back(parsePipelineFlow(PipelineFlowImpl::Direction::Out, flow));
				_product.value.fragmentPipelineFlows.push_back(parsePipelineFlow(PipelineFlowImpl::Direction::In, flow));
			}
			else if (flow.input == "FragmentPass" && flow.output == "Output")
			{
				_product.value.fragmentPipelineFlows.push_back(parsePipelineFlow(PipelineFlowImpl::Direction::Out, flow));
			}
			else
			{
				_product.errors.push_back(TokenBasedError("Invalid pipeline flow input/output.", flow.input + flow.output));
			}
		}
	}

	FunctionBodyImpl Parser::parseSymbolBody(const SymbolBodyInfo& p_symbolBody)
	{
		FunctionBodyImpl result;

		std::cout << "Not implemented : " << __FUNCTION__ << "::" << __LINE__ << std::endl;

		return (result);
	}

	FunctionImpl Parser::parsePipelinePass(const PipelinePassInfo& p_pipelinePass)
	{
		FunctionImpl result;

		result.returnType = {"void", {}};
		result.name = "main";
		result.parameters = {};
		result.body = parseSymbolBody(p_pipelinePass.body);

		return (result);
	}

	void Parser::parsePipelinePasses(const std::vector<PipelinePassInfo>& p_pipelinePasses)
	{
		for (const auto& pass : p_pipelinePasses)
		{
			if (pass.name == "VertexPass")
			{
				if (_vertexPassParsed == false)
				{
					_product.value.vertexMain = parsePipelinePass(pass);
					_vertexPassParsed = true;
				}
				else
				{
					_product.errors.push_back(TokenBasedError("VertexPass already defined.", pass.name));
				}
			}
			else if (pass.name == "FragmentPass")
			{
				if (_fragmentPassParsed == false)
				{
					_product.value.fragmentMain = parsePipelinePass(pass);
					_fragmentPassParsed = true;
				}
				else
				{
					_product.errors.push_back(TokenBasedError("FragmentPass already defined.", pass.name));
				}
			}
			else
			{
				_product.errors.push_back(TokenBasedError("Invalid pipeline pass definition name.", pass.name));
			}
		}
	}

	FunctionImpl Parser::parseMethodFunction(const std::string& p_blockName, const FunctionInfo& p_methodInfo)
	{
		FunctionImpl result;

		result.returnType.type = p_methodInfo.returnType.type.value.content;

		result.name = currentPrefix() + p_blockName + "_" + p_methodInfo.name.value.content;

		for (const auto& param : p_methodInfo.parameters)
		{
			ParameterImpl paramImpl;
			paramImpl.type = param.type.value.content;
			paramImpl.name = param.name.value.content;
			paramImpl.isReference = param.isReference;

			for (const auto& dim : param.arraySizes.dims)
			{
				paramImpl.arraySize.push_back(std::stoul(dim.content));
			}

			result.parameters.push_back(paramImpl);
		}

		result.body = parseSymbolBody(p_methodInfo.body);

		return result;
	}

	FunctionImpl Parser::parseOperatorFunction(const std::string& p_blockName, const OperatorInfo& p_operatorInfo)
	{
		FunctionImpl result;

		result.returnType.type = p_operatorInfo.returnType.type.value.content;

		const std::map<std::string, std::string> operatorStringMap = {
			{"+", "plus"},
			{"-", "minus"},
			{"*", "multiply"},
			{"/", "divide"},
			{"%", "modulus"},
			{"==", "equal"},
			{"!=", "not_equal"},
			{"<", "less_than"},
			{">", "greater_than"},
			{"<=", "less_equal"},
			{">=", "greater_equal"},
			{"&&", "and"},
			{"||", "or"},
			{"!", "not"},
			{"++", "increment"},
			{"--", "decrement"},
			{"+=", "plus_assign"},
			{"-=", "minus_assign"},
			{"*=", "multiply_assign"},
			{"/=", "divide_assign"}
		};

		result.name = currentPrefix() + p_blockName + "_operator_" + operatorStringMap.at(p_operatorInfo.opeType.content);

		for (const auto& param : p_operatorInfo.parameters)
		{
			ParameterImpl paramImpl;
			paramImpl.type = param.type.value.content;
			paramImpl.name = param.name.value.content;
			paramImpl.isReference = param.isReference;

			for (const auto& dim : param.arraySizes.dims)
			{
				paramImpl.arraySize.push_back(std::stoul(dim.content));
			}

			result.parameters.push_back(paramImpl);
		}

		result.body = parseSymbolBody(p_operatorInfo.body);

		return result;
	}

	BlockImpl Parser::parseBlockInfo(const BlockInfo& p_blockInfo)
	{
		BlockImpl result;

		result.name = currentPrefix() + p_blockInfo.name.value.content;

		for (const auto& attribute : p_blockInfo.attributes)
		{
			result.variables.push_back(parseVariable(attribute));
		}

		for (const auto& methodInfoArray : p_blockInfo.methodInfos)
		{
			for (const auto& methodInfo : methodInfoArray.second)
			{
				_product.value.functions.push_back(parseMethodFunction(result.name, methodInfo));
			}
		}

		for (const auto& operatorInfoArray : p_blockInfo.operatorInfos)
		{
			for (const auto& operatorInfo : operatorInfoArray.second)
			{
				_product.value.functions.push_back(parseOperatorFunction(result.name, operatorInfo));
			}
		}

		return (result);
	}

	void Parser::parseStructureBlockInfos(const std::vector<BlockInfo>& p_structureBlockInfo)
	{
		for (const auto& structure : p_structureBlockInfo)
		{
			_product.value.structures.push_back(parseBlockInfo(structure));
		}
	}

	void Parser::parseAttributeBlockInfos(const std::vector<BlockInfo>& p_attributeBlockInfo)
	{
		for (const auto& attribute : p_attributeBlockInfo)
		{
			_product.value.attributes.push_back(parseBlockInfo(attribute));
		}
	}

	void Parser::parseConstantBlockInfos(const std::vector<BlockInfo>& p_constantsBlockInfo)
	{
		for (const auto& constant : p_constantsBlockInfo)
		{
			_product.value.constants.push_back(parseBlockInfo(constant));
		}
	}

	VariableImpl Parser::parseTexture(const TextureInfo& p_textureInfo)
	{
		VariableImpl result;

		result.type = "Texture";
		result.name = currentPrefix() + p_textureInfo.name.value.content;

		for (const auto& dim : p_textureInfo.arraySizes.dims)
		{
			result.arraySize.push_back(std::stoul(dim.content));
		}

		return (result);
	}
	
	void Parser::parseTextures(const std::vector<TextureInfo>& p_textureInfos)
	{
		for (const auto& texture : p_textureInfos)
		{
			_product.value.textures.push_back(parseTexture(texture));
		}
	}

	FunctionImpl Parser::parseFunction(const FunctionInfo& p_function)
	{
		FunctionImpl result;

		result.returnType.type = p_function.returnType.type.value.content;
		result.name = currentPrefix() + p_function.name.value.content;

		for (const auto& param : p_function.parameters)
		{
			ParameterImpl paramImpl;
			paramImpl.type = param.type.value.content;
			paramImpl.name = param.name.value.content;
			paramImpl.isReference = param.isReference;

			for (const auto& dim : param.arraySizes.dims)
			{
				paramImpl.arraySize.push_back(std::stoul(dim.content));
			}

			result.parameters.push_back(paramImpl);
		}

		result.body = parseSymbolBody(p_function.body);

		return (result);
	}

	void Parser::parseFunctions(const std::vector<FunctionInfo>& p_functions)
	{
		for (const auto& function : p_functions)
		{
			_product.value.functions.push_back(parseFunction(function));
		}
	}

	void Parser::parseNamespace(const NamespaceInfo& p_namespace)
	{
		parseStructureBlockInfos(p_namespace.structureBlocks);
		parseAttributeBlockInfos(p_namespace.attributeBlocks);
		parseConstantBlockInfos(p_namespace.constantBlocks);
		parseTextures(p_namespace.textureInfos);

		for (const auto& functionArray : p_namespace.functionInfos)
		{
			parseFunctions(functionArray.second);
		}
		
		for (const auto& nspace : p_namespace.nestedNamespaces)
		{
			_currentNamespacePrefix.push_back(nspace.name.value.content);

			parseNamespace(nspace);

			_currentNamespacePrefix.pop_back();
		}
	}

	Parser::Product Parser::_parse(const Lexer::Output& p_input)
	{
		_product = Product();

		parsePipelineFlows(p_input.pipelineFlows);
		parsePipelinePasses(p_input.pipelinePasses);

		parseNamespace(p_input.anonymNamespace);

		return (_product);
	}

	Parser::Product Parser::parse(const Lexer::Output& p_input)
	{
		return (Parser()._parse(p_input));
	}
}