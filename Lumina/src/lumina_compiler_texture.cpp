#include "lumina_compiler.hpp"

namespace Lumina
{
	void Compiler::compileTexture(std::shared_ptr<TextureMetaToken> p_metaToken)
	{
		Variable newTextureVariable = { _type("Texture"), namespacePrefix() + p_metaToken->name.content, {} };

		_result.value.fragmentShaderCode += "uniform sampler2D Texture_" + newTextureVariable.name + ";\n";

		_result.value.textures += newTextureVariable.name + " Texture_" + newTextureVariable.name + " " + std::to_string(nbTexture) + "\n";

		_fragmentVariables.insert(newTextureVariable);

		nbTexture++;
	}
}