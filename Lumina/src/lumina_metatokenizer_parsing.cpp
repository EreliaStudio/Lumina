#include "lumina_metatokenizer.hpp"

namespace Lumina
{
	TypeDescriptor MetaTokenizer::parseTypeDescriptor()
	{
		TypeDescriptor result;

		if (currentToken().type == TokenType::NamespaceSeparator)
		{
			result.append(expect(TokenType::NamespaceSeparator, "Expected an anonymous namespace separator token '::'."));
		}
		while (nextToken().type == TokenType::NamespaceSeparator)
		{
			result.append(expect(TokenType::Identifier, "Expected a namespace identifier name."));
			result.append(expect(TokenType::NamespaceSeparator, "Expected a namespace separator token '::'."));
		}
		result.append(expect(TokenType::Identifier, "Expected a type identifier name."));

		return result;
	}

	int MetaTokenizer::parseArraySizeValue()
	{
		auto applyPrimary = [this]() -> int {
			switch (currentToken().type)
			{
			case TokenType::OpenParenthesis:
			{
				advance();
				int value = parseArraySizeValue();
				expect(TokenType::CloseParenthesis, "Expected a ')' token.");
				return value;
			}
			case TokenType::Number:
			{
				int value = std::stol(currentToken().content);
				advance();
				return value;
			}
			default:
			{
				throw Lumina::TokenBasedError("Expected a number or '(' token.", currentToken());
			}
			}
			};

		auto applyFactor = [&applyPrimary, this]() -> int {
			int leftValue = applyPrimary();

			while (currentToken().type == TokenType::Operator &&
				(currentToken() == "*" || currentToken() == "/" || currentToken() == "%"))
			{
				std::string op = currentToken().content;
				advance();
				int rightValue = applyPrimary();

				if (op == "*")
				{
					leftValue *= rightValue;
				}
				else if (op == "/")
				{
					if (rightValue == 0)
					{
						throw Lumina::TokenBasedError("Division by zero is not allowed.", currentToken());
					}
					leftValue /= rightValue;
				}
				else if (op == "%")
				{
					if (rightValue == 0)
					{
						throw Lumina::TokenBasedError("Modulo by zero is not allowed.", currentToken());
					}
					leftValue %= rightValue;
				}
			}

			return leftValue;
			};

		int leftValue = applyFactor();

		while (currentToken().type == TokenType::Operator &&
			(currentToken() == "+" || currentToken() == "-"))
		{
			std::string op = currentToken().content;
			advance();
			int rightValue = applyFactor();

			if (op == "+")
			{
				leftValue += rightValue;
			}
			else if (op == "-")
			{
				leftValue -= rightValue;
			}
		}

		return leftValue;
	}

	std::vector<size_t> MetaTokenizer::parseArraySizes()
	{
		std::vector<size_t> result;

		while (currentToken().type == TokenType::OpenBracket)
		{
			expect(TokenType::OpenBracket, "Expected a '[' token.");
			size_t startingIndex = _index;
			int newArraySize = parseArraySizeValue();

			if (newArraySize == 0)
			{
				throw Lumina::TokenBasedError("Array size evaluated to 0", composeToken(startingIndex, _index, TokenType::Number));
			}
			if (newArraySize < 0)
			{
				throw Lumina::TokenBasedError("Array size been evaluated to [" + std::to_string(newArraySize) + "].", composeToken(startingIndex, _index, TokenType::Number));
			}

			result.push_back(newArraySize);

			expect(TokenType::CloseBracket, "Expected a ']' token.");
		}

		return (result);
	}

	VariableDescriptor MetaTokenizer::parseVariableDescriptor()
	{
		VariableDescriptor result;

		result.type = parseTypeDescriptor();
		result.name = expect(TokenType::Identifier, "Expected an identifier name.");
		result.arraySizes = parseArraySizes();

		return result;
	}

	std::shared_ptr<PipelineFlowMetaToken> MetaTokenizer::parsePipelineFlowMetaToken()
	{
		std::shared_ptr<PipelineFlowMetaToken> result = std::make_shared<PipelineFlowMetaToken>();

		result->inputFlow = expect(TokenType::PipelineFlow, "Expected a pipeline flow token.");
		expect(TokenType::PipelineFlowSeparator, "Expected a '->' token.");
		result->outputFlow = expect(TokenType::PipelineFlow, "Expected a pipeline flow token.");
		expect(TokenType::Separator, "Expected a ':' token.");
		result->variableDescriptor = parseVariableDescriptor();
		expect(TokenType::EndOfSentence, "Expected a ';' token.");

		if (result->variableDescriptor.arraySizes.size() != 0)
		{
			throw Lumina::TokenBasedError("Pipeline flow variable cannot be array.", result->variableDescriptor.name);
		}

		return result;
	}

