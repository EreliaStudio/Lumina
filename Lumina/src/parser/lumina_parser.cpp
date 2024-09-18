#include "lumina_parser.hpp"

#include "lumina_utils.hpp"
#include "lumina_tokenizer.hpp"

namespace Lumina
{
	void Parser::handleInclude()
	{
		expect(Lumina::Token::Type::Include, "Unexpected token type.");
		Lumina::Token includePathToken = expect({ Lumina::Token::Type::StringLitteral, Lumina::Token::Type::IncludeLitteral }, "Unexpected include file token type.");

		std::string fileName = includePathToken.content.substr(1, includePathToken.content.size() - 2);
		std::filesystem::path filePath = composeFilePath(fileName, { std::filesystem::path(includePathToken.context.file).parent_path() });

		Expected<RootNode> includeResult = Parser::parse(Lumina::Tokenizer::tokenize(filePath));

		if (includeResult.value.vertexNode.parsed == true || includeResult.value.fragmentNode.parsed == true)
		{
			throw TokenBasedError("Vertex pass can't be defined inside an include file", includePathToken);
		}

		_parsingResult.insertErrors(includeResult);
		_parsingResult.value.merge(includeResult.value);
	}

	size_t Parser::evaluateArraySize()
	{
		std::vector<Lumina::Token> tokens;

		while (hasTokenLeft() && currentToken().type != Token::Type::CloseBracket)
		{
			tokens.push_back(currentToken());
			advance();
		}

		if (tokens.empty()) {
			throw TokenBasedError("Array size expression is empty.", currentToken());
		}

		auto precedence = [](char op) -> int {
			if (op == '*' || op == '/') return 2;
			if (op == '+' || op == '-') return 1;
			return 0;
			};

		auto applyOperator = [](char op, size_t a, size_t b) -> size_t {
			switch (op) {
			case '+': return a + b;
			case '-': return a - b;
			case '*': return a * b;
			case '/':
				if (b == 0)throw std::runtime_error("Division by zero.");
				return a / b;
			default: throw std::runtime_error("Unknown operator.");
			}
			};

		std::vector<size_t> values;
		std::vector<char> operators;
		int openParentheses = 0;

		auto evaluateTopOperation = [&]() {
			if (values.size() < 2 || operators.empty()) {
				throw std::runtime_error("Invalid expression.");
			}
			size_t b = values.back(); values.pop_back();
			size_t a = values.back(); values.pop_back();
			char op = operators.back(); operators.pop_back();
			values.push_back(applyOperator(op, a, b));
			};

		for (const auto& token : tokens)
		{
			if (token.type == Lumina::Token::Type::Number)
			{
				values.push_back(std::stoul(token.content));
			}
			else if (token.type == Lumina::Token::Type::Operator)
			{
				char op = token.content[0];

				while (!operators.empty() && precedence(operators.back()) >= precedence(op))
				{
					evaluateTopOperation();
				}
				operators.push_back(op);
			}
			else if (token.content == "(")
			{
				operators.push_back('(');
				openParentheses++;
			}
			else if (token.content == ")")
			{
				if (openParentheses == 0) {
					throw TokenBasedError("Unmatched closing parenthesis.", token);
				}
				openParentheses--;

				while (!operators.empty() && operators.back() != '(')
				{
					evaluateTopOperation();
				}
				operators.pop_back();
			}
			else
			{
				throw TokenBasedError("Invalid token in array size expression.", token);
			}
		}

		if (openParentheses > 0) {
			throw TokenBasedError("Unmatched opening parenthesis.", currentToken());
		}

		while (!operators.empty()) {
			evaluateTopOperation();
		}

		if (values.size() != 1) {
			throw TokenBasedError("Invalid array size expression.", currentToken());
		}

		return values.back();
	}


	TypeNode Parser::parseTypeNode()
	{
		TypeNode typeNode;

		typeNode.nameTokens.push_back(expect(Token::Type::Identifier, "Expected type name."));

		while (hasTokenLeft() && currentToken().type == Token::Type::NamespaceSeparator)
		{
			typeNode.nameTokens.push_back(expect(Token::Type::NamespaceSeparator, "Expected '::' token."));
			typeNode.nameTokens.push_back(expect(Token::Type::Identifier, "Expected identifier after '::'."));
		}

		if (currentToken().type == Token::Type::OpenBracket)
		{
			expect(Token::Type::OpenBracket, "Expected '[' before array size or formula.");

			try
			{
				typeNode.arraySize = evaluateArraySize();
			}
			catch (...)
			{
				throw TokenBasedError("Invalid operation detected during array size calculation.", currentToken());
			}

			expect(Token::Type::CloseBracket, "Expected ']' after array size or formula.");
		}

		return typeNode;
	}

