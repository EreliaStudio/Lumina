#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkPipelineBodyInstruction(const std::filesystem::path& p_file, const std::shared_ptr<PipelineBodyInstruction>& p_instruction)
	{
		if (_currentNamespace.empty() == false)
		{
			throw TokenBasedError(p_file, "[" + p_instruction->pipelineToken.content + "] can't be defined inside a namespace.", p_instruction->pipelineToken);
		}

		if ((p_instruction->pipelineToken.content == "VertexPass" && _vertexParsed == true) ||
			(p_instruction->pipelineToken.content == "FragmentPass" && _fragmentParsed == true))
		{
			throw TokenBasedError(p_file, "[" + p_instruction->pipelineToken.content + "] already parsed.", p_instruction->pipelineToken);
		}

		checkSymbolBodyInstruction(p_file, p_instruction->body, (p_instruction->pipelineToken.content == "VertexPass" ? _vertexPassVariables : _fragmentPassVariables), type("void"));
	}

	void SemanticChecker::compilePipelineBodyInstruction(const std::shared_ptr<PipelineBodyInstruction>& p_instruction)
	{
		// Implementation goes here
	}
}