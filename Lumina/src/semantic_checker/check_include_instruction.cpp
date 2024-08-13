#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkIncludeInstruction(const std::filesystem::path& p_file, const std::shared_ptr<IncludeInstruction>& p_instruction)
	{
		std::string fileName = p_instruction->includeFile.content.substr(1, p_instruction->includeFile.content.size() - 2);
		std::filesystem::path filePath = composeFilePath(fileName, { p_file.parent_path() });

		if (filePath.empty())
		{
			throw TokenBasedError(p_file, "Include file [" + fileName + "] not found" + DEBUG_INFORMATION, p_instruction->includeFile);
		}

		if (_loadedIncludes.contains(filePath) == false)
		{
			Lumina::LexerChecker::Result includeFileLexer = Lumina::LexerChecker::checkSyntax(fileName, Lumina::Tokenizer::tokenize(Lumina::readFileAsString(filePath)));

			_result.errors.insert(_result.errors.end(), includeFileLexer.errors.begin(), includeFileLexer.errors.end());

			std::vector<Element> newElements;
			for (auto& instruction : includeFileLexer.instructions)
			{
				newElements.push_back(Element{ filePath, instruction });
			}

			_elements.insert(_elements.begin() + _index + 1, newElements.begin(), newElements.end());

			_loadedIncludes.insert(filePath);
		}
	}
}