	PipelineFlowNode Parser::parsePipelineFlowNode()
	{
		PipelineFlowNode pipelineFlowNode;

		pipelineFlowNode.input = expect(Token::Type::PipelineFlow, "Expected input flow in pipeline.");

		expect(Token::Type::PipelineFlowSeparator, "Expected '->' between input and output flows.");

		pipelineFlowNode.output = expect(Token::Type::PipelineFlow, "Expected output flow in pipeline.");

		expect(Token::Type::Separator, "Expected ':' before type declaration.");

		pipelineFlowNode.type = parseTypeNode();

		if (pipelineFlowNode.type.arraySize != 0)
		{
			throw TokenBasedError("Pipeline flow type can't be arrays.", currentToken());
		}

		pipelineFlowNode.name = expect(Token::Type::Identifier, "Expected pipeline flow name.");

		expect(Token::Type::EndOfSentence, "Expected ';' after pipeline flow definition.");

		return pipelineFlowNode;
	}

	void Parser::handlePipelineFlow()
	{
		PipelineFlowNode newNode = parsePipelineFlowNode();

		if (newNode.input.content == "Input" || newNode.input.content == "VertexPass")
		{
			if (newNode.input.content == "Input" && newNode.output.content == "VertexPass")
			{
				_parsingResult.value.vertexPipelineFlow.push_back(newNode);
			}
			else if (newNode.input.content == "VertexPass" && newNode.output.content == "FragmentPass")
			{
				_parsingResult.value.fragmentPipelineFlow.push_back(newNode);
			}
			else
			{
				throw TokenBasedError("Invalid output pipeline token", newNode.output);
			}
		}
		else
		{
			throw TokenBasedError("Invalid input pipeline token", newNode.input);
		}
	}

	BlockNode::Element Parser::parseBlockElement()
	{
		BlockNode::Element element;

		element.returnType = parseTypeNode();
		element.name = expect(Token::Type::Identifier, "Expected variable name after type.");

		expect(Token::Type::EndOfSentence, "Expected ';' after variable declaration.");

		return element;
	}

	NamespaceNode Parser::parseNamespace()
	{
		NamespaceNode result;

		expect(Lumina::Token::Type::Namespace, "Expected a namespace keyword.");
		result.nameToken = expect(Lumina::Token::Type::Identifier, "Expected a namespace name.");

		expect(Token::Type::OpenCurlyBracket, "Expected '{' after namespace name.");

		while (hasTokenLeft() && currentToken().type != Token::Type::CloseCurlyBracket)
		{
			try
			{
				switch (currentToken().type)
				{
				case Lumina::Token::Type::Include:
				{
					throw TokenBasedError("Include cannot be placed inside namespace definition", currentToken());
					break;
				}
				case Lumina::Token::Type::PipelineFlow:
				{
					throw TokenBasedError("Pipeline flow cannot be placed inside namespace definition", currentToken());
					break;
				}
				case Lumina::Token::Type::StructureBlock:
				{
					result.structureNodes.push_back(parseBlock<StructureNode>(Lumina::Token::Type::StructureBlock));
					break;
				}
				case Lumina::Token::Type::AttributeBlock:
				{
					result.attributeNodes.push_back(parseBlock<AttributeNode>(Lumina::Token::Type::AttributeBlock));
					break;
				}
				case Lumina::Token::Type::ConstantBlock:
				{
					result.constantNodes.push_back(parseBlock<ConstantNode>(Lumina::Token::Type::ConstantBlock));
					break;
				}
				case Lumina::Token::Type::Identifier:
				{
					result.functionNodes.push_back(parseFunction());
					break;
				}
				case Lumina::Token::Type::Namespace:
				{
					result.innerNamespaceNodes.push_back(parseNamespace());
					break;
				}
				default:
					throw TokenBasedError("Invalid token type [" + Lumina::Token::to_string(currentToken().type) + "]", currentToken());
				}
			}
			catch (const TokenBasedError& e)
			{
				_parsingResult.errors.push_back(e);
				skipLine();
			}
		}

		expect(Token::Type::CloseCurlyBracket, "Expected '}' to close namespace definition.");

		return (result);
	}

