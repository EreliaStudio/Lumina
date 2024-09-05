#include "lumina_semantic_checker.hpp"

#include <regex>

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
		std::string symbolContent = "void main(){\n";
		
		int currentLine = -1;
		for (const auto& token : p_instruction->body->completeBodyTokens)
		{
			if (token.context.line != currentLine)
			{
				symbolContent += token.context.inputLine + "\n";
				currentLine = token.context.line;
			}
		}

		symbolContent = std::regex_replace(symbolContent, std::regex("::"), "_");

		symbolContent += "}\n";

		if (p_instruction->pipelineToken.content == "VertexPass")
		{
			_result.sections.vertexShader += symbolContent;
		}
		else
		{
			_result.sections.fragmentShader += "layout(location = 0) out vec4 pixelColor;\n\n";
			_result.sections.fragmentShader += symbolContent;
		}
	}
}