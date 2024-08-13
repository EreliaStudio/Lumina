#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<BlockElementInstruction> LexerChecker::parseBlockElementInstruction()
	{
		std::shared_ptr<BlockElementInstruction> result = std::make_shared<BlockElementInstruction>();

		result->type = parseTypeInstruction();
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an identifier name");
		if (currentToken().type != Lumina::Token::Type::EndOfSentence)
		{
			expect(Lumina::Token::Type::OpenBracket, "Expected an opening bracket to define an array or an end of sentence." + DEBUG_INFORMATION);
			Token numberToken = expect(Lumina::Token::Type::Number, "Expected a number of element for the array." + DEBUG_INFORMATION);
			result->nbElement = std::stoi(numberToken.content);
			expect(Lumina::Token::Type::CloseBracket, "Expected a closing bracket." + DEBUG_INFORMATION);
		}
		else
		{
			result->nbElement = 0;
		}
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<StructureBlockInstruction> LexerChecker::parseStructureBlockInstruction()
	{
		std::shared_ptr<StructureBlockInstruction> result = std::make_shared<StructureBlockInstruction>();

		expect(Lumina::Token::Type::StructureBlock, "Expected a structure block token."+ DEBUG_INFORMATION);
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an structure type name");
		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected an open curly bracket."+ DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			result->elements.push_back(parseBlockElementInstruction());
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a close curly bracket."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<AttributeBlockInstruction> LexerChecker::parseAttributeBlockInstruction()
	{
		std::shared_ptr<AttributeBlockInstruction> result = std::make_shared<AttributeBlockInstruction>();

		expect(Lumina::Token::Type::AttributeBlock, "Expected an attribute block token."+ DEBUG_INFORMATION);
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an attribute type name");
		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected an open curly bracket."+ DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			result->elements.push_back(parseBlockElementInstruction());
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a close curly bracket."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<ConstantBlockInstruction> LexerChecker::parseConstantBlockInstruction()
	{
		std::shared_ptr<ConstantBlockInstruction> result = std::make_shared<ConstantBlockInstruction>();

		expect(Lumina::Token::Type::ConstantBlock, "Expected a constant block token."+ DEBUG_INFORMATION);
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an constant type name");
		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected an open curly bracket."+ DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				result->elements.push_back(parseBlockElementInstruction());
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);

				skipLine();
			}
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a close curly bracket."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<TextureInstruction> LexerChecker::parseTextureInstruction()
	{
		std::shared_ptr<TextureInstruction> result = std::make_shared<TextureInstruction>();

		expect(Lumina::Token::Type::Texture, "Expected a texture token."+ DEBUG_INFORMATION);
		result->name = expect(Lumina::Token::Type::Identifier);
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}
}