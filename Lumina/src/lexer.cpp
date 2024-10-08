#include "lexer.hpp"
#include "token.hpp"

namespace Lumina
{
	// Define the empty token
	const Lumina::Token Lexer::_emptyToken = Lumina::Token();

	TypeInfo Lexer::parseTypeInfo()
	{
		TypeInfo result;

		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
			advance();

		while (peekNext().type == Lumina::Token::Type::NamespaceSeparator)
		{
			result.nspace.push_back(expect(Lumina::Token::Type::Identifier, "Expected namespace identifier."));
			advance();
		}

		result.value = expect(Lumina::Token::Type::Identifier, "Expected type identifier.");

		return result;
	}

	NameInfo Lexer::parseNameInfo()
	{
		NameInfo result;
		result.value = expect(Lumina::Token::Type::Identifier, "Expected variable name.");
		return result;
	}

	ArraySizeInfo Lexer::parseArraySizeInfo()
	{
		ArraySizeInfo result;

		while (currentToken().type == Lumina::Token::Type::OpenBracket)
		{
			expect(Lumina::Token::Type::OpenBracket, "Expected closing bracket '['.");
			result.dims.push_back(expect(Lumina::Token::Type::Number, "Expected an array size token."));
			expect(Lumina::Token::Type::CloseBracket, "Expected closing bracket ']'.");
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

		expect(Lumina::Token::Type::Texture, "Expected 'Texture' token.");

		result.name = parseNameInfo();

		result.arraySizes = parseArraySizeInfo();

		expect(Lumina::Token::Type::EndOfSentence, "Expected ';' after texture declaration.");

		return result;
	}

	SymbolBodyInfo Lexer::parseSymbolBody()
	{
		SymbolBodyInfo result;

		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start symbol body.");

		size_t openBraces = 1;

		while (openBraces > 0 && hasTokenLeft())
		{
			if (currentToken().type == Lumina::Token::Type::OpenCurlyBracket)
			{
				openBraces++;
			}
			else if (currentToken().type == Lumina::Token::Type::CloseCurlyBracket)
			{
				openBraces--;
				if (openBraces == 0)
					break;
			}
			advance();
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to end symbol body.");

		if (openBraces != 0)
		{
			throw TokenBasedError("Unmatched '{' in symbol body.", currentToken());
		}

		return result;
	}

	FunctionInfo Lexer::parseFunctionInfo()
	{
		FunctionInfo result;

		result.returnType.type = parseTypeInfo();

		result.name = expect(Lumina::Token::Type::Identifier, "Expected function name.");

		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' for function parameters.");
		while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			if (result.parameters.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected ',' between function parameters.");
			}

			result.parameters.push_back(parseParameterInfo());
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected closing parenthesis ')' for parameters.");

		result.body = parseSymbolBody();

		return result;
	}

	OperatorInfo Lexer::parseOperatorInfo()
	{
		OperatorInfo result;

		expect(Lumina::Token::Type::OperatorKeyword, "Expected 'operator' keyword.");
		result.opeType = expect({ Lumina::Token::Type::Operator, Lumina::Token::Type::ComparatorOperator, Lumina::Token::Type::ConditionOperator}, "Expected an operator token.");

		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' for operator parameters.");
		while (currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			if (result.parameters.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected ',' between function parameters.");
			}

			result.parameters.push_back(parseParameterInfo());
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected closing parenthesis ')' for parameters.");

		result.body = parseSymbolBody();

		return result;
	}

	bool Lexer::describeFunction()
	{
		size_t offset = 0;

		// Allow "::" at the beginning for global namespace or nested types.
		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset++;
		}

		// Parse through a sequence of identifiers (type or namespace names).
		while (tokenAtOffset(offset).type == Lumina::Token::Type::Identifier &&
			tokenAtOffset(offset + 1).type == Lumina::Token::Type::NamespaceSeparator)
		{
			// Move past the identifier and the "::"
			offset += 2;
		}

		// After parsing the potential namespace/type chain, expect an identifier for the method name.
		const Lumina::Token& methodNameToken = tokenAtOffset(offset);
		if (methodNameToken.type != Lumina::Token::Type::Identifier)
		{
			return false;
		}

		// The token after the method name should be an open parenthesis, indicating method parameters.
		const Lumina::Token& afterMethodName = tokenAtOffset(offset + 1);
		return (afterMethodName.type == Lumina::Token::Type::OpenParenthesis);
	}

	bool Lexer::describeVariableInfo()
	{
		size_t offset = 0;

		// Allow "::" at the beginning for global namespace or nested types.
		if (currentToken().type == Lumina::Token::Type::NamespaceSeparator)
		{
			offset++;
		}

		// Parse through a sequence of identifiers (type or namespace names).
		while (tokenAtOffset(offset).type == Lumina::Token::Type::Identifier &&
			tokenAtOffset(offset + 1).type == Lumina::Token::Type::NamespaceSeparator)
		{
			// Move past the identifier and the "::"
			offset += 2;
		}

		// After parsing the potential namespace/type chain, expect an identifier for the variable name.
		const Lumina::Token& nameToken = tokenAtOffset(offset);
		if (nameToken.type != Lumina::Token::Type::Identifier)
		{
			return false;
		}

		// Check for possible array sizes or an end-of-sentence ';' to indicate a variable declaration.
		const Lumina::Token& nextToken = tokenAtOffset(offset + 1);
		return (nextToken.type == Lumina::Token::Type::OpenBracket ||
			nextToken.type == Lumina::Token::Type::EndOfSentence);
	}

	BlockInfo Lexer::parseBlockInfo()
	{
		BlockInfo result;

		expect({Lumina::Token::Type::StructureBlock, Lumina::Token::Type:: AttributeBlock, Lumina::Token::Type::ConstantBlock}, "Expected a valid block definition token.");
		result.name = expect(Lumina::Token::Type::Identifier, "Expected block name.");
		
		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start block.");
		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				if (describeFunction())
				{
					FunctionInfo methodInfo = parseFunctionInfo();
					result.methodInfos[methodInfo.name.content].push_back(std::move(methodInfo));
				}
				else if (currentToken().type == Lumina::Token::Type::OperatorKeyword)
				{
					OperatorInfo operatorInfo = parseOperatorInfo();
					result.operatorInfos[operatorInfo.opeType.content].push_back(operatorInfo);
				}
				else if (describeVariableInfo())
				{
					result.attributes.push_back(parseVariableInfo());
				}
				else
				{
					throw TokenBasedError("Unexpected token inside block.", currentToken());
				}
			}
			catch (TokenBasedError& e)
			{
				_product.errors.push_back(e);
			}
			
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to end block.");

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

		result.body = parseSymbolBody();

		return result;
	}

	NamespaceInfo Lexer::parseNamespaceInfo()
	{
		NamespaceInfo result;

		expect(Lumina::Token::Type::Namespace, "Expected 'namespace' token.");
		result.name = expect(Lumina::Token::Type::Identifier, "Expected namespace name.");
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
					result.functionInfos[functionInfo.name.content].push_back(std::move(functionInfo));
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

	ShaderInfo Lexer::parseShaderInfo()
	{
		ShaderInfo result;

		while (hasTokenLeft())
		{
			try
			{
				if (currentToken().type == Lumina::Token::Type::PipelineFlow)
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
					result.anonymNamespace.functionInfos[functionInfo.name.content].push_back(std::move(functionInfo));
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

		_product.value = parseShaderInfo();

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
		while (hasTokenLeft() && currentToken().context.line == _tokens[_index + 1].context.line)
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
}
