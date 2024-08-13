#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<IncludeInstruction> LexerChecker::parseIncludeInstruction()
	{
		std::shared_ptr<IncludeInstruction> result = std::make_shared<IncludeInstruction>();

		expect(Lumina::Token::Type::Include, "Unexpected token found."+ DEBUG_INFORMATION);
		result->includeFile = expect({ Lumina::Token::Type::StringLitteral, Lumina::Token::Type::IncludeLitteral }, "Expected a valid include file token."+ DEBUG_INFORMATION);

		return (result);
	}
}