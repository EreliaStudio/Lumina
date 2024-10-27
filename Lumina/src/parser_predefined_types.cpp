#include "parser.hpp"

#include "tokenizer.hpp"
#include "lexer.hpp"

namespace Lumina
{
	void Parser::composeStandardTypes()
	{
		// Insert standard types into ShaderRepresentation using designated initializers
		ShaderRepresentation::Type* voidType = _shaderRepresentation.insertType({
			.name = "void",
			.attributes = {},
			.acceptedConvertions = {}
			});

		ShaderRepresentation::Type* boolType = _shaderRepresentation.insertType({
			.name = "bool",
			.attributes = {},
			.acceptedConvertions = {}
			});

		ShaderRepresentation::Type* intType = _shaderRepresentation.insertType({
			.name = "int",
			.attributes = {},
			.acceptedConvertions = {}
			});

		ShaderRepresentation::Type* uintType = _shaderRepresentation.insertType({
			.name = "uint",
			.attributes = {},
			.acceptedConvertions = {}
			});

		ShaderRepresentation::Type* floatType = _shaderRepresentation.insertType({
			.name = "float",
			.attributes = {},
			.acceptedConvertions = {}
			});

		// Set accepted conversions between types
		intType->acceptedConvertions = { intType, uintType, floatType };
		uintType->acceptedConvertions = { intType, uintType, floatType };
		floatType->acceptedConvertions = { intType, uintType, floatType };
		boolType->acceptedConvertions = { boolType };
	}

	void Parser::composePredefinedTypes()
	{
		// Tokenize the predefined header file
		std::vector<Lumina::Token> predefinedTokens = Lumina::Tokenizer::tokenize("predefined_header/lumina_header.lum");

		// Lex the tokens to get the Lexer::Output
		Lexer::Product lexerProduct = Lexer::lex(predefinedTokens);

		if (lexerProduct.errors.size() != 0)
		{
			for (const auto& error : lexerProduct.errors)
			{
				std::cerr << error.what() << std::endl;
			}
			throw std::runtime_error("Error parsing predefined types");
		}

		// Parse the anonymous namespace from the lexed output
		_parseNamespace(lexerProduct.value.anonymNamespace);
	}

	void Parser::composeComplexStandardTypes()
	{
		// Insert the Texture type using designated initializers
		ShaderRepresentation::Type* textureType = _shaderRepresentation.insertType({
			.name = "Texture",
			.attributes = {},
			.acceptedConvertions = {}
			});

		textureType->methods["getPixel"].push_back(composeMethod(textureType, R"(
            Color getPixel(Vector2 p_UV)
            {
                return texture(self, p_UV);
            }
        )"));
	}

	// Implement composeMethod to parse method source code and create a Function
	ShaderRepresentation::Function Parser::composeMethod(const ShaderRepresentation::Type* p_originatorType, const std::string& p_sourceCode)
	{
		return _composeMethodFunction(p_originatorType, Lexer::lexFunctionSourceCode(p_sourceCode));
	}

	// Similarly, implement composeConstructor and composeOperator if needed
	ShaderRepresentation::Type::Constructor Parser::composeConstructor(const ShaderRepresentation::Type* p_originatorType, const std::string& p_sourceCode)
	{
		return _composeConstructorFunction(p_originatorType, Lexer::lexConstructorSourceCode(p_sourceCode));
	}

	ShaderRepresentation::Function Parser::composeOperator(const ShaderRepresentation::Type* p_originatorType, const std::string& p_sourceCode)
	{
		return _composeOperatorFunction(p_originatorType, Lexer::lexOperatorSourceCode(p_sourceCode));
	}

	Parser::Parser()
	{
		// Initialize the parser and compose the standard types
		_shaderRepresentation.reservedIdentifiers.insert("main");

		composeStandardTypes();
		composePredefinedTypes();
		composeComplexStandardTypes();
	}
}
