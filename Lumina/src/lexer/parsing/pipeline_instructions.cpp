#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<PipelineFlowInstruction> LexerChecker::parsePipelineFlowInstruction()
	{
		std::shared_ptr<PipelineFlowInstruction> result = std::make_shared<PipelineFlowInstruction>();

		result->inputPipeline = expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow token."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::PipelineFlowSeparator, "Expected a pipeline flow separator token."+ DEBUG_INFORMATION);
		result->outputPipeline = expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow token."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::Separator, "Expected a separator token."+ DEBUG_INFORMATION);
		result->type = parseTypeInstruction();
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an identifier name");
		expect(Lumina::Token::Type::EndOfSentence, "Expected end of sentence."+ DEBUG_INFORMATION);

		return result;
	}

	std::shared_ptr<PipelineBodyInstruction> LexerChecker::parsePipelineBodyInstruction()
	{
		std::shared_ptr<PipelineBodyInstruction> result = std::make_shared<PipelineBodyInstruction>();

		result->pipelineToken = expect(Lumina::Token::Type::PipelineFlow, "Expected a pipeline flow token."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::OpenParenthesis, "Expected an open parenthesis."+ DEBUG_INFORMATION);
		expect(Lumina::Token::Type::CloseParenthesis, "Expected a close parenthesis."+ DEBUG_INFORMATION);

		result->body = parseSymbolBodyInstruction();

		return result;
	}
}