#include "lexer.hpp"
#include "token.hpp"

#include "tokenizer.hpp"
#include "utils.hpp"

namespace Lumina
{
	NamespaceDesignation Lexer::parseNamespaceDesignation()
	{
		NamespaceDesignation result;

		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
			advance();

		while (peekNext().type == Lumina::Token::Type::NamespaceSeparator)
		{
			result.push_back(expect(Lumina::Token::Type::Identifier, "Expected namespace identifier." + DEBUG_INFORMATION));
			advance();
		}

		return result;
	}

	TypeInfo Lexer::parseTypeInfo()
	{
		TypeInfo result;

		result.nspace = parseNamespaceDesignation();
		result.value = expect(Lumina::Token::Type::Identifier, "Expected type identifier." + DEBUG_INFORMATION);

		return result;
	}

	NameInfo Lexer::parseNameInfo()
	{
		NameInfo result;
		result.value = expect(Lumina::Token::Type::Identifier, "Expected variable name." + DEBUG_INFORMATION);
		return result;
	}

	ArraySizeInfo Lexer::parseArraySizeInfo()
	{
		ArraySizeInfo result;

		while (currentToken().type == Lumina::Token::Type::OpenBracket)
		{
			expect(Lumina::Token::Type::OpenBracket, "Expected closing bracket '['." + DEBUG_INFORMATION);
			result.dims.push_back(expect(Lumina::Token::Type::Number, "Expected an array size token." + DEBUG_INFORMATION));
			expect(Lumina::Token::Type::CloseBracket, "Expected closing bracket ']'." + DEBUG_INFORMATION);
		}

		return result;
	}

	VariableInfo Lexer::parseVariableInfo()
	{
		VariableInfo result;

		result.type = parseTypeInfo();
		result.name = parseNameInfo();
		result.arraySizes = parseArraySizeInfo();

		return result;
	}

	ParameterInfo Lexer::parseParameterInfo()
	{
		ParameterInfo result;

		result.type = parseTypeInfo();

		result.isReference = false;
		if (currentToken().type == Lumina::Token::Type::Operator && currentToken().content == "&")
		{
			result.isReference = true;
			advance();
		}

		result.name = parseNameInfo();

		result.arraySizes = parseArraySizeInfo();

		return result;
	}

	TextureInfo Lexer::parseTextureInfo()
	{
		TextureInfo result;

		expect(Lumina::Token::Type::Texture, "Expected 'Texture' token." + DEBUG_INFORMATION);

		result.name = parseNameInfo();

		result.arraySizes = parseArraySizeInfo();

		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after texture declaration." + DEBUG_INFORMATION);

		return result;
	}

	FunctionInfo Lexer::parseFunctionInfo()
	{
		FunctionInfo result;

		result.returnType.type = parseTypeInfo();

		result.name = parseNameInfo();

		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' for function parameters." + DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			if (result.parameters.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected ',' between function parameters." + DEBUG_INFORMATION);
			}

			result.parameters.push_back(parseParameterInfo());
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected closing parenthesis ')' for parameters." + DEBUG_INFORMATION);

		if (currentToken().type != Token::Type::EndOfSentence)
		{
			result.isPrototype = false;
			result.body = parseSymbolBodyInfo();
		}
		else
		{
			result.isPrototype = true;
			result.body = SymbolBodyInfo();
			expect(Lumina::Token::Type::EndOfSentence, "Expected ';' to end function prototype." + DEBUG_INFORMATION);
		}