	SymbolBodyNode Parser::parseSymbolBody()
	{
		SymbolBodyNode result;

		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected '{' to start symbol body.");

		int openBraces = 1;

		while (hasTokenLeft() && currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				switch (currentToken().type)
				{
					case Lumina::Token::Type::Comment:
					{
						skipToken();
						break;
					}
					case Lumina::Token::Type::OpenCurlyBracket:
					{
						SymbolBodyNode innerSymbolBody = parseSymbolBody();
					}

					default:
					{
						throw TokenBasedError("Invalid token type [" + Lumina::Token::to_string(currentToken().type) + "]", currentToken());
					}
				}
			}
			catch (const TokenBasedError& e)
			{
				_parsingResult.errors.push_back(e);
				skipLine();
			}
		}
		

		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected '}' to close symbol body.");


		return result;
	}

	ParameterNode Parser::parseParameter()
	{
		ParameterNode result;

		result.type = parseTypeNode();
		result.name = expect(Token::Type::Identifier, "Expected a parameter name token.");

		return (result);
	}

	FunctionNode Parser::parseFunction()
	{
		FunctionNode result;

		TypeNode returnType;
		Lumina::Token name;
		std::vector<ParameterNode> parameters;

		result.returnType = parseTypeNode();
		result.name = expect(Token::Type::Identifier, "Expected a function name token.");
		expect(Lumina::Token::Type::OpenParenthesis, "Expected '(' before function parameter(s).");
		while (hasTokenLeft() && currentToken().type != Lumina::Token::Type::CloseParenthesis)
		{
			if (result.parameters.size() != 0)
			{
				expect(Lumina::Token::Type::Comma, "Expected ',' between function parameter(s).");
			}

			result.parameters.push_back(parseParameter());

			
		}
		expect(Lumina::Token::Type::CloseParenthesis, "Expected ')' after function parameter(s).");
		result.body = parseSymbolBody();

		return (result);
	}

	PipelineBodyNode Parser::parsePipelineBody()
	{
		PipelineBodyNode result;

		result.parsed = true;

		expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow name token.");
		expect(Lumina::Token::Type::OpenParenthesis, "Expected a '()' token after pipeline flow name.");
		expect(Lumina::Token::Type::CloseParenthesis, "Expected a '()' token after pipeline flow name.");

		result.body = parseSymbolBody();

		return (result);
	}

	void Parser::handlePipelineDefinition()
	{
		if (currentToken().content == "VertexPass")
		{
			_parsingResult.value.vertexNode = parsePipelineBody();
		}
		else if (currentToken().content == "FragmentPass")
		{
			_parsingResult.value.fragmentNode = parsePipelineBody();
		}
		else
		{
			throw TokenBasedError("[Input] is not a valid pipeline pass name", currentToken());
		}
	}

	Expected<RootNode> Parser::operator()(const std::vector<Token>& tokens)
	{
		_tokens = &tokens;
		_index = 0;

		while (hasTokenLeft())
		{
			try
			{
				switch (currentToken().type)
				{
				case Lumina::Token::Type::Include:
				{
					handleInclude();
					break;
				}
				case Lumina::Token::Type::PipelineFlow:
				{
					if (nextToken().type == Token::Type::PipelineFlowSeparator)
						handlePipelineFlow();
					else
						handlePipelineDefinition();
					break;
				}
				case Lumina::Token::Type::StructureBlock:
				{
					_parsingResult.value.anonymNamespace.structureNodes.push_back(parseBlock<StructureNode>(Lumina::Token::Type::StructureBlock));
					break;
				}
				case Lumina::Token::Type::AttributeBlock:
				{
					_parsingResult.value.anonymNamespace.attributeNodes.push_back(parseBlock<AttributeNode>(Lumina::Token::Type::AttributeBlock));
					break;
				}
				case Lumina::Token::Type::ConstantBlock:
				{
					_parsingResult.value.anonymNamespace.constantNodes.push_back(parseBlock<ConstantNode>(Lumina::Token::Type::ConstantBlock));
					break;
				}
				case Lumina::Token::Type::Identifier:
				{
					_parsingResult.value.anonymNamespace.functionNodes.push_back(parseFunction());
					break;
				}
				case Lumina::Token::Type::Namespace:
				{
					_parsingResult.value.anonymNamespace.innerNamespaceNodes.push_back(parseNamespace());
					break;
				}
				default:
					throw TokenBasedError("Invalid token type [" + Lumina::Token::to_string(currentToken().type) + "]", currentToken());
				}
			}
			catch (const TokenBasedError& e)
			{
				_parsingResult.errors.push_back(e);
				skipLine();
			}
		}

		return _parsingResult;
	}
}
