#pragma once

#include "lumina_tokenizer.hpp"
#include "lumina_exception.hpp"

using TokenType = Lumina::Token::Type;

struct MetaToken
{
	enum class Type
	{
		Unknow,
		Include,
		PipelineFlow,
		Constant,
		Attribute,
		Structure,
	};

	Type type;

	MetaToken(Type p_type) : 
		type(p_type)
	{

	}
};

struct IncludeMetaToken : public MetaToken
{
	std::string filePath;

	IncludeMetaToken() :
		MetaToken(Type::Include)
	{

	}
};

struct TypeDescriptor
{
	Lumina::Token value;

	void append(const Lumina::Token& p_newToken)
	{
		if (value.content == "")
		{
			value = p_newToken;
		}
		else
		{
			value.content += p_newToken.content;
		}
	}
};

struct VariableDescriptor
{
	TypeDescriptor type;
	Lumina::Token name;
	size_t arraySize;
};

struct PipelineFlowMetaToken : public MetaToken
{
	Lumina::Token inputFlow;
	Lumina::Token outputFlow;
	VariableDescriptor variableDescriptor;

	PipelineFlowMetaToken() :
		MetaToken(Type::PipelineFlow)
	{

	}
};

struct BlockMetaToken : public MetaToken
{
	Lumina::Token name;
	std::vector<VariableDescriptor> elements;

	BlockMetaToken(Type p_type) :
		MetaToken(p_type)
	{

	}
};

struct ConstantMetaToken : public BlockMetaToken
{
	ConstantMetaToken() :
		BlockMetaToken(MetaToken::Type::Constant)
	{

	}
};

struct AttributeMetaToken : public BlockMetaToken
{
	AttributeMetaToken() :
		BlockMetaToken(MetaToken::Type::Attribute)
	{

	}
};

struct StructureMetaToken : public BlockMetaToken
{
	StructureMetaToken() :
		BlockMetaToken(MetaToken::Type::Structure)
	{

	}
};

class MetaTokenizer
{
public:
	using Product = Lumina::Expected<std::vector<std::shared_ptr<MetaToken>>>;

private:
	std::vector<Lumina::Token> _tokens;
	size_t _index = 0;
	Lumina::Token noToken;

	MetaTokenizer()
	{

	}

	std::shared_ptr<IncludeMetaToken> parseIncludeMetaToken()
	{
		std::shared_ptr<IncludeMetaToken> result = std::make_shared<IncludeMetaToken>();

		expect(TokenType::Include, "Expected a '#include' token.");
		std::string pathToken = expect({ TokenType::IncludeLitteral, TokenType::StringLitteral }, "Expected an include file path.").content;
		result->filePath = pathToken.substr(1, pathToken.size() - 2);

		return (result);
	}

	TypeDescriptor parseTypeDescriptor()
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

