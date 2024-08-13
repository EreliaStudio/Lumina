#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkPipelineFlowInstruction(const std::filesystem::path& p_file, const std::shared_ptr<PipelineFlowInstruction>& p_instruction)
	{
		std::unordered_map<std::string, Type*>* target;
		std::unordered_map<std::string, Type*>* targetSecond = nullptr;

		if (p_instruction->inputPipeline.content == "Input")
		{
			if (p_instruction->outputPipeline.content == "VertexPass")
			{
				target = &(_vertexPassVariables);
			}
			else
			{
				throw TokenBasedError(p_file, "Invalid pipeline output token for input [Input] [" + p_instruction->outputPipeline.content + "]", p_instruction->outputPipeline);
			}
		}
		else if (p_instruction->inputPipeline.content == "VertexPass")
		{
			if (p_instruction->outputPipeline.content == "FragmentPass")
			{
				target = &(_fragmentPassVariables);
				targetSecond = &(_vertexPassVariables);
			}
			else
			{
				throw TokenBasedError(p_file, "Invalid pipeline output token for input [VertexPass] [" + p_instruction->outputPipeline.content + "]", p_instruction->outputPipeline);
			}
		}
		else
		{
			throw TokenBasedError(p_file, "Invalid pipeline input token [" + p_instruction->inputPipeline.content + "]", p_instruction->inputPipeline);
		}

		Token typeToken = Token::merge(p_instruction->type->tokens, Token::Type::Identifier);

		Type* tmpType = standardType(typeToken.content);
		if (tmpType == nullptr)
		{
			throw TokenBasedError(p_file, "Invalid pipeline flow type [" + typeToken.content + "]", typeToken);
		}

		(*target)[p_instruction->name.content] = tmpType;
		if (targetSecond != nullptr)
			(*targetSecond)[p_instruction->name.content] = tmpType;
	}
}