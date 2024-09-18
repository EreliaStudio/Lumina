#include "lumina_semantic_checker.hpp"

#include <regex>

namespace Lumina
{
	void SemanticChecker::checkTextureInstruction(const std::filesystem::path& p_file, const std::shared_ptr<TextureInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();
		std::string textureName = namespacePrefix + p_instruction->name.content;

		if (_textures.contains(textureName) == true)
		{
			throwException(p_file, "Texture named [" + p_instruction->name.content + "] already exist", p_instruction->name);
		}

		_textures.insert(textureName);

		if (_vertexPassVariables.contains(textureName) == true ||
			_fragmentPassVariables.contains(textureName) == true)
		{
			throwException(p_file, "Conflict name [" + p_instruction->name.content + "] : Name already used previously", p_instruction->name);
		}

		_vertexPassVariables[textureName] = { type("Texture") };
		_fragmentPassVariables[textureName] = {type("Texture") };
	}

	void SemanticChecker::compileTextureInstruction(const std::shared_ptr<TextureInstruction>& p_instruction)
	{
		std::string namespacePrefix = createNamespacePrefix();
		std::string textureName = std::regex_replace(namespacePrefix + p_instruction->name.content, std::regex("::"), "_");

		_result.sections.texture += namespacePrefix + p_instruction->name.content + " luminaTexture_" + textureName + " " + std::to_string(_nbTexture) + "\n";
		_result.sections.fragmentShader += "uniform sampler2D " + textureName + ";\n\n";
		_nbTexture++;
	}
}