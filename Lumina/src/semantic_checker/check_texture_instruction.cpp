#include "lumina_semantic_checker.hpp"

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
}