		return (result);
	}

	size_t parseArraySize()
	{
		auto applyPrimary = [this]() -> size_t {
			switch (currentToken().type)
			{
			case TokenType::OpenParenthesis:
			{
				advance();
				size_t value = parseArraySize();
				expect(TokenType::CloseParenthesis, "Expected a ')' token.");
				return value;
			}
			case TokenType::Number:
			{
				size_t value = std::stoul(currentToken().content);
				advance();
				return value;
			}
			default:
			{
				throw Lumina::TokenBasedError("Expected a number or '(' token.", currentToken());
			}
			}
			};

		auto applyFactor = [&applyPrimary, this]() -> size_t {
			size_t leftValue = applyPrimary();

			while (currentToken().type == TokenType::Operator &&
				(currentToken().content == "*" || currentToken().content == "/" || currentToken().content == "%"))
			{
				std::string op = currentToken().content;
				advance();
				size_t rightValue = applyPrimary();

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

		size_t leftValue = applyFactor();

		while (currentToken().type == TokenType::Operator &&
			(currentToken().content == "+" || currentToken().content == "-"))
		{
			std::string op = currentToken().content;
			advance();
			size_t rightValue = applyFactor();

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


	VariableDescriptor parseVariableDescriptor()
	{
		VariableDescriptor result;

		result.type = parseTypeDescriptor();
		result.name = expect(TokenType::Identifier, "Expected a identifier name.");
		if (currentToken().type == TokenType::OpenBracket)
		{
			expect(TokenType::OpenBracket, "Expected a '[' token.");
			result.arraySize = parseArraySize();

			if (result.arraySize == 0)
			{
				throw Lumina::TokenBasedError("Array size evaluated to 0", result.name);
			}
			expect(TokenType::CloseBracket, "Expected a ']' token.");
		}
		else
		{
			result.arraySize = 0;
		}

		return (result);
	}

	std::shared_ptr<PipelineFlowMetaToken> parsePipelineFlowMetaToken()
	{
		std::shared_ptr<PipelineFlowMetaToken> result = std::make_shared<PipelineFlowMetaToken>();

		result->inputFlow = expect(TokenType::PipelineFlow, "Expected a pipeline flow token.");
		expect(TokenType::PipelineFlowSeparator, "Expected a '->' token.");
		result->outputFlow = expect(TokenType::PipelineFlow, "Expected a pipeline flow token.");
		expect(TokenType::Separator, "Expected a ':' token.");
		result->variableDescriptor = parseVariableDescriptor();
		expect(TokenType::EndOfSentence, "Expected a ';' token.");

		if (result->variableDescriptor.arraySize != 0)
		{
			throw Lumina::TokenBasedError("Pipeline flow variable cannot be array (" + std::to_string(result->variableDescriptor.arraySize) + ").", result->variableDescriptor.name);
		}

		return (result);
	}

	std::shared_ptr<BlockMetaToken> parseBlock(const TokenType& p_tokenType)
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
			VariableDescriptor newElement = parseVariableDescriptor();

			expect(TokenType::EndOfSentence, "Expected a ';' token.");
			result->elements.push_back(newElement);
		}
		expect(TokenType::CloseCurlyBracket, "Expected a '}' token.");
		expect(TokenType::EndOfSentence, "Expected a ';' token.");

		return (result);
	}

	Product _analyse(const std::vector<Lumina::Token>& p_tokens)
	{
		Product result;

		_tokens = p_tokens;

		while (hasTokenLeft() == true)
		{
			try
			{
				switch (currentToken().type)
				{
				case TokenType::Include:
				{
					result.value.push_back(parseIncludeMetaToken());
					break;
				}
				case TokenType::StructureBlock:
				case TokenType::ConstantBlock:
				case TokenType::AttributeBlock:
				{
					result.value.push_back(parseBlock(currentToken().type));
					break;
				}
				case TokenType::PipelineFlow:
				{
					if (nextToken().type == TokenType::PipelineFlowSeparator)
					{
						result.value.push_back(parsePipelineFlowMetaToken());
						break;
					}
					else
					{

					}
				}
				default:
					throw Lumina::TokenBasedError("Invalid token type [" + Lumina::to_string(currentToken().type) + "].", currentToken());
				}
			}
			catch (const Lumina::TokenBasedError& e)
			{
				result.errors.push_back(e);
				skipLine();
			}
		}

		return (result);
	}

	bool hasTokenLeft() const
	{
		return (_index < _tokens.size());
	}

	void backOff()
	{
		_index--;
	}

	void advance()
	{
		_index++;
	}

	const Lumina::Token& currentToken() const
	{
		return (_tokens.operator[](_index));
	}

	const Lumina::Token& tokenAtIndex(size_t p_index) const
	{
		if (_index + p_index >= _tokens.size())
			return noToken;
		return (_tokens.operator[](_index + p_index));
	}

	const Lumina::Token& nextToken() const
	{
		return (tokenAtIndex(1));
	}

	void skipToken()
	{
		_index++;
	}

	void skipLine()
	{
		int currentLine = currentToken().context.line;
		while (hasTokenLeft() == true &&
			currentLine == currentToken().context.line)
		{
			skipToken();
		}
	}

	const Lumina::Token& expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage)
	{
		if (currentToken().type != p_expectedType)
		{
			throw Lumina::TokenBasedError(p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return (result);
	}

	const Lumina::Token& expect(std::vector<Lumina::Token::Type> p_expectedTypes, const std::string& p_errorMessage)
	{
		bool found = false;

		for (const auto& type : p_expectedTypes)
		{
			if (currentToken().type == type)
			{
				found = true;
			}
		}

		if (found == false)
		{
			throw Lumina::TokenBasedError(p_errorMessage, currentToken());
		}
		const Lumina::Token& result = currentToken();
		advance();
		return (result);
	}

public:

	static Product analyse(const std::vector<Lumina::Token>& p_tokens)
	{
		return (MetaTokenizer()._analyse(p_tokens));
	}
};

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1]);

	MetaTokenizer::Product metaTokens = MetaTokenizer::analyse(tokens);

	if (metaTokens.errors.size() != 0)
	{
		for (const auto& error : metaTokens.errors)
		{
			std::cout << error.what() << std::endl;
		}
		return (-1);
	}

	return (0);
}
