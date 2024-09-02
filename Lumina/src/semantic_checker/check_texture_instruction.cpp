#include "lumina_semantic_checker.hpp"

#include <regex>

namespace Lumina
{
	void SemanticChecker::checkTextureInstruction(const std::filesystem::path& p_file, const std::shared_ptr<TextureInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();

		if (_textures.contains(namespacePrefix + p_instruction->name.content) == true)
		{
			throwException(p_file, "Texture named [" + p_instruction->name.content + "] already exist", p_instruction->name);
		}

		_textures.insert(namespacePrefix + p_instruction->name.content);
	}

	void SemanticChecker::compileTextureInstruction(const std::shared_ptr<TextureInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();
		std::string textureName = std::regex_replace(namespacePrefix + p_instruction->name.content, std::regex("::"), "_");

		_result.sections.texture += namespacePrefix + p_instruction->name.content + "\n";
		_result.sections.fragmentShader += "sampler2D " + textureName + ";\n";
	}
}