		return result;
	}

	ConstructorInfo Lexer::parseConstructorInfo()
	{
		ConstructorInfo result;

		expect(Lumina::Token::Type::Identifier, "Expected a valid identifier." + DEBUG_INFORMATION);
		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' for function parameters." + DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			if (result.parameters.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected ',' between function parameters." + DEBUG_INFORMATION);
			}

			result.parameters.push_back(parseParameterInfo());
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected closing parenthesis ')' for parameters." + DEBUG_INFORMATION);

		if (currentToken().type != Token::Type::EndOfSentence)
		{
			result.isPrototype = false;
			result.body = parseSymbolBodyInfo();
		}
		else
		{
			result.isPrototype = true;
			result.body = SymbolBodyInfo();
			expect(Lumina::Token::Type::EndOfSentence, "Expected ';' to end constructor prototype." + DEBUG_INFORMATION);
		}

		return result;
	}

	OperatorInfo Lexer::parseOperatorInfo()
	{
		OperatorInfo result;


		result.returnType.type = parseTypeInfo();

		expect(Lumina::Token::Type::OperatorKeyword, "Expected 'operator' keyword." + DEBUG_INFORMATION);
		result.opeType = expect({ Lumina::Token::Type::Operator, Lumina::Token::Type::Assignator }, "Expected an operator token." + DEBUG_INFORMATION);

		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' for operator parameters." + DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			if (result.parameters.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected ',' between function parameters." + DEBUG_INFORMATION);
			}

			result.parameters.push_back(parseParameterInfo());
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected closing parenthesis ')' for parameters." + DEBUG_INFORMATION);

		if (currentToken().type != Token::Type::EndOfSentence)
		{
			result.isPrototype = false;
			result.body = parseSymbolBodyInfo();
		}
		else
		{
			result.isPrototype = true;
			result.body = SymbolBodyInfo();
			expect(Lumina::Token::Type::EndOfSentence, "Expected ';' to end operator prototype." + DEBUG_INFORMATION);
		}

		return result;
	}

	bool Lexer::describeFunction()
	{
		size_t offset = 0;

		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset++;
		}

		while (tokenAtOffset(offset).type == Lumina::Token::Type::Identifier &&
			tokenAtOffset(offset + 1).type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset += 2;
		}

		const Lumina::Token& returnTypeNameToken = tokenAtOffset(offset);
		if (returnTypeNameToken.type != Lumina::Token::Type::Identifier)
		{
			return false;
		}
		offset++;

		const Lumina::Token& methodNameToken = tokenAtOffset(offset);
		if (methodNameToken.type != Lumina::Token::Type::Identifier)
		{
			return false;
		}
		offset++;

		const Lumina::Token& afterMethodName = tokenAtOffset(offset);
		return (afterMethodName.type == Lumina::Token::Type::OpenParenthesis);
	}

	bool Lexer::describeConstructor()
	{
		size_t offset = 0;

		const Lumina::Token& constructedTypeToken = tokenAtOffset(offset);
		if (constructedTypeToken.type != Lumina::Token::Type::Identifier)
		{
			return false;
		}
		offset++;

		const Lumina::Token& parenthesisToken = tokenAtOffset(offset);
		return (parenthesisToken.type == Lumina::Token::Type::OpenParenthesis);
	}

	bool Lexer::describeOperator()
	{
		size_t offset = 0;

		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset++;
		}

		while (tokenAtOffset(offset).type == Lumina::Token::Type::Identifier &&
			tokenAtOffset(offset + 1).type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset += 2;
		}

		const Lumina::Token& returnTypeNameToken = tokenAtOffset(offset);
		if (returnTypeNameToken.type != Lumina::Token::Type::Identifier)
		{
			return false;
		}
		offset++;

		const Lumina::Token& operatorToken = tokenAtOffset(offset);
		if (operatorToken.type != Lumina::Token::Type::OperatorKeyword)
		{
			return false;
		}
		offset++;

		const Lumina::Token& operatorDesignationToken = tokenAtOffset(offset);
		if (operatorDesignationToken.type != Lumina::Token::Type::Operator &&
			operatorDesignationToken.type != Lumina::Token::Type::Assignator)
		{

			return false;
		}
		offset++;

		const Lumina::Token& afterMethodName = tokenAtOffset(offset);
		return (afterMethodName.type == Lumina::Token::Type::OpenParenthesis);
	}

	bool Lexer::describeVariableInfo()
	{
		size_t offset = 0;

		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset++;
		}

		while (tokenAtOffset(offset).type == Lumina::Token::Type::Identifier &&
			tokenAtOffset(offset + 1).type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset += 2;
		}

		if (tokenAtOffset(offset).type != Lumina::Token::Type::Identifier)
		{
			return false;
		}
		offset++;

		if (tokenAtOffset(offset).type != Lumina::Token::Type::Identifier)
		{
			return false;
		}
		offset++;

		if (tokenAtOffset(offset).type == Lumina::Token::Type::EndOfSentence ||
			tokenAtOffset(offset).type == Lumina::Token::Type::OpenBracket)
			return (true);
		return (false);
	}

	BlockInfo Lexer::parseBlockInfo()
	{
		BlockInfo result;

		expect({Lumina::Token::Type::StructureBlock, Lumina::Token::Type:: AttributeBlock, Lumina::Token::Type::ConstantBlock}, "Expected a valid block definition token.");
		result.name = parseNameInfo();
		
		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start block." + DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				if (describeConstructor())
				{
					ConstructorInfo constructorInfo = parseConstructorInfo();
					result.constructorInfos.push_back(std::move(constructorInfo));
				}
				else if (describeFunction())
				{
					FunctionInfo methodInfo = parseFunctionInfo();
					result.methodInfos[methodInfo.name.value.content].push_back(std::move(methodInfo));
				}
				else if (describeOperator())
				{
					OperatorInfo operatorInfo = parseOperatorInfo();
					result.operatorInfos[operatorInfo.opeType.content].push_back(operatorInfo);
				}
				else if (describeVariableInfo())
				{
					result.attributes.push_back(parseVariableInfo());
					expect(Lumina::Token::Type::EndOfSentence, "Expected ';' to end block attribute definition." + DEBUG_INFORMATION);
				}
				else
				{
					throw TokenBasedError("Unexpected token inside block." + DEBUG_INFORMATION, currentToken());
				}
			}
			catch (TokenBasedError& e)
			{
				_product.errors.push_back(e);
				skipLine();
			}
			
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to end block.");
		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' to end block.");

		return result;
	}

	PipelineFlowInfo Lexer::parsePipelineFlowInfo()
	{
		PipelineFlowInfo result;

		result.input = expect(Lumina::Token::Type::PipelineFlow, "Expected pipeline input stage.");

		expect(Lumina::Token::Type::PipelineFlowSeparator, "Expected '->' in pipeline flow.");

		result.output = expect(Lumina::Token::Type::PipelineFlow, "Expected pipeline output stage.");

		expect(Lumina::Token::Type::Separator, "Expected ':' in pipeline flow.");

		result.variable = parseVariableInfo();

		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' at the end of pipeline flow definition.");

		return result;
	}

	PipelinePassInfo Lexer::parsePipelinePassInfo()
	{
		PipelinePassInfo result;

		result.name = expect(Lumina::Token::Type::PipelineFlow, "Expected pipeline pass name.");
		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' token.");
		expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' token.");

		result.body = parseSymbolBodyInfo();

		return result;
	}

	NamespaceInfo Lexer::parseNamespaceInfo()
	{
		NamespaceInfo result;

		expect(Lumina::Token::Type::Namespace, "Expected 'namespace' token.");
		result.name = parseNameInfo();
		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start namespace body.");

		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				if (currentToken().type == Lumina::Token::Type::Namespace)
				{
					result.nestedNamespaces.push_back(parseNamespaceInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::StructureBlock)
				{
					result.structureBlocks.push_back(parseBlockInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::AttributeBlock)
				{
					result.attributeBlocks.push_back(parseBlockInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::ConstantBlock)
				{
					result.constantBlocks.push_back(parseBlockInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::Texture)
				{
					result.textureInfos.push_back(parseTextureInfo());
				}
				else if (describeFunction() == true)
				{
					FunctionInfo functionInfo = parseFunctionInfo();
					result.functionInfos[functionInfo.name.value.content].push_back(std::move(functionInfo));
				}
				else
				{
					throw TokenBasedError("Unexpected token inside namespace.", currentToken());
				}
			}
			catch (TokenBasedError& e)
			{
				_product.errors.push_back(e);
				skipLine();
			}
		}

		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to end namespace.");

		return result;
	}
	
	void Lexer::parseInclude()
	{
		skipToken();

		Lumina::Token includeToken = expect({ Lumina::Token::Type::IncludeLitteral, Lumina::Token::Type::StringLitteral }, "Expected a valid include path token.");

		std::string fileRelativePath = includeToken.content.substr(1, includeToken.content.size() - 2);
		std::filesystem::path currentDir = std::filesystem::current_path();

		std::vector<std::filesystem::path> additionalPaths = {
			currentDir,
			includeToken.context.originFile.parent_path(),
			includeToken.context.originFile.parent_path() / "..\\predefined_header"
		};

		std::string luminaCompilerPathEnv = getEnvVar("LuminaCompilerPath");
		if (!luminaCompilerPathEnv.empty())
		{
			additionalPaths.push_back(luminaCompilerPathEnv);
		}

		std::filesystem::path filePath = composeFilePath(fileRelativePath, additionalPaths);

		if (!std::filesystem::exists(filePath))
		{
			_index--;
			throw TokenBasedError("Included file '" + fileRelativePath + "' does not exist.", includeToken);
		}

		if (_alreadyLoadedFiles.contains(filePath) == true)
		{
			return ;
		}

		_alreadyLoadedFiles.insert(filePath);
		std::string rawCode = Lumina::readFileAsString(filePath);

		std::vector<Lumina::Token> includeTokens = Lumina::Tokenizer::tokenize(filePath, rawCode);

		_tokens.insert(_tokens.begin() + _index, includeTokens.begin(), includeTokens.end());
	}

	ShaderInfo Lexer::parseShaderInfo()
	{
		ShaderInfo result;

		while (hasTokenLeft())
		{
			try
			{
				if (currentToken().type == Lumina::Token::Type::Comment)
				{
					skipToken();
				}
				else if (currentToken().type == Lumina::Token::Type::Include)
				{
					parseInclude();
				}
				else if (currentToken().type == Lumina::Token::Type::PipelineFlow)
				{
					if (peekNext().type == Lumina::Token::Type::PipelineFlowSeparator)
					{
						result.pipelineFlows.push_back(parsePipelineFlowInfo());
					}
					else
					{
						result.pipelinePasses.push_back(parsePipelinePassInfo());
					}
				}
				else if (currentToken().type == Lumina::Token::Type::Namespace)
				{
					result.anonymNamespace.nestedNamespaces.push_back(parseNamespaceInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::StructureBlock)
				{
					result.anonymNamespace.structureBlocks.push_back(parseBlockInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::AttributeBlock)
				{
					result.anonymNamespace.attributeBlocks.push_back(parseBlockInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::ConstantBlock)
				{
					result.anonymNamespace.constantBlocks.push_back(parseBlockInfo());
				}
				else if (currentToken().type == Lumina::Token::Type::Texture)
				{
					result.anonymNamespace.textureInfos.push_back(parseTextureInfo());
				}
				else if (describeFunction() == true)
				{
					FunctionInfo functionInfo = parseFunctionInfo();
					result.anonymNamespace.functionInfos[functionInfo.name.value.content].push_back(std::move(functionInfo));
				}
				else if (currentToken().type == Lumina::Token::Type::Namespace)
				{
					result.anonymNamespace = parseNamespaceInfo();
				}
				else
				{
					throw TokenBasedError("Unexpected token in shader file.", currentToken());
				}
			}
			catch (TokenBasedError& e)
			{
				_product.errors.push_back(e);
				skipLine();
			}
		}

		return result;
	}

	Lexer::Product Lexer::_lex(const std::vector<Lumina::Token>& p_tokens)
	{
		_tokens = p_tokens;
		_index = 0;
		_product = Product();

		if (p_tokens.size() != 0)
		{
			_emptyToken = Lumina::Token();
			_emptyToken.context.originFile = p_tokens.at(0).context.originFile;

			_product.value = parseShaderInfo();
		}

		return _product;
	}

	const Lumina::Token& Lexer::currentToken() const
	{
		if (_index < _tokens.size())
		{
			return _tokens[_index];
		}
		return _emptyToken;
	}

	void Lexer::advance()
	{
		if (hasTokenLeft())
		{
			++_index;
		}
	}

	const Lumina::Token& Lexer::expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage)
	{
		if (currentToken().type != p_expectedType)
		{
			throw TokenBasedError(p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return result;
	}

	const Lumina::Token& Lexer::expect(const std::vector<Lumina::Token::Type>& p_expectedTypes, const std::string& p_errorMessage)
	{
		if (std::find(p_expectedTypes.begin(), p_expectedTypes.end(), currentToken().type) == p_expectedTypes.end())
		{
			throw TokenBasedError(p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return result;
	}

	void Lexer::skipToken()
	{
		advance();
	}

	void Lexer::skipLine()
	{
		while (hasTokenLeft(1) && currentToken().context.line == _tokens[_index + 1].context.line)
		{
			advance();
		}
		advance();
	}

	bool Lexer::hasTokenLeft(size_t p_offset) const
	{
		return (_index + p_offset) < _tokens.size();
	}

	const Lumina::Token& Lexer::peekNext() const
	{
		if (_index + 1 < _tokens.size())
		{
			return _tokens[_index + 1];
		}
		return _emptyToken;
	}

	const Lumina::Token& Lexer::tokenAtOffset(size_t p_offset) const
	{
		size_t targetIndex = _index + p_offset;
		if (targetIndex < _tokens.size())
		{
			return _tokens[targetIndex];
		}
		return _emptyToken;
	}

	void Lexer::moveBack(size_t p_steps)
	{
		if (p_steps <= _index)
		{
			_index -= p_steps;
		}
		else
		{
			_index = 0;
		}
	}

	Lexer::Product Lexer::lex(const std::vector<Lumina::Token>& p_tokens)
	{
		return (Lexer()._lex(p_tokens));
	}

	ConstructorInfo Lexer::lexConstructorSourceCode(const std::string& p_sourceCode)
	{
		ConstructorInfo result;
		Lexer tmpLexer;

		tmpLexer._tokens = Lumina::Tokenizer::tokenize("Native code", p_sourceCode);
		tmpLexer._index = 0;

		result = tmpLexer.parseConstructorInfo();

		return (result);
	}

	FunctionInfo Lexer::lexFunctionSourceCode(const std::string& p_sourceCode)
	{
		FunctionInfo result;
		Lexer tmpLexer;

		tmpLexer._tokens = Lumina::Tokenizer::tokenize("Native code", p_sourceCode);
		tmpLexer._index = 0;

		result = tmpLexer.parseFunctionInfo();

		return (result);
	}

	OperatorInfo Lexer::lexOperatorSourceCode(const std::string& p_sourceCode)
	{
		OperatorInfo result;
		Lexer tmpLexer;

		tmpLexer._tokens = Lumina::Tokenizer::tokenize("Native code", p_sourceCode);
		tmpLexer._index = 0;

		result = tmpLexer.parseOperatorInfo();

		return (result);
	}
}
