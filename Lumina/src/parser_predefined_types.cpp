#include "parser.hpp"

#include "tokenizer.hpp"
#include "lexer.hpp"

namespace Lumina
{
	void Parser::composeStandardTypes()
	{
		Type* voidType = _insertType({
					.name = "void",
					.attributes = {
				}
			});

		Type* boolType = _insertType({
					.name = "bool",
					.attributes = {
				}
			});

		Type* intType = _insertType({
					.name = "int",
					.attributes = {
					}
			});

		Type* uintType = _insertType({
					.name = "uint",
					.attributes = {
				}
			});

		Type* floatType = _insertType({
					.name = "float",
					.attributes = {
				}
			});

		intType->acceptedConvertions = { intType, uintType, floatType };
		uintType->acceptedConvertions = { intType, uintType, floatType };
		floatType->acceptedConvertions = { intType, uintType, floatType };

		boolType->acceptedConvertions = { boolType };
	}
	void Parser::composePredefinedTypes()
	{
		std::vector<Lumina::Token> predefinedTokens = Lumina::Tokenizer::tokenize("predefined_header/lumina_header.lum");

		Lexer::Product lexerProduct = Lexer::lex(predefinedTokens);

		if (lexerProduct.errors.size() != 0)
		{
			for (const auto& error : lexerProduct.errors)
			{
				std::cerr << error.what() << std::endl;
			}
		}

		_parseNamespace(lexerProduct.value.anonymNamespace);
	}
	
	void Parser::composeComplexStandardTypes()
	{
		Type* textureType = _insertType({
					.name = "Texture",
					.attributes = {
				}
			});

		textureType->methods["getPixel"] = {
			composeMethod(textureType, R"(Color getPixel(Vector2 p_UV){return (texture(self, p_UV));})")
		};
	}

	Parser::Parser()
	{
		_reservedIdentifiers.insert("main");

		composeStandardTypes();
		composePredefinedTypes();
		composeComplexStandardTypes();
	}
}