	std::shared_ptr<BlockMetaToken> MetaTokenizer::parseBlockMetaToken(const TokenType& p_tokenType)
	{
		std::shared_ptr<BlockMetaToken> result;

		switch (p_tokenType)
		{
		case TokenType::ConstantBlock:
		{
			result = std::make_shared<ConstantMetaToken>();
			break;
		}
		case TokenType::AttributeBlock:
		{
			result = std::make_shared<AttributeMetaToken>();
			break;
		}
		case TokenType::StructureBlock:
		{
			result = std::make_shared<StructureMetaToken>();
			break;
		}
		}

		expect({ TokenType::ConstantBlock , TokenType::AttributeBlock , TokenType::StructureBlock }, "Expected a block token.");
		result->name = expect(TokenType::Identifier, "Expected an identifier token.");
		expect(TokenType::OpenCurlyBracket, "Expected a '{' token.");
		while (hasTokenLeft() && currentToken().type != TokenType::CloseCurlyBracket)
		{
			try
			{
				VariableDescriptor newElement = parseVariableDescriptor();
				result->elements.push_back(newElement);
				expect(TokenType::EndOfSentence, "Expected a ';' token.");
			}
			catch (Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);
				skipUntilReach(TokenType::EndOfSentence);
			}
		}
		expect(TokenType::CloseCurlyBracket, "Expected a '}' token.");
		expect(TokenType::EndOfSentence, "Expected a ';' token.");

		return result;
	}

	std::shared_ptr<TextureMetaToken> MetaTokenizer::parseTextureMetaToken()
	{
		std::shared_ptr<TextureMetaToken> result = std::make_shared<TextureMetaToken>();

		expect(TokenType::Texture, "Expected a texture keyword.");
		result->name = expect(TokenType::Identifier, "Expected an identifier token.");
		expect(TokenType::EndOfSentence, "Expected a ';' token.");

		return result;
	}

	ReturnTypeDescriptor MetaTokenizer::parseReturnTypeDescriptor()
	{
		ReturnTypeDescriptor result;

		result.type = parseTypeDescriptor();
		result.arraySizes = parseArraySizes();

		return result;
	}

	std::shared_ptr<FunctionMetaToken> MetaTokenizer::parseFunctionMetaToken()
	{
		std::shared_ptr<FunctionMetaToken> result = std::make_shared<FunctionMetaToken>();

		result->returnType = parseReturnTypeDescriptor();
		result->name = expect(TokenType::Identifier, "Expected an identifier token.");
		expect(TokenType::OpenParenthesis, "Expected a '(' token.");

		while (hasTokenLeft() && currentToken().type != TokenType::CloseParenthesis)
		{
			if (result->parameters.size() != 0)
			{
				expect(TokenType::Comma, "Expected a ',' token.");
			}

			try
			{
				result->parameters.push_back(parseVariableDescriptor());
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);
				skipUntilReach({ TokenType::Comma, TokenType::CloseParenthesis });
			}
		}

		expect(TokenType::CloseParenthesis, "Expected a ')' token.");
		result->body = parseSymbolBody();

		return result;
	}

	std::shared_ptr<PipelineBodyMetaToken> MetaTokenizer::parsePipelineBodyMetaToken()
	{
		std::shared_ptr<PipelineBodyMetaToken> result = std::make_shared<PipelineBodyMetaToken>();

		result->type = expect(TokenType::PipelineFlow, "Expected a pipeline token.");
		expect(TokenType::OpenParenthesis, "Expected a '(' token.");
		expect(TokenType::CloseParenthesis, "Expected a ')' token.");
		result->body = parseSymbolBody();

		return result;
	}

	std::shared_ptr<NamespaceMetaToken> MetaTokenizer::parseNamespaceMetaToken()
	{
		std::shared_ptr<NamespaceMetaToken> result = std::make_shared<NamespaceMetaToken>();

		expect(TokenType::Namespace, "Expected a namespace keyword.");
		result->name = expect(TokenType::Identifier, "Expected an identifier token.");
		expect(TokenType::OpenCurlyBracket, "Expected a '{' token.");
		while (hasTokenLeft() && currentToken().type != TokenType::CloseCurlyBracket)
		{
			try
			{
				switch (currentToken().type)
				{
				case TokenType::StructureBlock:
				case TokenType::ConstantBlock:
				case TokenType::AttributeBlock:
				{
					result->innerMetaTokens.push_back(parseBlockMetaToken(currentToken().type));
					break;
				}
				case TokenType::Identifier:
				{
					result->innerMetaTokens.push_back(parseFunctionMetaToken());
					break;
				}
				case TokenType::Texture:
				{
					result->innerMetaTokens.push_back(parseTextureMetaToken());
					break;
				}
				case TokenType::Namespace:
				{
					result->innerMetaTokens.push_back(parseNamespaceMetaToken());
					break;
				}
				default:
					throw Lumina::TokenBasedError("Invalid token type [" + Lumina::to_string(currentToken().type) + "].", currentToken());
				}
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);
				skipLine();
			}
		}
		expect(TokenType::CloseCurlyBracket, "Expected a '}' token.");

		return result;
	}
}